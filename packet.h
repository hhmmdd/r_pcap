#ifndef PACKET_H
#define PACKET_H

#ifdef WIN32
#include <winsock2.h>
#endif

#include <ip_address.h>

#define ETH_HEADER_LEN		14
#define IP_HEADER_LEN		20
#define TCP_HEADER_LEN		20
#define UDP_HEADER_LEN		8

#define ETH_TYPE_IP			0x0800
#define ETH_TYPE_802_1Q		0x8100

#pragma pack(1)    //进入字节对齐方式

/* Ethernet header */
struct ether_header
{
	u_char dmac[6];			// destination hardware address
	u_char smac[6];			// source hardware address
	u_short ether_type;		// ether type.              
};

struct ether_802_1q_vlan
{
	u_short priority3_cfi1_id12;
	u_short original_ether_type;
};

/* IPv4 首部 */
struct ip_header
{
    u_char  ver_ihl;        // 版本 (4 bits) + 首部长度 (4 bits)
    u_char  tos;            // 服务类型(Type of service) 
    u_short tlen;           // 总长(Total length) 
    u_short identification; // 标识(Identification)
    u_short flags_fo;       // 标志位(Flags) (3 bits) + 段偏移量(Fragment offset) (13 bits)
    u_char  ttl;            // 存活时间(Time to live)
    u_char  proto;          // 协议(Protocol)
    u_short crc;            // 首部校验和(Header checksum)
    ip_address  saddr;      // 源地址(Source address)
    ip_address  daddr;      // 目的地址(Destination address)
    u_int   op_pad;         // 选项与填充(Option + Padding)
};

/* UDP 首部 */
struct udp_header
{
    u_short sport;          // 源端口(Source port)
    u_short dport;          // 目的端口(Destination port)
    u_short len;            // UDP数据包长度(Datagram length=headerLen+dataLen)
    u_short crc;            // 校验和(Checksum)
};

/* TCP数据包头 */
struct tcp_header
{                     
    u_short sport;			//源端口
    u_short dport;			//目的端口
    u_int	seqno;			//序号
    u_int	ackno;			//确认号
    u_short offset4_reserved6_flag6;    //头部长度+保留+标志
    u_short window;			//窗口大小
    u_short checksum;		//校验和
    u_short urgentptr;		//紧急指针
    u_int	option_;		//选项+填充
};

/* Transport Packet */
struct tpkt_header
{
	u_char	version;
	u_char	reserved;
	u_short	length;			//including the tpkt header 
};

/* RTP数据包头 */
struct rtp_header
{
	u_char cc		:4;		//CSRC count, The number of CSRC identifiers that follow the fixed header.
	u_char x		:1;		//Extension, If set, the fixed header is followed by exactly one header extension.
	u_char padding	:1;		//If set, this packet contains one or more additional padding bytes at the end which are not part of the payload. The last byte of the padding contains a count of how many padding bytes should be ignored. Padding may be needed by some encryption algorithms with fixed block sizes or for carrying several RTP packets in a lower-layer protocol data unit.
	u_char ver		:2;		//RTP version number. Always set to 2.

	u_char pt		:7;		//Payload Type.
	u_char marker	:1;		

	u_short seq_num;		//Sequence Number. 16 bits.
	u_int timestamp;
	u_int ssrc;				//Synchronization source
	u_int csrc[1];			//Contributing source. An array of 0 to 15 CSRC elements identifying the contributing sources for the payload contained in this packet.
};

#pragma pack()    //恢复默认对齐方式

#endif
