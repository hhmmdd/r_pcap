#include "h323lib.h"
#include "asn/h245.h"

H323_Q931_Decode_Result h323_q931_decode(u_char *data, u_int len)
{
	H323_Q931_Decode_Result ret;
	ret.Result = false;
	
	Q931 q931_msg;
	if (!q931_msg.Decode(data, len)) return ret;
	ret.Result = true;
	ret.MsgType = q931_msg.GetMessageType();

	switch (ret.MsgType)
	{
	case Q931::SetupMsg:
	{
		Q931::InformationTransferCapability capability;
		unsigned transferRate;
		if (!q931_msg.GetBearerCapabilities(capability, transferRate) || 
			(capability != Q931::TransferSpeech && capability != Q931::Transfer3_1kHzAudio)) return ret;

		ret.CallRef = q931_msg.GetCallReference();
		q931_msg.GetCallingPartyNumber(ret.Caller);
		q931_msg.GetCalledPartyNumber(ret.Called);
		break;
	}
	case Q931::CallProceedingMsg:
		ret.CallRef = q931_msg.GetCallReference();
		q931_msg.GetCallProceedingInfo(ret);
		break;
	case Q931::AlertingMsg:
		ret.CallRef = q931_msg.GetCallReference();
		q931_msg.GetAlertingInfo(ret);
		break;
	case Q931::ConnectMsg:
		ret.CallRef = q931_msg.GetCallReference();
		q931_msg.GetConnectInfo(ret);
		break;
	case Q931::ReleaseMsg:
	case Q931::ReleaseCompleteMsg:
		ret.CallRef = q931_msg.GetCallReference();
		break;
	case Q931::NotifyMsg:
		if (q931_msg.GetConnectedNumber(ret.Called))
		{
			ret.CallRef = q931_msg.GetCallReference();
		}
		break;
	case Q931::FacilityMsg: //Avaya?
		ret.CallRef = q931_msg.GetCallReference();
		q931_msg.GetFacilityInfo(ret);
		break;
	}
	return ret;
}

bool h323_h245_decode(u_char *data, u_int len, u_int &rtp_ip, u_short &rtp_port)
{
	PPER_Stream strm(data, len);
	H245_MultimediaSystemControlMessage h245_msg;
	if (!h245_msg.Decode(strm) || 
		h245_msg.GetTag() != H245_MultimediaSystemControlMessage::e_response ||
		((H245_ResponseMessage &)h245_msg).GetTag() != H245_ResponseMessage::e_openLogicalChannelAck
		) return false;
	
	H245_OpenLogicalChannelAck &open_ack_msg = (H245_OpenLogicalChannelAck &)(H245_ResponseMessage &)h245_msg;
	H245_H2250LogicalChannelAckParameters &ack_param = (H245_H2250LogicalChannelAckParameters &)open_ack_msg.m_forwardMultiplexAckParameters;
	H245_UnicastAddress_iPAddress &addr = (H245_UnicastAddress_iPAddress &)(H245_UnicastAddress &)ack_param.m_mediaChannel;
	memcpy(&rtp_ip, (const BYTE *)addr.m_network, sizeof(u_int));
	rtp_port = addr.m_tsapIdentifier;
	return true;
}
