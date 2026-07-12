#ifndef H323LIB_H
#define H323LIB_H

#ifdef WIN32
#include <winsock2.h>
#endif

#include <string>
#include "q931.h"

struct H323_Q931_Decode_Result
{
	bool Result;
	Q931::MsgTypes MsgType;
	
	u_int CallRef;
	std::string Caller;
	std::string Called;

	u_long	H245_IP;
	u_short H245_Port;

	u_long	FastStart_ForwardMediaChannelIP;
	u_short	FastStart_ForwardMediaChannelPort;

	u_long	FastStart_ReverseMediaChannelIP;
	u_short	FastStart_ReverseMediaChannelPort;

	H323_Q931_Decode_Result() : 
		Result(false), CallRef(0),
		H245_IP(0), H245_Port(0),
		FastStart_ForwardMediaChannelIP(0), FastStart_ForwardMediaChannelPort(0),
		FastStart_ReverseMediaChannelIP(0), FastStart_ReverseMediaChannelPort(0)
	{}
};

H323_Q931_Decode_Result h323_q931_decode(u_char *data, u_int len);
bool h323_h245_decode(u_char *data, u_int len, u_int &rtp_ip, u_short &rtp_port);

#endif
