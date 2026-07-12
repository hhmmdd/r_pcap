/*
 * q931.cxx
 *
 * Q.931 protocol handler
 *
 * Changed by maodonghu base-on Open H323 Library for parse only
 *
 */

#include "q931.h"
#include "h323lib.h"

const char* GetIEName(Q931::IETypes ieType)
{
	switch (ieType)
	{
	case Q931::BearerCapabilityIE:		return "Bearer-Capability";
	case Q931::CauseIE:					return "Cause";
	case Q931::FacilityIE:				return "Facility";
	case Q931::ProgressIndicatorIE:		return "Progress-Indicator";
	case Q931::CallStateIE:				return "Call-State";
	case Q931::DisplayIE:				return "Display";
	case Q931::SignalIE:				return "Signal";
	case Q931::KeypadIE:				return "Keypad";
	case Q931::ConnectedNumberIE:		return "Connected-Number";
	case Q931::CallingPartyNumberIE:	return "Calling-Party-Number";
	case Q931::CalledPartyNumberIE:		return "Called-Party-Number";
	case Q931::RedirectingNumberIE:		return "Redirecting-Number";
	case Q931::ChannelIdentificationIE:	return "Channel-Identification";
	case Q931::UserUserIE:				return "User-User";
	}
	return "Unknown IE type";
}

const char* GetCauseName(Q931::CauseValues cause)
{
	switch (cause)
	{
    case Q931::UnallocatedNumber:           return "Unallocated number";
    case Q931::NoRouteToNetwork:            return "No route to network";
    case Q931::NoRouteToDestination:        return "No route to destination";
    case Q931::SendSpecialTone:             return "Send special tone";
    case Q931::MisdialledTrunkPrefix:       return "Misdialled trunk prefix";
    case Q931::ChannelUnacceptable:         return "Channel unacceptable";
    case Q931::NormalCallClearing:          return "Normal call clearing";
    case Q931::UserBusy:                    return "User busy";
    case Q931::NoResponse:                  return "No response";
    case Q931::NoAnswer:                    return "No answer";
    case Q931::SubscriberAbsent:            return "Subscriber absent";
    case Q931::CallRejected:                return "Call rejected";
    case Q931::NumberChanged:               return "Number changed";
    case Q931::Redirection:                 return "Redirection";
    case Q931::ExchangeRoutingError:        return "Exchange routing error";
    case Q931::NonSelectedUserClearing:     return "Non selected user clearing";
    case Q931::DestinationOutOfOrder:       return "Destination out of order";
    case Q931::InvalidNumberFormat:         return "Invalid number format";
    case Q931::FacilityRejected:            return "Facility rejected";
    case Q931::StatusEnquiryResponse:       return "Status enquiry response";
    case Q931::NormalUnspecified:           return "Normal unspecified";
    case Q931::NoCircuitChannelAvailable:   return "No circuit/channel available";
    case Q931::NetworkOutOfOrder:           return "Network out of order";
    case Q931::TemporaryFailure:            return "Temporary failure";
    case Q931::Congestion:                  return "Congestion";
    case Q931::RequestedCircuitNotAvailable:return "RequestedCircuitNotAvailable";
    case Q931::ResourceUnavailable:         return "Resource unavailable";
    case Q931::ServiceOptionNotAvailable:   return "Service or option not available";
    case Q931::InvalidCallReference:        return "Invalid call reference";
    case Q931::IncompatibleDestination:     return "Incompatible destination";
    case Q931::IENonExistantOrNotImplemented:return "IE non-existent or not implemented";
    case Q931::TimerExpiry:                 return "Recovery from timer expiry";
    case Q931::ProtocolErrorUnspecified:    return "Protocol error: unspecified";
    case Q931::InterworkingUnspecified:     return "Interworking: unspecified";
	}
	return "Unknown cause";  
}


///////////////////////////////////////////////////////////////////////////////

Q931::Q931()
{
	protocolDiscriminator = 8;  // Q931 always has 00001000
	messageType = NationalEscapeMsg;
	fromDestination = false;
	callReference = 0;
}


bool Q931::Decode(u_char *data, u_int len)
{
	// Clear all existing data before reading new
	InformationElements.clear();
	if (len < 5) // Packet too short
		return false;

	protocolDiscriminator = data[0];
	if (protocolDiscriminator != 8) // Q.931 always 0x08
		return false;

	if (data[1] != 2) // Call reference must be 2 bytes long
		return false;

	callReference = ((data[2]&0x7f) << 8) | data[3];
	fromDestination = (data[2]&0x80) != 0;

	messageType = (MsgTypes)data[4];

	// Have preamble, start mapping the InformationElements
	u_int offset = 5;
	while (offset < len) 
	{
		// Get field discriminator
		int discriminator = data[offset++];
		// For discriminator with high bit set there is no data
		if ((discriminator&0x80) != 0) continue;
		
		u_int ie_len = data[offset++];
		if (discriminator == UserUserIE) 
		{
			// Special case of User-user field. See 7.2.2.31/H.225.0v4.
			ie_len <<= 8;
			ie_len |= data[offset++];

			// we also have a protocol discriminator, which we ignore
			offset++;

			// before decrementing the length, make sure it is not zero
			if (ie_len == 0) return false;

			// adjust for protocol discriminator
			ie_len--;
		}
		if (offset + ie_len > len) return false;

		IE_ITEM item;
		item.ie_len = ie_len;
		item.ie_data = data + offset;
		InformationElements[(IETypes)discriminator] = item;

		offset += ie_len;
	}
	return true;
}

const char* Q931::GetMessageTypeName() const
{
	switch (messageType) 
	{
    case AlertingMsg:			return "Alerting";
    case CallProceedingMsg:		return "CallProceeding";
    case ConnectMsg:			return "Connect";
    case ConnectAckMsg:			return "ConnectAck";
    case ProgressMsg:			return "Progress";
    case SetupMsg:				return "Setup";
    case SetupAckMsg:			return "SetupAck";
    case FacilityMsg:			return "Facility";
    case ReleaseCompleteMsg:	return "ReleaseComplete";
    case StatusEnquiryMsg:		return "StatusEnquiry";
    case StatusMsg:				return "Status";
    case InformationMsg:		return "Information";
    case NationalEscapeMsg:		return "Escape";
    case NotifyMsg:				return "NotifyMsg";
    case ResumeMsg:				return "ResumeMsg";
    case ResumeAckMsg:			return "ResumeAckMsg";
	case ResumeRejectMsg:		return "ResumeRejectMsg";
    case SuspendMsg:			return "SuspendMsg";
    case SuspendAckMsg:			return "SuspendAckMsg";
    case SuspendRejectMsg:		return "SuspendRejectMsg";
    case UserInformationMsg:	return "UserInformationMsg";
    case DisconnectMsg:			return "DisconnectMsg";
    case ReleaseMsg:			return "ReleaseMsg";
    case RestartMsg:			return "RestartMsg";
    case RestartAckMsg:			return "RestartAckMsg";
    case SegmentMsg:			return "SegmentMsg";
    case CongestionCtrlMsg:		return "CongestionCtrlMsg";
	}
	return "Unknown message type";
}

Q931::IE_ITEM Q931::GetIE(IETypes ie) const
{
	std::map<IETypes, IE_ITEM>::const_iterator it = InformationElements.find(ie);
	return it == InformationElements.end() ? IE_ITEM() : it->second;
}

bool Q931::GetBearerCapabilities(InformationTransferCapability & capability,
                                 unsigned & transferRate,
                                 unsigned * codingStandard,
                                 unsigned * userInfoLayer1)
{
	IE_ITEM item = GetIE(BearerCapabilityIE);
	if (item.ie_len < 2) return false;

	capability = (InformationTransferCapability)(item.ie_data[0] & 0x1f);
	if (codingStandard != NULL) *codingStandard = (item.ie_data[0] >> 5) & 3;

	int nextByte = 2;
	switch (item.ie_data[1]) 
	{
    case 0x90 :
		transferRate = 1;
		break;
    case 0x91 :
		transferRate = 2;
		break;
    case 0x93 :
		transferRate = 6;
		break;
    case 0x95 :
		transferRate = 24;
		break;
    case 0x97 :
		transferRate = 30;
		break;
    case 0x18 :
		if (item.ie_len < 3) return false;
		transferRate = item.ie_data[2]&0x7f;
		nextByte = 3;
		break;
    default :
		return false;
	}

	if (userInfoLayer1 != NULL)
		*userInfoLayer1 = item.ie_len >= nextByte && ((item.ie_data[nextByte]>>5)&3) == 1 ? (item.ie_data[nextByte]&0x1f) : 0;

	return true;
}

Q931::CauseValues Q931::GetCause(unsigned * standard, unsigned * location) const
{
	IE_ITEM item = GetIE(CauseIE);
	if (item.ie_len < 2) return ErrorInCauseIE;

	if (standard != NULL)
		*standard = (item.ie_data[0] >> 5)&3;
	if (location != NULL)
		*location = item.ie_data[0]&15;

	if ((item.ie_data[0]&0x80) != 0)
		return (CauseValues)(item.ie_data[1]&0x7f);

	// Allow for optional octet
	if (item.ie_len < 3)
		return ErrorInCauseIE;

	return (CauseValues)(item.ie_data[2]&0x7f);
}

Q931::CallStates Q931::GetCallState(unsigned * standard) const
{
	IE_ITEM item = GetIE(CallStateIE);
	if (item.ie_len == 0) return CallState_ErrorInIE;

	if (standard != NULL)
		*standard = (item.ie_data[0] >> 6)&3;

	return (CallStates)(item.ie_data[0]&0x3f);
}

Q931::SignalInfo Q931::GetSignalInfo() const
{
	IE_ITEM item = GetIE(SignalIE);
	if (item.ie_len == 0) SignalErrorInIE;

	return (SignalInfo)item.ie_data[0];
}

std::string Q931::GetKeypad() const
{
	IE_ITEM item = GetIE(Q931::KeypadIE);
	if (item.ie_len == 0) return "";

	return std::string((const char*)item.ie_data, item.ie_len);
}

bool Q931::GetProgressIndicator(unsigned & description,
                                unsigned * codingStandard,
                                unsigned * location) const
{
	IE_ITEM item = GetIE(ProgressIndicatorIE);
	if (item.ie_len < 2) return false;

	if (codingStandard != NULL)
		*codingStandard = (item.ie_data[0]>>5)&0x03;
	if (location != NULL)
		*location = item.ie_data[0]&0x0f;
	description = item.ie_data[1]&0x7f;

	return true;
}

std::string Q931::GetDisplayName() const
{
	IE_ITEM item = GetIE(Q931::DisplayIE);
	if (item.ie_len == 0) return "";

	return std::string((const char *)item.ie_data, item.ie_len);
}

static bool GetNumberIE(const Q931::IE_ITEM & item,
						std::string  & number,
                        unsigned * plan,
                        unsigned * type,
                        unsigned * presentation,
                        unsigned * screening,
                        unsigned * reason,
                        unsigned   defPresentation,
                        unsigned   defScreening,
                        unsigned   defReason)
{
	if (item.ie_len == 0) return false;

	if (plan != NULL)
		*plan = item.ie_data[0]&15;

	if (type != NULL)
		*type = (item.ie_data[0]>>4)&7;

	int offset;
	if ((item.ie_data[0] & 0x80) != 0) // Octet 3a not provided, set defaults
	{
		if (presentation != NULL)
			*presentation = defPresentation;

		if (screening != NULL)
			*screening = defScreening;

		offset = 1;
	}
	else
	{
		if (item.ie_len < 2) return false;

		if (presentation != NULL)
			*presentation = (item.ie_data[1]>>5)&3;

		if (screening != NULL)
			*screening = item.ie_data[1]&3;

		if ((item.ie_data[1] & 0x80) != 0) // Octet 3b not provided, set defaults
		{
			if (reason != NULL)
				*reason = defReason;

			offset = 2;
		}
		else
		{
			if (item.ie_len < 3) return false;

			if (reason != NULL)
				*reason = item.ie_data[2]&15;

			offset = 3;
		}
	}

	if (item.ie_len < offset) return false;

	int len = item.ie_len - offset;

	if (len > 0)
		number.assign((const char*)item.ie_data + offset, len);

	return !number.empty();
}

bool Q931::GetCallingPartyNumber(std::string  & number,
                                 unsigned * plan,
                                 unsigned * type,
                                 unsigned * presentation,
                                 unsigned * screening,
                                 unsigned   defPresentation,
                                 unsigned   defScreening) const
{
	IE_ITEM item = GetIE(CallingPartyNumberIE);
	return GetNumberIE(item, number,
                     plan, type, presentation, screening, NULL,
                     defPresentation, defScreening, 0);
}

bool Q931::GetCalledPartyNumber(std::string & number, unsigned * plan, unsigned * type) const
{
	IE_ITEM item = GetIE(CalledPartyNumberIE);
	return GetNumberIE(item,
                     number, plan, type, NULL, NULL, NULL, 0, 0, 0);
}

bool Q931::GetRedirectingNumber(std::string  & number,
                                unsigned * plan,
                                unsigned * type,
                                unsigned * presentation,
                                unsigned * screening,
                                unsigned * reason,
                                unsigned   defPresentation,
                                unsigned   defScreening,
                                unsigned   defReason) const
{
	IE_ITEM item = GetIE(RedirectingNumberIE);
	return GetNumberIE(item,
                     number, plan, type, presentation, screening, reason,
                     defPresentation, defScreening, defReason);
}


bool Q931::GetConnectedNumber(std::string  & number,
                              unsigned * plan,
                              unsigned * type,
                              unsigned * presentation,
                              unsigned * screening,
                              unsigned * reason,
                              unsigned   defPresentation,
                              unsigned   defScreening,
                              unsigned   defReason) const
{
	IE_ITEM item = GetIE(ConnectedNumberIE);
	return GetNumberIE(item, number,
                     plan, type, presentation, screening, reason,
                     defPresentation, defScreening, defReason);
}

bool Q931::GetChannelIdentification(unsigned * interfaceType,
                                    unsigned * preferredOrExclusive,
                                    int      * channelNumber) const
{
	IE_ITEM item = GetIE(ChannelIdentificationIE);
	if (item.ie_len < 1) return false;

	*interfaceType        = (item.ie_data[0]>>5) & 0x01;
	*preferredOrExclusive = (item.ie_data[0]>>3) & 0x01;

	if (*interfaceType == 0)  // basic rate
	{
		if ( (item.ie_data[0] & 0x04) == 0 ) // D Channel
		{
			*channelNumber = 0;
		}
		else
		{
			if ( (item.ie_data[0] & 0x03) == 0x03 ) // any channel
			{
				*channelNumber = -1;
			}
			else // B Channel
			{
				*channelNumber = (item.ie_data[0] & 0x03);
			}
		}
	}
	else if (*interfaceType == 1) // primary rate
	{
		if ( (item.ie_data[0] & 0x04) == 0 ) // D Channel
		{
			*channelNumber = 0;
		}
		else
		{
			if ( (item.ie_data[0] & 0x03) == 0x03 ) // any channel
			{
				*channelNumber = -1;
			}
			else // B Channel
			{
				if (item.ie_len < 3) return false;

				if (item.ie_data[1] != 0x83) return false;

				*channelNumber = item.ie_data[2] & 0x7f;
			}
		}
	}
	return true;
}

void Q931::GetFastStartInfo(struct H323_Q931_Decode_Result &result, H225_ArrayOf_PASN_OctetString &m_fastStart) const
{
	int item_count = m_fastStart.GetSize();
	for (int i = 0; i < item_count; i++)
	{
		PPER_Stream sss(m_fastStart[i].GetValue());
		H245_OpenLogicalChannel open_msg;
		if (open_msg.Decode(sss))
		{
			if (open_msg.HasOptionalField(H245_OpenLogicalChannel::e_reverseLogicalChannelParameters) &&
				open_msg.m_reverseLogicalChannelParameters.m_dataType.GetTag() == H245_DataType::e_audioData)
			{
				H245_H2250LogicalChannelParameters &ch_param = (H245_H2250LogicalChannelParameters &)open_msg.m_reverseLogicalChannelParameters.m_multiplexParameters;
				if (ch_param.HasOptionalField(H245_H2250LogicalChannelParameters::e_mediaChannel)) {
					H245_UnicastAddress_iPAddress &addr = (H245_UnicastAddress_iPAddress &)(H245_UnicastAddress &)ch_param.m_mediaChannel;
					memcpy(&result.FastStart_ReverseMediaChannelIP, (const BYTE *)addr.m_network, sizeof(u_long));
					result.FastStart_ReverseMediaChannelPort = addr.m_tsapIdentifier;
					//printf("FastStart_ReverseMediaChannel: %s:%u\n", inet_ntoa(*(PIN_ADDR)&result.FastStart_ReverseMediaChannelIP), result.FastStart_ReverseMediaChannelPort);
				}
			}
			else if (open_msg.m_forwardLogicalChannelParameters.m_dataType.GetTag() == H245_DataType::e_audioData)
			{
				
				H245_H2250LogicalChannelParameters &ch_param = (H245_H2250LogicalChannelParameters &)open_msg.m_forwardLogicalChannelParameters.m_multiplexParameters;
				if (ch_param.HasOptionalField(H245_H2250LogicalChannelParameters::e_mediaChannel)) {
					H245_UnicastAddress_iPAddress &addr = (H245_UnicastAddress_iPAddress &)(H245_UnicastAddress &)ch_param.m_mediaChannel;
					memcpy(&result.FastStart_ForwardMediaChannelIP, (const BYTE *)addr.m_network, sizeof(u_long));
					result.FastStart_ForwardMediaChannelPort = addr.m_tsapIdentifier;
					//printf("FastStart_ForwardMediaChannel: %s:%u\n", inet_ntoa(*(PIN_ADDR)&result.FastStart_ForwardMediaChannelIP), result.FastStart_ForwardMediaChannelPort);
				}
			}
		}
	}
}

bool Q931::GetConnectInfo(struct H323_Q931_Decode_Result &result) const
{
	IE_ITEM item = GetIE(UserUserIE);
	if (item.ie_len < 16) return false;

	PPER_Stream strm(item.ie_data, item.ie_len);
	H225_H323_UserInformation h323_ui;
	if (!h323_ui.Decode(strm)) return false;
	if (h323_ui.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_connect) return false;

	H225_Connect_UUIE &uuie = (H225_Connect_UUIE &)h323_ui.m_h323_uu_pdu.m_h323_message_body;
	if (uuie.HasOptionalField(H225_Connect_UUIE::e_h245Address))
	{
		H225_TransportAddress_ipAddress &addr = (H225_TransportAddress_ipAddress &)uuie.m_h245Address;
		memcpy(&result.H245_IP, (const BYTE *)addr.m_ip, sizeof(u_long));
		result.H245_Port = addr.m_port;
	}

	if (uuie.HasOptionalField(H225_Connect_UUIE::e_fastStart))
	{
		GetFastStartInfo(result, uuie.m_fastStart);
	}
	return true;
}

bool Q931::GetCallProceedingInfo(struct H323_Q931_Decode_Result &result) const
{
	IE_ITEM item = GetIE(UserUserIE);
	if (item.ie_len < 16) return false;

	PPER_Stream strm(item.ie_data, item.ie_len);
	H225_H323_UserInformation h323_ui;
	if (!h323_ui.Decode(strm)) return false;
	if (h323_ui.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_callProceeding) return false;

	H225_CallProceeding_UUIE &uuie = (H225_CallProceeding_UUIE &)h323_ui.m_h323_uu_pdu.m_h323_message_body;
	if (uuie.HasOptionalField(H225_CallProceeding_UUIE::e_h245Address))
	{
		H225_TransportAddress_ipAddress &addr = (H225_TransportAddress_ipAddress &)uuie.m_h245Address;
		memcpy(&result.H245_IP, (const BYTE *)addr.m_ip, sizeof(u_long));
		result.H245_Port = addr.m_port;
	}

	if (uuie.HasOptionalField(H225_CallProceeding_UUIE::e_fastStart))
	{
		GetFastStartInfo(result, uuie.m_fastStart);
	}
	return true;
}

bool Q931::GetAlertingInfo(struct H323_Q931_Decode_Result &result) const
{
	IE_ITEM item = GetIE(UserUserIE);
	if (item.ie_len < 16) return false;

	PPER_Stream strm(item.ie_data, item.ie_len);
	H225_H323_UserInformation h323_ui;
	if (!h323_ui.Decode(strm)) return false;
	if (h323_ui.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_alerting) return false;

	H225_Alerting_UUIE &uuie = (H225_Alerting_UUIE &)h323_ui.m_h323_uu_pdu.m_h323_message_body;
	if (uuie.HasOptionalField(H225_CallProceeding_UUIE::e_h245Address))
	{
		H225_TransportAddress_ipAddress &addr = (H225_TransportAddress_ipAddress &)uuie.m_h245Address;
		memcpy(&result.H245_IP, (const BYTE *)addr.m_ip, sizeof(u_long));
		result.H245_Port = addr.m_port;
	}

	if (uuie.HasOptionalField(H225_CallProceeding_UUIE::e_fastStart))
	{
		GetFastStartInfo(result, uuie.m_fastStart);
	}
	return true;
}

bool Q931::GetFacilityInfo(struct H323_Q931_Decode_Result &result) const
{
	IE_ITEM item = GetIE(UserUserIE);
	if (item.ie_len < 16) return false;

	PPER_Stream strm(item.ie_data, item.ie_len);
	H225_H323_UserInformation h323_ui;
	if (!h323_ui.Decode(strm)) return false;
	if (h323_ui.m_h323_uu_pdu.m_h323_message_body.GetTag() != H225_H323_UU_PDU_h323_message_body::e_facility) return false;

	H225_Facility_UUIE &uuie = (H225_Facility_UUIE &)h323_ui.m_h323_uu_pdu.m_h323_message_body;
	if (uuie.HasOptionalField(H225_Facility_UUIE::e_h245Address))
	{
		H225_TransportAddress_ipAddress &addr = (H225_TransportAddress_ipAddress &)uuie.m_h245Address;
		memcpy(&result.H245_IP, (const BYTE *)addr.m_ip, sizeof(u_long));
		result.H245_Port = addr.m_port;
	}

	if (uuie.HasOptionalField(H225_Facility_UUIE::e_fastStart))
	{
		GetFastStartInfo(result, uuie.m_fastStart);
	}
	return true;
}
