/*
 * 2010-12-06 Skinny support ccm 8.0.2 (reserved=0x13)
 * 2010-08-06 Skinny support ccm 7.1.3 (reserved=0x12)
 * 2010-04-30 Skinny support ccm 7.1.2 (reserved=0x11)
 */
#include <r_pcap.h>
#include <payload.h>
#include <Utils/voip/dtmf.h>
#include <Utils/StringUtil.h>
#include <Utils/IniFile.h>
#include <Utils/SEH_Exception.h>

PcapDev::PcapDev(const char *name, u_int mask) : 
	Name(name), netmask(mask), 
	devhd(NULL),
 	EnabledSIP(true),
	EnabledH323(true),
	EnabledSCCP(true)
{
}

PcapDev::~PcapDev(void)
{
}

bool PcapDev::Open()
{
	Utils::IniFile ini("./elogger.config");
	ini.Section() = "Pcap " + Name;
	bool Enabled = ini.ReadBool("Enabled", true);
	if (!Enabled)
	{
		pcap_mgr.pRec->log(INF, "\t*** DISABLED ***");
		return false;
	}
	pcap_mgr.pRec->log(INF, "\topen...");
	EnabledSIP = ini.ReadBool("EnabledSIP", true);
	EnabledH323 = ini.ReadBool("EnabledH323", true);
	EnabledSCCP = ini.ReadBool("EnabledSCCP", true);
	if (!EnabledSIP) pcap_mgr.pRec->log(INF, "\tSIP capture disabled");
	if (!EnabledH323) pcap_mgr.pRec->log(INF, "\tH323 capture disabled");
	if (!EnabledSCCP) pcap_mgr.pRec->log(INF, "\tSCCP capture disabled");

	if (!openDev()) return false;

	m_pktHandler.Start([this](const CapPacket& capPkt) {
		this->header = &capPkt.header;
		this->pkt_data = capPkt.pkt_data;
		this->doPacket();
		delete[] capPkt.pkt_data;
	});
	Thread::Start();
	return true;
}

bool PcapDev::openDev()
{
	closeDev();
	/*
		将 to_ms 设置为0意味着没有超时，那么如果没有数据包到达的话，读操作将永远不会返回。 
		如果设置成-1，则情况恰好相反，无论有没有数据包到达，读操作都会立即返回
	*/
	char errbuf[PCAP_ERRBUF_SIZE];
	devhd = pcap_open_live(
				Name.c_str(),
				64000	/*snaplen*/,
				1		/*promiscuous mode*/,
				20		/*read timeout*/,
				errbuf);
	if (devhd == NULL)
	{
		pcap_mgr.pRec->log(ERR, "[pcap_open_live] %s", errbuf);
		return false;
	}
	
	/* 检查数据链路层，只考虑以太网 */
    if (pcap_datalink(devhd) != DLT_EN10MB)
    {
        pcap_mgr.pRec->log(ERR, "Non-Ethernet networks.");
		pcap_close(devhd);
        return false;
    }	

	//A fragmented IP datagram has the TCP or UDP header in the first fragment only
	if (!setFilter("udp or tcp or vlan"))
	{
		pcap_close(devhd);
		return false;
	}
	return true;
}

bool PcapDev::OpenFile()
{
	pcap_mgr.pRec->log(INF, "Open %s...", Name.c_str());
	char errbuf[PCAP_ERRBUF_SIZE];
	devhd = pcap_open_offline(Name.c_str(), errbuf);
	if (devhd == NULL)
	{
		pcap_mgr.pRec->log(ERR, "[pcap_open_offline] %s", errbuf);
		return false;
	}
	if (!setFilter("udp or tcp or vlan"))
	{
		pcap_close(devhd);
		return false;
	}

	m_pktHandler.Start([this](const CapPacket& capPkt) {
		this->header = &capPkt.header;
		this->pkt_data = capPkt.pkt_data;
		this->doPacket();
		delete[] capPkt.pkt_data;
	});
	Thread::Start();
	return true;
}

bool PcapDev::setFilter(const char *filter)
{
	struct bpf_program fcode;
	//编译过滤器
    if (pcap_compile(devhd, &fcode, (char*)filter, 1, netmask) < 0)
    {
        pcap_mgr.pRec->log(ERR, "Unable to compile the packet filter. Check the syntax.");
        return false;
    }
    
    //设置过滤器
    if (pcap_setfilter(devhd, &fcode) < 0)
    {
        pcap_mgr.pRec->log(ERR, "Error setting the filter.");
        return false;
    }
	return true;
}

void PcapDev::Close()
{
	pcap_mgr.pRec->log(INF, "Close %s...", Name.c_str());
	if (devhd) pcap_breakloop(devhd);
	m_pktHandler.Stop();
	Thread::Stop();
	closeDev();
}

void PcapDev::closeDev()
{
	if (devhd)
	{
		pcap_close(devhd);
		devhd = nullptr;
	}
}

/* 回调函数，当收到每一个数据包时会被pcap_loop所调用 */
/* 
void packet_handler(u_char *arg, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	PcapDev *dev = (PcapDev*)arg;

	dev->header = (struct pcap_pkthdr *)header;
	dev->pkt_data = pkt_data;
	dev->doPacket();
} 
*/

void PcapDev::Run()
{
	/*
	pcap_next_ex得到的header 和 pkt_data 由 libpcap 管理，你无需 / 不能手动释放；
	每次调用 pcap_next_ex()/pcap_loop() 会覆盖上一次的内存内容；
	调用 pcap_close() 关闭抓包句柄时，libpcap 释放整个缓冲区，指针失效；
	如需长期使用数据包，必须手动拷贝 pkt_data 到自己的内存，并自行管理释放。
	The pcap_next_ex return value can be:
    1 if the packet has been read without problems
    0 if the timeout set with pcap_open_live() has elapsed. In this case pkt_header and pkt_data don't point to a valid packet
    -1 if an error occurred
    -2 if EOF was reached reading from an offline capture
	*/
#if 0
	/* 开始捕捉 */
    pcap_loop(devhd, 0, packet_handler, (u_char*)this);
#else
	int ret;
	CapPacket capPkt;
	struct pcap_pkthdr *_header;
	const u_char *_pkt_data;

	/* 获取数据包 */
	while (!Thread::IsShutdown())
	{
	#if 0
		ret = pcap_next_ex(devhd, &header, &pkt_data);
		if (ret == 1) doPacket();
	#else
		ret = pcap_next_ex(devhd, &_header, &_pkt_data);
		if (ret == 1)
		{
			capPkt.header = *_header;
			capPkt.pkt_data = new u_char[_header->caplen];
			memcpy(capPkt.pkt_data, _pkt_data, _header->caplen);
			m_pktHandler.Enqueue(capPkt);
		}
	#endif
		//else if (ret == 0/* 超时时间到 */) continue;
		else if (ret == -1)
		{
			pcap_mgr.pRec->log(ERR, "[pcap_next_ex] %s", pcap_geterr(devhd));
			if (!openDev()) 
			{
				pcap_mgr.pRec->Abort("pcap error and failed reopen device!");
				break;
			}
		}
		else if (ret == -2)
		{
			pcap_mgr.pRec->log(INF, "EOF");
			break;
		}
	}
#endif
}



inline void PcapDev::doPacket()
{
	if (header->caplen < ETH_HEADER_LEN) return;
	
	eh = (ether_header*)pkt_data;
	u_short ether_type = ntohs(eh->ether_type);
	
	if (ether_type == ETH_TYPE_IP)
	{
		/* 获得IP数据包头部的位置 */
		ih = (ip_header*)(pkt_data + ETH_HEADER_LEN);	//以太网头部长度
		xlen = header->caplen - ETH_HEADER_LEN;
		doIP();
	}
	else if (ether_type == ETH_TYPE_802_1Q)
	{
		ether_802_1q_vlan *vlan = (ether_802_1q_vlan*)(pkt_data + ETH_HEADER_LEN);
		if (ntohs(vlan->original_ether_type) == ETH_TYPE_IP)
		{
			/* 获得IP数据包头部的位置 */
			ih = (ip_header*)(vlan + 1);
			xlen = header->caplen - ETH_HEADER_LEN - sizeof(ether_802_1q_vlan);
			doIP();
		}
	}
}

inline void PcapDev::doIP()
{
	if ((ih->ver_ihl & 0xf0) != 0x40/*IPv4*/) return;
	u_short ip_len = ntohs(ih->tlen);
	if (xlen < ip_len) return;

	/* 获得IP首部的长度 */
    u_int ih_len = (ih->ver_ihl & 0xf) * 4;
	u_char *ip_body = (u_char*)ih + ih_len;
	xlen = ip_len - ih_len;

	if (ih->proto == IPPROTO_TCP)
	{
		th = (tcp_header*)ip_body;
		doTCP();
	}
	else if (ih->proto == IPPROTO_UDP)
	{
		uh = (udp_header*)ip_body;
		doUDP();
	}
}

void PcapDev::doTCP()
{
	/* 将网络字节序列转换成主机字节序列 */
	sport = ntohs(th->sport);
    dport = ntohs(th->dport);

	u_int offset = (ntohs(th->offset4_reserved6_flag6) >> 12) * 4;
	u_char *data = (u_char*)th + offset;
	u_int data_len = xlen - offset;
	if (EnabledSCCP && (sport == 2000 || dport == 2000))
	{
		doSkinny(data, data_len);
	}
	else if (EnabledSIP && data_len > 16 && isSipMsg(data))
	{
		doSIP(data, data_len);
	}
	else if (EnabledH323 && (sport == 1720 || dport == 1720))
	{
		doH225(data, data_len);
	}
	else if (EnabledH323)
	{
		doH245(ih->saddr, sport, ih->daddr, dport, data, data_len);
	}
}

inline void PcapDev::doUDP()
{
    /* 将网络字节序列转换成主机字节序列 */
    sport = ntohs(uh->sport);
    dport = ntohs(uh->dport);

	u_char *data = (u_char*)uh + UDP_HEADER_LEN;
	u_int data_len = (u_int)ntohs(uh->len) - UDP_HEADER_LEN;
	if (data_len != xlen - UDP_HEADER_LEN) {
		pcap_mgr.pRec->logt("udp-data-len=%u, real-len=%u", data_len, xlen - UDP_HEADER_LEN);
		pcap_mgr.pRec->logt("======================================");
		data_len = xlen - UDP_HEADER_LEN;
	}

	if (EnabledSIP && data_len > 16 && isSipMsg(data)) 
	{
		doSIP(data, data_len);
	}
	else if ((sport&1) == 0 && (dport&1) == 0 && data_len > (sizeof(rtp_header) - 4))
	{/*rtcp use odd port and rtp use even port*/
		doRTP((rtp_header*)data, data_len);
	}
}
static bool isVersionAllowed(u_int version)
{
	return version == 0 ||		//ccm 6
		version == 0x11 ||		//ccm 7.1.2
		version == 0x12 ||		//ccm 7.1.3
		version == 0x13 ||		//ccm 8.0.2
		(version >= 0x14 && version < 0x3f);		//cucm 11.5 or cucm 12.5
}
//-----------------------------------------------------------------------------
inline void PcapDev::doSkinny(u_char *data, u_int len)
{
	// one-tcp-package may contain multi-skinnny-message
	while (len > sizeof(skinny_header))
	{
		pSkinnyMsgHdr = (skinny_header*)data;
		if (!isVersionAllowed(pSkinnyMsgHdr->version) ||
			(pSkinnyMsgHdr->datalen + 8) > len) break; // may not a skinny message
		u_char *body = data + sizeof(skinny_header);
		nSkinnyMsgBodyLen = pSkinnyMsgHdr->datalen - sizeof(u_int);
		len -= (sizeof(skinny_header) + nSkinnyMsgBodyLen);
		if (len < 0) break;

		//recorder.LogPool.Debug("Skinny %#x", hdr->messageid);
		switch (pSkinnyMsgHdr->messageid)
		{
		/*
		//DTMF可能导致混音结果不同步，这是因为DTMF转化为声音数据可能导致单边的数据量增加
		//而且每通电话DTMF的Payload可能都不一样
		case SKINNY_0x3_KEYPAD_BUTTON:
			if (recorder.FeatureDTMF)
				doSkinny0x3KeypadButton((skinny_0x3_keypad_button *)body);
			break;
		*/
		case SKINNY_0x8f_CALL_INFO:
			doSkinny0x8fCallInfo((skinny_0x8f_call_info *)body);
			break;
		case SKINNY_0x14a_CALL_INFO:
			doSkinny0x14aCallInfo((skinny_0x14a_call_info*)body);
			break;
		case SKINNY_0x105_OPEN_RECEIVE_CHANNEL:
			doSkinny0x105OpenReceiveChannel((skinny_0x105_open_receive_channel*)body);
			break;
		case SKINNY_0x8a_START_MEDIA_TRANSMISSION:
			doSkinny0x8aStartMediaTransmission((skinny_0x8a_start_media_transmission*)body);
			break;
		case SKINNY_0x22_OPEN_RECEIVE_CHANNEL_ACK:
			doSkinny0x22OpenReceiveChannelAck((skinny_0x22_open_receive_channel_ack*)body);
			break;
		case SKINNY_0x106_CLOSE_RECEIVE_CHANNEL:
			doSkinny0x106CloseReceiveChannel((skinny_0x106_close_receive_channel*)body);
			break;
		case SKINNY_0x111_CALL_STATE:
			doSkinny0x111CallState((skinny_0x111_call_state*)body);
			break;
		}

		data = body + nSkinnyMsgBodyLen;
	}
}

static inline std::string CallIdStr(u_int callid)
{
	std::string callid_s;
	Utils::StringUtil::Assign(callid_s, callid, true, 8);
	return callid_s;
}

inline void PcapDev::doSkinny0x3KeypadButton(skinny_0x3_keypad_button *keybutton)
{
	const u_int DtmfEvent[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 10, 11 };
	if (keybutton->KeypadButton > 15) return;

#if 0
	std::string callid = CallIdStr(keybutton->CallId);
	RTPEventPayload evt;
	evt.event = DtmfEvent[keybutton->KeypadButton];
	evt.end = 0;
	evt.duration = 160;
	evt.volume = 10; // -10dBm
	for (int i = 0; i < 5; i++) {
		pcap_mgr.pRec->OnAudioData(callid, true, 255, sizeof(RTPEventPayload), (const u_char*)&evt);
		evt.duration += 160;
	}
#endif
}
inline void PcapDev::doSkinny0x8fCallInfo(skinny_0x8f_call_info *callinfo)
{
	std::string callid = CallIdStr(callinfo->CallId);
	if (callinfo->CallingParty[0] != '\0' || callinfo->CalledParty[0] != '\0')
	{
		std::string ani(callinfo->CallingParty);
		std::string dnis(callinfo->CalledParty);
		pcap_mgr.pRec->UpdateCall(callid, ani, dnis);
	}
}
inline void PcapDev::doSkinny0x14aCallInfo(skinny_0x14a_call_info *callinfo)
{
	std::string callid = CallIdStr(callinfo->CallId);
	std::string ani = callinfo->Calling_Called;
	std::string dnis;
	char *bp = callinfo->Calling_Called, *p = bp + ani.length(); 
LABEL_FIND_DNIS:
	while (*p == '\0' && (u_int)(p - (char*)callinfo) < nSkinnyMsgBodyLen) p++;
	if ((u_int)(p - (char*)callinfo) < nSkinnyMsgBodyLen)
	{
		dnis = p;
		if (dnis == ani) 
		{
			p += dnis.length();
			goto LABEL_FIND_DNIS;
		}
	}
	if (!ani.empty() || !dnis.empty())
	{
		ICall *call = pcap_mgr.pRec->CreateCall(callid, CT_SKINNY, ani, dnis);
		pcap_mgr.pRec->UpdateCall(callid, ani, dnis);
	}
}
inline void PcapDev::doSkinny0x105OpenReceiveChannel(skinny_0x105_open_receive_channel *orc_msg)
{
	std::string callid = CallIdStr(orc_msg->CallId);
	std::string ani(""), dnis("");
	ICall *call = pcap_mgr.pRec->CreateCall(callid, CT_SKINNY, ani, dnis);

	lastOrcPassThruPartyId = orc_msg->PassThruPartyId;
	lastOrcCallId = orc_msg->CallId;
}
inline void PcapDev::doSkinny0x8aStartMediaTransmission(skinny_0x8a_start_media_transmission *smt_msg)
{
	u_int tmpCallId = smt_msg->CallId;
	ip_address tmpIP = smt_msg->RemoteIpAddress;
	u_short tmpPort = smt_msg->RemotePort;
	if (pSkinnyMsgHdr->version >= 0x11)
	{
		CCM7_skinny_0x8a_start_media_transmission *smt712 = (CCM7_skinny_0x8a_start_media_transmission*)smt_msg;
		tmpCallId = smt712->CallId;
		tmpIP = smt712->RemoteIpAddress;
		tmpPort = smt712->RemotePort;
	}

	std::string callid = CallIdStr(tmpCallId);
	pcap_mgr.pRec->SetMediaEndpoint(callid, true, tmpIP, tmpPort);
}
inline void PcapDev::doSkinny0x22OpenReceiveChannelAck(skinny_0x22_open_receive_channel_ack *orc_ack_msg)
{
	//some CCM 4.x or 5.x have no callid in OpenReceiveChannelAck, only PassThruPartyID, so must check first
	//printf("orc_ack_len=%u, real_len=%u\n", sizeof(skinny_0x22_open_receive_channel_ack), nSkinnyMsgBodyLen);
	//printf("this_pass=%x, last_pass=%x, last_call=%x\n", orc_ack_msg->PassThruPartyId, lastOrcPassThruPartyId, lastOrcCallId);
	u_int tmpCallId = nSkinnyMsgBodyLen == (sizeof(skinny_0x22_open_receive_channel_ack) - sizeof(u_int)) ?
		(orc_ack_msg->PassThruPartyId == lastOrcPassThruPartyId ? lastOrcCallId : 0) :
		orc_ack_msg->CallId;
	
	ip_address tmpIP = orc_ack_msg->IpAddress;
	u_short tmpPort = orc_ack_msg->Port;
	if (pSkinnyMsgHdr->version >= 0x11)
	{
		CCM7_skinny_0x22_open_receive_channel_ack *ack712 = (CCM7_skinny_0x22_open_receive_channel_ack*)orc_ack_msg;
		tmpCallId = ack712->CallId;
		tmpIP = ack712->IpAddress;
		tmpPort = ack712->Port;
	}

	std::string callid = CallIdStr(tmpCallId);
	pcap_mgr.pRec->SetMediaEndpoint(callid, false, tmpIP, tmpPort);
}
inline void PcapDev::doSkinny0x106CloseReceiveChannel(skinny_0x106_close_receive_channel *crc_msg)
{
	// can't delete call, CloseReceiveChannel may be caused by Hold call
	//std::string callid = CallIdStr(crc_msg->CallId);
	//pcap_mgr.pRec->DeleteCall(callid);
}
inline void PcapDev::doSkinny0x111CallState(skinny_0x111_call_state *callstate)
{
	std::string callid = CallIdStr(callstate->CallId);
	if (callstate->CallState == CS_On_Hook) pcap_mgr.pRec->DeleteCall(callid);
}

static inline bool AllowedPayload(u_char pt)
{
	return pt == PT_PCMU || pt == PT_PCMA || pt == PT_G722 || pt == PT_G729;
}

void PcapDev::doRTP(rtp_header *rh, u_int len)
{
	if (rh->ver != 2 || !AllowedPayload(rh->pt)) return;

	u_char *data = (u_char*)rh;
	u_int fac = 12 + 4 * rh->cc;
	if (len < fac) return;

	u_int ex_len = 0;
	if (rh->x == 1)
	{
		if (len < fac + 2) return;
		ex_len = (ntohs(*(u_short*)(data + 2 + fac)) + 1) * 4;
	}
	
	len -= fac + ex_len + (rh->padding == 1 ? data[len-1] : 0);
	data += fac + ex_len;

	if (((rh->pt == PT_PCMU || rh->pt == PT_PCMA) && len >= 80) 
		|| rh->pt == PT_G722
		|| (rh->pt == PT_G729 && len >= 10)
		//|| (rh->pt == recorder.DTMFPayload && recorder.FeatureDTMF)
		//DTMF可能导致混音结果不同步，这是因为DTMF转化为声音数据可能导致单边的数据量增加
		//而且每通电话DTMF的Payload可能都不一样
		)
	{
		pcap_mgr.pRec->TryEnqueue(
			ih->saddr, sport,
			ih->daddr, dport, 
			rh->pt, len, data, rh->timestamp);
	}
}

//-----------------------------------------------------------------------------
bool PcapDev::isSipMsg(u_char* data)
{
	return
		0 == strncasecmp((char*)data, "INVITE ", 7) ||
		0 == strncasecmp((char*)data, "ACK ", 4) ||
		0 == strncasecmp((char*)data, "BYE ", 4) ||
		0 == strncasecmp((char*)data, "CANCEL ", 7) ||
		0 == strncasecmp((char*)data, "UPDATE ", 7) ||
		0 == strncasecmp((char*)data, "SIP/2.0", 7);
}

void PcapDev::doSIP(u_char *data, u_int len)
{
	std::string str((const char*)data, len);
	pcap_mgr.pRec->logt("%s", str.c_str());
	if (Utils::StringUtil::Contains(str, "x-refci") || Utils::StringUtil::Contains(str, "record-invoker=auto")) {
		pcap_mgr.pRec->log(INF, "No pcap for cisco built-in-bridge");
		return;
	}

	sip_msg msg;
	memset(&msg, 0, sizeof(sip_msg));
	msg.buf = (char*)data;
	msg.len = len;
	if (-1 == parse_msg((char*)data, len, &msg)) {
		free_sip_msg(&msg);
		return;
	}
	if (msg.callid == NULL) {
		/* MSN may generate SIP like message
		BYE wingpang_gz@hotmail.com;{401ba1d6-7197-4f5b-ae04-a72b263f5dc4} 1
		so need check msg.callid exists
		*/
		free_sip_msg(&msg);
		return;
	}
	// 2022-8-26 a=recvonly or a=inactive may not for m=audio when both m=audio and m=video exists
	if (!strstr(str.c_str(), "m=video")) {
		if (strstr(str.c_str(), "a=sendonly") || strstr(str.c_str(), "a=recvonly") || strstr(str.c_str(), "a=inactive")) {
			free_sip_msg(&msg);
			return;
		}
	}

	if (msg.first_line.type == SIP_REQUEST) doSIPRequest(msg);
	else if (msg.first_line.type == SIP_REPLY) doSIPReply(msg);

	free_sip_msg(&msg);
}

bool PcapDev::getMediaInfoFromSdp(const struct sip_msg &msg, ip_address &ip, u_short &port)
{
	port = 0;
	std::string sLen(msg.content_length->body.s, msg.content_length->body.len);
	int sdp_len = atoi(sLen.c_str());
	if (sdp_len == 0) return false;
	/*
	const char *sdp_buf = msg.buf + msg.len - sdp_len;
	char *end = sdp_buf + sdp_len - 1;
	if (*end == '\r' || *end == '\n') *end = '\0';
	*/
	const char *sdp_buf = get_body((sip_msg*)&msg);

	sdp_t *sdp = NULL;
	if (-1 == sdp_init(&sdp))
	{
		if (sdp != NULL) sdp_free(sdp);
		return false;
	}

	/*
		2017/2/17 maodonghu 
		由于TCP包可能被分段segment, 如果INVITE消息被放在2个TCP包里传输（ackno相同），sdp解析就会失败
		但是有可能已经获取到了mediainfo，所以即使sdp_parse返回-1,也可以继续
		这是最简单和快速的解决SPAN录音问题的途径，否则要引入重组TCP包的逻辑，对程序改动太大，待以后完善。
	*/
	sdp_parse(sdp, sdp_buf);
	//if (0 == sdp_parse(sdp, sdp_buf))
	{
		sdp_connection_t* connection = sdp_connection_get(sdp, 0, 0);
		if (connection == NULL) connection = sdp_connection_get(sdp, -1, 0); // if no c= in media, use global c=
		if (connection == NULL) 
		{
			sdp_free(sdp);
			return false;
		}
		ip.from(connection->c_addr);
		for (int i = 0; ; i++)
		{
			char *m = sdp_m_media_get(sdp, i);
			if (m == NULL) break;
			char *szPort = sdp_m_port_get(sdp, i);
			if (0 == strcmp(m, "audio") && szPort != NULL)
			{
				port = atoi(szPort);
				break;
			}
		}
	}
	sdp_free(sdp);
	return port > 0;
}

bool PcapDev::getRemoteParty(const struct sip_msg& msg, std::string& name, std::string& number, bool& isCalling)
{
	/*
		--- Outbound ---
		CUCM place call(INVITE) as soon as possible before all number 013764309322 input.
		and give complete number in 200 OK Remote-Party-ID
		INVITE
		From: "蔡莉" <sip:9525@172.16.128.22>;tag=d55bcd58-0600-4db3-9007-a22ae9f15638
		To: sip:0@172.16.128.22;tag=303652~7b59372f-2586-4516-9c77-c066d6d40bbc-46150949
		Remote-Party-ID: <sip:13764309322@172.16.128.22>;party=called;screen=no;privacy=off

		--- Incoming ---
		手机呼入uccx, 3008为uccx虚拟号码，UPDATE消息更新calling号码为手机号
		UPDATE sip:78a7e8ab-4498-2d10-d353-603675326768@192.168.235.25:5060;transport=udp SIP/2.0
		From: <sip:3008@192.168.239.55>;tag=43438546~2403984e-946f-4782-8800-eae20aadeda5-44346614
		To: <sip:401@192.168.239.55>;tag=2c574113c42c0ee678a251d1-00ddfd9a
		Remote-Party-ID: <sip:13479749563@192.168.239.55>;party=calling;screen=yes;privacy=off
	*/
	const char* p = strstr(msg.buf, "Remote-Party-ID:");
	if (!p) return false;
	const char* lineEnd = strstr(p, "\r\n");
	if (!lineEnd) return false;

	str remotePartyID;
	remotePartyID.s = (char*)(p + 16);
	remotePartyID.len = lineEnd - remotePartyID.s;
	name_addr na;
	if (0 != parse_nameaddr(&remotePartyID, &na)) return false;

	sip_uri uri;
	if (na.name.s != NULL) name.assign(na.name.s, na.name.len);
	if (na.uri.s != NULL && 0 == parse_uri(na.uri.s, na.uri.len, &uri) && uri.user.s != NULL) number.assign(uri.user.s, uri.user.len);

	p = strstr(msg.buf, "party=");
	if (!p) return false;
	const char* pEnd = strchr(p, ';');
	if (!pEnd) pEnd = lineEnd;
	std::string party(p + 6, pEnd - p - 6);
	if (party == "calling") isCalling = true;
	else if (party == "called") isCalling = false;
	else return false;

	std::string callid(msg.callid->body.s, msg.callid->body.len);
	pcap_mgr.pRec->log(INF, "[%s] Remote-Party-ID: %s<%s>;party=%s", callid.c_str(), name.c_str(), number.c_str(), party.c_str());
	return true;
}

void PcapDev::doSIPRequest(const struct sip_msg &msg)
{
	std::string callid(msg.callid->body.s, msg.callid->body.len);
	std::string method(msg.first_line.u.request.method.s, msg.first_line.u.request.method.len);

	if (msg.REQ_METHOD == METHOD_INVITE)
	{
		std::string ani, dnis, callerName, calledName;
		sip_uri uri;
		name_addr na;
		if (0 == parse_from_header((sip_msg*)&msg))
		{
			to_body *from_b = get_from(&msg);
			if (0 == parse_nameaddr(&from_b->body, &na) && na.name.s != NULL) callerName.assign(na.name.s, na.name.len);
			if (0 == parse_uri(from_b->uri.s, from_b->uri.len, &uri) && uri.user.s != NULL) ani.assign(uri.user.s, uri.user.len);
		}
		if (0 == parse_to_header((sip_msg*)&msg))
		{
			to_body *to_b = get_to(&msg);
			if (0 == parse_nameaddr(&to_b->body, &na) && na.name.s != NULL) calledName.assign(na.name.s, na.name.len);
			if (0 == parse_uri(to_b->uri.s, to_b->uri.len, &uri) && uri.user.s != NULL) dnis.assign(uri.user.s, uri.user.len);
		}
		pcap_mgr.pRec->log(INF, "[%s] INVITE from=%s<%s>, to=%s<%s>", 
			callid.c_str(), 
			callerName.c_str(), ani.c_str(),
			calledName.c_str(), dnis.c_str()
		);
		if (!pcap_mgr.pRec->IsCallExists(callid))
		{
			//hold re-invite的from/to跟incoming invite的from/to相反, 而且hold之前可能已经根据UPDATE的remotePartyID更新过主被叫，此处不能再更新
			pcap_mgr.pRec->CreateCall(callid, CT_SIP, ani, dnis);
			pcap_mgr.pRec->UpdateCaller(callid, callerName, ani);
			pcap_mgr.pRec->UpdateCalled(callid, calledName, dnis);
		}
		
		if (msg.content_type != NULL 
			&& 0 == strncmp(msg.content_type->body.s, "application/sdp", msg.content_type->body.len)
			&& msg.content_length != NULL)
		{
			ip_address mip;
			u_short mport = 0;
			if (getMediaInfoFromSdp(msg, mip, mport)) pcap_mgr.pRec->SetMediaEndpoint(callid, true, mip, mport);
		}
	}
	else if (msg.REQ_METHOD == METHOD_ACK)
	{
		pcap_mgr.pRec->log(INF, "[%s] ACK", callid.c_str());
		
		if (msg.content_type != NULL 
			&& 0 == strncmp(msg.content_type->body.s, "application/sdp", msg.content_type->body.len)
			&& msg.content_length != NULL) 
		{
			ip_address mip;
			u_short mport = 0;
			if (getMediaInfoFromSdp(msg, mip, mport)) pcap_mgr.pRec->SetMediaEndpoint(callid, true, mip, mport);
		}
	}
	else if (msg.REQ_METHOD == METHOD_BYE || msg.REQ_METHOD == METHOD_CANCEL)
	{
		pcap_mgr.pRec->log(INF, "[%s] %s", callid.c_str(), method.c_str());
		pcap_mgr.pRec->DeleteCall(callid);
	}
	else if (method == "UPDATE")
	{
		pcap_mgr.pRec->log(INF, "[%s] UPDATE", callid.c_str());
		std::string rname, rnumber;
		bool isCalling;
		if (getRemoteParty(msg, rname, rnumber, isCalling)) {
			if (isCalling) pcap_mgr.pRec->UpdateCaller(callid, rname, rnumber);
			else pcap_mgr.pRec->UpdateCalled(callid, rname, rnumber);
		}
	}
}

void PcapDev::doSIPReply(const struct sip_msg &msg)
{
	struct cseq_body *cseq_b = get_cseq(&msg);
	if (cseq_b == NULL || strncmp(cseq_b->method.s, "INVITE", cseq_b->method.len) != 0) return;

	std::string callid(msg.callid->body.s, msg.callid->body.len);
	std::string reason(msg.first_line.u.reply.reason.s, msg.first_line.u.reply.reason.len);

	if (msg.REPLY_STATUS == 200 || msg.REPLY_STATUS == 180 || msg.REPLY_STATUS == 183)
	{
		pcap_mgr.pRec->log(INF, "[%s] %u %s", callid.c_str(), msg.REPLY_STATUS, reason.c_str());
		std::string rname, rnumber;
		bool isCalling;
		if (getRemoteParty(msg, rname, rnumber, isCalling)) {
			if (isCalling) pcap_mgr.pRec->UpdateCaller(callid, rname, rnumber);
			else pcap_mgr.pRec->UpdateCalled(callid, rname, rnumber);
		}

		if (msg.content_type != NULL 
			&& 0 == strncmp(msg.content_type->body.s, "application/sdp", msg.content_type->body.len)
			&& msg.content_length != NULL) {
			ip_address mip;
			u_short mport = 0;
			if (getMediaInfoFromSdp(msg, mip, mport)) pcap_mgr.pRec->SetMediaEndpoint(callid, false, mip, mport);
		}
	}
	else if (msg.REPLY_STATUS >= 300)
	{
		pcap_mgr.pRec->log(INF, "[%s] %u %s", callid.c_str(), msg.REPLY_STATUS, reason.c_str());
		pcap_mgr.pRec->DeleteCall(callid);
	}
}

//-----------------------------------------------------------------------------
void PcapDev::doH225(u_char *data, u_int len)
{
	std::string callid, ani;
	ICall *call = NULL;

	while (len > 9)
	{
		tpkt_header *ttt = (tpkt_header *)data;
		ttt->length = ntohs(ttt->length);
		if (ttt->version != 3 || ttt->reserved != 0 || ttt->length > len) return;

		H323_Q931_Decode_Result ret = h323_q931_decode(data + sizeof(tpkt_header), ttt->length - sizeof(tpkt_header));
		if (ret.Result == true)
		{
			callid = CallIdStr(ret.CallRef);
			switch (ret.MsgType)
			{
			case Q931::SetupMsg:
				pcap_mgr.pRec->CreateCall(callid, CT_H323, ret.Caller, ret.Called);
				break;
			/*case Q931::FacilityMsg:
				pcap_mgr.pRec->CreateOrUpdateCall(callid, CT_H323, ret.Caller, ret.Called);

				if (ret.FastStart_ForwardMediaChannelIP != 0)
				{
					pcap_mgr.pRec->SetMediaEndpoint(callid, true,  *(ip_address*)&ret.FastStart_ForwardMediaChannelIP, ret.FastStart_ForwardMediaChannelPort);
					pcap_mgr.pRec->SetMediaEndpoint(callid, false, *(ip_address*)&ret.FastStart_ReverseMediaChannelIP, ret.FastStart_ReverseMediaChannelPort);
				}
				else if (ret.H245_IP != 0 && ret.H245_Port != 0)
				{
					if (call->pData != NULL) ((H245CallData*)call->pData)->h245Assign(*(ip_address*)&ret.H245_IP, ret.H245_Port);
				}
				break;
			*/
			case Q931::CallProceedingMsg:
			case Q931::AlertingMsg:
				if (ret.FastStart_ForwardMediaChannelIP != 0)
				{
					pcap_mgr.pRec->SetMediaEndpoint(callid, true,  *(ip_address*)&ret.FastStart_ForwardMediaChannelIP, ret.FastStart_ForwardMediaChannelPort);
					pcap_mgr.pRec->SetMediaEndpoint(callid, false, *(ip_address*)&ret.FastStart_ReverseMediaChannelIP, ret.FastStart_ReverseMediaChannelPort);
				}
				else if (ret.H245_IP != 0 && ret.H245_Port != 0)
				{
					h245Register(callid, *(ip_address*)&ret.H245_IP, ret.H245_Port);
				}
				break;
			case Q931::ConnectMsg:
				if (ret.FastStart_ForwardMediaChannelIP != 0)
				{
					pcap_mgr.pRec->SetMediaEndpoint(callid, true,  *(ip_address*)&ret.FastStart_ForwardMediaChannelIP, ret.FastStart_ForwardMediaChannelPort);
					pcap_mgr.pRec->SetMediaEndpoint(callid, false, *(ip_address*)&ret.FastStart_ReverseMediaChannelIP, ret.FastStart_ReverseMediaChannelPort);
				}
				else if (ret.H245_IP != 0 && ret.H245_Port != 0)
				{
					h245Register(callid, *(ip_address*)&ret.H245_IP, ret.H245_Port);
				}
				break;
			case Q931::ReleaseMsg:
			case Q931::ReleaseCompleteMsg:
				pcap_mgr.pRec->DeleteCall(callid);
				break;
			case Q931::NotifyMsg:
				ani = "";
				pcap_mgr.pRec->UpdateCall(callid, ani, ret.Called);
				break;
			}
		}

		data += ttt->length;
		len -= ttt->length;
	}
}

bool PcapDev::doH245(const ip_address& sip, u_short sport, const ip_address& dip, u_short dport, u_char* data, u_int len)
{
	IP_PORT_PAIR saddr(sip, sport), daddr(dip, dport);

	std::lock_guard lock(h245_mtx);
	/*
	if (sport == 60782 || dport == 60782) pcap_mgr.pRec->log(INF, "%d.%d.%d.%d, maplen=%u",
		sip.byte1, sip.byte2, sip.byte3, sip.byte4, H245_Calls.size());
	*/
	auto it = H245_Calls.find(saddr);
	if (it != H245_Calls.end())
	{
		h245Decode(it->second, data, len, true);
		H245_Calls.erase(it);
		return true;
	}

	it = H245_Calls.find(daddr);
	if (it != H245_Calls.end())
	{
		h245Decode(it->second, data, len, false);
		H245_Calls.erase(it);
		return true;
	}
	return false;
}

void PcapDev::h245Register(const std::string& callid, const ip_address& h245_ip, u_short h245_port)
{
	std::lock_guard lock(h245_mtx);
	IP_PORT_PAIR h245_address(h245_ip, h245_port);
	H245_Calls[h245_address] = callid;
	pcap_mgr.pRec->log(INF, "[%s] H245 address %d.%d.%d.%d:%d",
		callid.c_str(),
		h245_ip.byte1, h245_ip.byte2, h245_ip.byte3, h245_ip.byte4,
		h245_port);

	if (H245_Calls.size() > 5000) {
		//2020/8/22 hhmmdd
		//TODO: H245_Calls clean：if no h245 decode occurred before call end.
		pcap_mgr.pRec->log(WRN, "%u h245 info items reached.", H245_Calls.size());
	}
}

void PcapDev::h245Decode(const std::string& callid, u_char* data, u_int len, bool srcOrdst)
{
	if (len < 9) return;
	tpkt_header* ttt = (tpkt_header*)data;
	ttt->length = ntohs(ttt->length);
	if (ttt->version != 3 || ttt->reserved != 0 || ttt->length != len) return;

	data += sizeof(tpkt_header);
	len -= sizeof(tpkt_header);
	if (len < 16) return;

	ip_address rtp_ip;
	u_short rtp_port;
	try
	{
		if (h323_h245_decode(data, len, *(u_int*)&rtp_ip, rtp_port))
		{
			pcap_mgr.pRec->SetMediaEndpoint(callid, srcOrdst, rtp_ip, rtp_port);
		}
	}
	catch (SEH_Exception ex)
	{
		pcap_mgr.pRec->log(ERR, "[%s] H245 decode exception: data length=%u", callid.c_str(), len);
		throw;
	}
}
