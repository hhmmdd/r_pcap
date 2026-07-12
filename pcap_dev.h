#ifndef PCAP_DEV_H
#define PCAP_DEV_H

#include <pcap.h>
#include <list>
#include "packet.h"
#include "skinny.h"
#include <Utils/Thread.h>
#include <Utils/Handler.h>

#include <sipparser/parser/msg_parser.h>
#include <sipparser/parser/parse_uri.h>
#include <sipparser/parser/parse_from.h>
#include <sipparser/parser/parse_to.h>
#include <sipparser/parser/parse_nameaddr.h>
#include <sipparser/sdp/sdp.h>
#include <sipparser/mem/mem.h>
#include <h323lib/h323lib.h>

class PcapDev : public Utils::Thread
{ 
public:
	PcapDev(const char *name, u_int mask);
	~PcapDev(void);

	bool Open();
	bool OpenFile();
	void Close();
	void Run();

	std::string Name;
	std::list<std::string> IPs;
	bool EnabledSIP;
	bool EnabledH323;
	bool EnabledSCCP; //SKINNY

private:
	struct CapPacket
	{
		pcap_pkthdr header;
		u_char *pkt_data;
	};
	Utils::Handler<CapPacket> m_pktHandler;

private:
	pcap_t *devhd{ nullptr };
	u_int netmask;
	bool openDev();
	void closeDev();
	bool setFilter(const char *filter);

	const pcap_pkthdr *header;
	const u_char *pkt_data;
	const ether_header *eh;
	const ip_header *ih;
	const tcp_header *th;
	const udp_header *uh;
	u_int xlen;
	u_short sport;
    u_short dport;

	skinny_header *pSkinnyMsgHdr;
	u_int nSkinnyMsgBodyLen;
	u_int lastOrcPassThruPartyId;
	u_int lastOrcCallId;
	
	friend void packet_handler(u_char *arg, const struct pcap_pkthdr *header, const u_char *pkt_data);
	void doPacket();
	void doIP();
	void doTCP();
	void doUDP();
	void doRTP(rtp_header *rh, u_int len);

	void doSkinny(u_char *data, u_int len);
	void doSkinny0x3KeypadButton(skinny_0x3_keypad_button *keybutton);
	void doSkinny0x8fCallInfo(skinny_0x8f_call_info *callinfo);
	void doSkinny0x14aCallInfo(skinny_0x14a_call_info *callinfo);
	void doSkinny0x105OpenReceiveChannel(skinny_0x105_open_receive_channel *orc_msg);
	void doSkinny0x8aStartMediaTransmission(skinny_0x8a_start_media_transmission *smt_msg);
	void doSkinny0x22OpenReceiveChannelAck(skinny_0x22_open_receive_channel_ack *orc_ack_msg);
	void doSkinny0x106CloseReceiveChannel(skinny_0x106_close_receive_channel *crc_msg);
	void doSkinny0x111CallState(skinny_0x111_call_state *callstate);
	
	bool isSipMsg(u_char* data);
	void doSIP(u_char *data, u_int len);
	void doSIPRequest(const struct sip_msg &msg);
	void doSIPReply(const struct sip_msg &msg);
	bool getRemoteParty(const struct sip_msg& msg, std::string& name, std::string& number, bool& isCalling);
	bool getMediaInfoFromSdp(const struct sip_msg& msg, ip_address& ip, u_short& port);

private:
	void doH225(u_char* data, u_int len);

	bool doH245(const ip_address& sip, u_short sport, const ip_address& dip, u_short dport, u_char* data, u_int len);
	std::map<IP_PORT_PAIR, std::string> H245_Calls;
	std::mutex h245_mtx;
	void h245Decode(const std::string &callid, u_char* data, u_int len, bool srcOrdst);
	void h245Register(const std::string& callid, const ip_address& h245_ip, u_short h245_port);
};

#endif
