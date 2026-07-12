/*
 * q931.h
 *
 * Q.931 protocol handler
 *
 * Changed by maodonghu base-on Open H323 Library for parse only
 */
 
#ifndef Q931_H
#define Q931_H

#include <string>
#include <map>
#include <ptlib.h>
#include "asn/h225.h"

#ifdef WIN32
#include <winsock2.h>
#endif

/**This class embodies a Q.931 Protocol Data Unit.
  */
class Q931
{
public:
	enum MsgTypes 
	{
		NationalEscapeMsg  = 0x00,
		AlertingMsg        = 0x01,
		CallProceedingMsg  = 0x02,
		ConnectMsg         = 0x07,
		ConnectAckMsg      = 0x0f,
		ProgressMsg        = 0x03,
		SetupMsg           = 0x05,
		SetupAckMsg        = 0x0d,
		ResumeMsg          = 0x26,
		ResumeAckMsg       = 0x2e,
		ResumeRejectMsg    = 0x22,
		SuspendMsg         = 0x25,
		SuspendAckMsg      = 0x2d,
		SuspendRejectMsg   = 0x21,
		UserInformationMsg = 0x20,
		DisconnectMsg      = 0x45,
		ReleaseMsg         = 0x4d,
		ReleaseCompleteMsg = 0x5a,
		RestartMsg         = 0x46,
		RestartAckMsg      = 0x4e,
		SegmentMsg         = 0x60,
		CongestionCtrlMsg  = 0x79,
		InformationMsg     = 0x7b,
		NotifyMsg          = 0x6e,
		StatusMsg          = 0x7d,
		StatusEnquiryMsg   = 0x75,
		FacilityMsg        = 0x62
    };

    Q931();
 
    bool Decode(u_char *data, u_int len);
    const char* GetMessageTypeName() const;
	//static const char* GetIEName(Q931::IETypes ieType);
	//static const char* GetCauseName(Q931::CauseValues cause);

    unsigned GetCallReference() const { return callReference; }
    bool IsFromDestination() const { return fromDestination; }
    MsgTypes GetMessageType() const { return messageType; }

	enum IETypes 
	{
		BearerCapabilityIE      = 0x04,
		CauseIE                 = 0x08,
		ChannelIdentificationIE = 0x18,
		FacilityIE              = 0x1c,
		ProgressIndicatorIE     = 0x1e,
		CallStateIE             = 0x14,
		DisplayIE               = 0x28,
		KeypadIE                = 0x2c,
		SignalIE                = 0x34,
		ConnectedNumberIE       = 0x4c,
		CallingPartyNumberIE    = 0x6c,
		CalledPartyNumberIE     = 0x70,
		RedirectingNumberIE     = 0x74,
		UserUserIE              = 0x7e
    };

	struct IE_ITEM
	{
		IE_ITEM() : ie_len(0), ie_data(NULL) {}
		int		ie_len;
		u_char*	ie_data;
	};
    IE_ITEM GetIE(IETypes ie) const;

	enum InformationTransferCapability 
	{
		TransferSpeech,
		TransferUnrestrictedDigital = 8,
		TransferRestrictedDigital = 9,
		Transfer3_1kHzAudio = 16,
		TrasnferUnrestrictedDigitalWithTones = 17,
		TransferVideo = 24
    };

    bool GetBearerCapabilities(
		InformationTransferCapability & capability,
		unsigned & transferRate,        ///<  Number of 64k B channels
		unsigned * codingStandard = NULL,
		unsigned * userInfoLayer1 = NULL
    );

	enum CauseValues 
	{
		UnknownCauseIE               =  0,
		UnallocatedNumber            =  1,
		NoRouteToNetwork             =  2,
		NoRouteToDestination         =  3,
		SendSpecialTone              =  4,
		MisdialledTrunkPrefix        =  5,
		ChannelUnacceptable          =  6,
		NormalCallClearing           = 16,
		UserBusy                     = 17,
		NoResponse                   = 18,
		NoAnswer                     = 19,
		SubscriberAbsent             = 20,
		CallRejected                 = 21,
		NumberChanged                = 22,
		Redirection                  = 23,
		ExchangeRoutingError         = 25,
		NonSelectedUserClearing      = 26,
		DestinationOutOfOrder        = 27,
		InvalidNumberFormat          = 28,
		FacilityRejected             = 29,
		StatusEnquiryResponse        = 30,
		NormalUnspecified            = 31,
		NoCircuitChannelAvailable    = 34,
		NetworkOutOfOrder            = 38,
		TemporaryFailure             = 41,
		Congestion                   = 42,
		RequestedCircuitNotAvailable = 44,
		ResourceUnavailable          = 47,
		ServiceOptionNotAvailable    = 63,
		InvalidCallReference         = 81,
		ClearedRequestedCallIdentity = 86,
		IncompatibleDestination      = 88,
		IENonExistantOrNotImplemented= 99,
		TimerExpiry                  = 102,
		ProtocolErrorUnspecified     = 111,
		InterworkingUnspecified      = 127,
		ErrorInCauseIE               = 0x100
    };

    CauseValues GetCause(
      unsigned * standard = NULL,  ///<  0 = ITU-T standardized coding
      unsigned * location = NULL   ///<  0 = User
    ) const;

	enum CallStates 
	{
		CallState_Null                  = 0,
		CallState_CallInitiated         = 1,
		CallState_OverlapSending        = 2,
		CallState_OutgoingCallProceeding= 3,
		CallState_CallDelivered         = 4,
		CallState_CallPresent           = 6,
		CallState_CallReceived          = 7,
		CallState_ConnectRequest        = 8,
		CallState_IncomingCallProceeding= 9,
		CallState_Active                = 10,
		CallState_DisconnectRequest     = 11,
		CallState_DisconnectIndication  = 12,
		CallState_SuspendRequest        = 15,
		CallState_ResumeRequest         = 17,
		CallState_ReleaseRequest        = 19,
		CallState_OverlapReceiving      = 25,
		CallState_ErrorInIE             = 0x100
    };

    CallStates GetCallState(
      unsigned * standard = NULL  ///<  0 = ITU-T standardized coding
    ) const;

	enum SignalInfo 
	{
		SignalDialToneOn,
		SignalRingBackToneOn,
		SignalInterceptToneOn,
		SignalNetworkCongestionToneOn,
		SignalBusyToneOn,
		SignalConfirmToneOn,
		SignalAnswerToneOn,
		SignalCallWaitingTone,
		SignalOffhookWarningTone,
		SignalPreemptionToneOn,
		SignalTonesOff = 0x3f,
		SignalAlertingPattern0 = 0x40,
		SignalAlertingPattern1,
		SignalAlertingPattern2,
		SignalAlertingPattern3,
		SignalAlertingPattern4,
		SignalAlertingPattern5,
		SignalAlertingPattern6,
		SignalAlertingPattern7,
		SignalAlertingOff = 0x4f,
		SignalErrorInIE = 0x100
    };
    SignalInfo GetSignalInfo() const;

	std::string GetKeypad() const;

	enum ProgressIndication 
	{
		ProgressNotEndToEndISDN      = 1,		// Call is not end-to-end ISDN; 
												// further call progress information may be available in-band  
		ProgressDestinationNonISDN   = 2,		// Destination address is non ISDN  
		ProgressOriginNotISDN        = 3,		// Origination address is non ISDN  
		ProgressReturnedToISDN       = 4,		// Call has returned to the ISDN 
		ProgressServiceChange        = 5,		// Interworking has occurred and has 
												// resulted in a telecommunication service change
		ProgressInbandInformationAvailable = 8	// In-band information or an appropriate pattern is now available.   
    };

	bool GetProgressIndicator(
		unsigned & description,
		unsigned * codingStandard = NULL,
		unsigned * location = NULL
    ) const;

	std::string GetDisplayName() const;

	enum NumberingPlanCodes 
	{
		UnknownPlan          = 0x00,
		ISDNPlan             = 0x01,
		DataPlan             = 0x03,
		TelexPlan            = 0x04,
		NationalStandardPlan = 0x08,
		PrivatePlan          = 0x09,
		ReservedPlan         = 0x0f
    };

	enum TypeOfNumberCodes 
	{
		UnknownType          = 0x00,
		InternationalType    = 0x01,
		NationalType         = 0x02,
		NetworkSpecificType  = 0x03,
		SubscriberType       = 0x04,
		AbbreviatedType      = 0x06,
		ReservedType         = 0x07
    };

    bool GetCallingPartyNumber(
		std::string & number,           ///<  Number string
		unsigned * plan = NULL,         ///<  ISDN/Telephony numbering system
		unsigned * type = NULL,         ///<  Number type
		unsigned * presentation = NULL, ///<  Presentation indicator
		unsigned * screening = NULL,    ///<  Screening indicator
		unsigned defPresentation = 0,   ///<  Default value if octet3a not present
		unsigned defScreening = 0       ///<  Default value if octet3a not present
    ) const;

    bool GetCalledPartyNumber(
		std::string & number,       ///<  Number string
		unsigned * plan = NULL,		///<  ISDN/Telephony numbering system
		unsigned * type = NULL		///<  Number type
    ) const;

    bool GetRedirectingNumber(
		std::string & number,           ///<  Number string
		unsigned * plan = NULL,         ///<  ISDN/Telephony numbering system
		unsigned * type = NULL,         ///<  Number type
		unsigned * presentation = NULL, ///<  Presentation indicator
		unsigned * screening = NULL,    ///<  Screening indicator
		unsigned * reason = NULL,       ///<  Reason for redirection
		unsigned defPresentation = 0,   ///<  Default value if octet3a not present
		unsigned defScreening = 0,      ///<  Default value if octet3a not present
		unsigned defReason =0           ///<  Default value if octet 3b not present
    ) const;

    bool GetConnectedNumber(
		std::string & number,			///<  Number string
		unsigned * plan = NULL,			///<  ISDN/Telephony numbering system
		unsigned * type = NULL,         ///<  Number type
		unsigned * presentation = NULL, ///<  Presentation indicator
		unsigned * screening = NULL,    ///<  Screening indicator
		unsigned * reason = NULL,       ///<  Reason for redirection
		unsigned defPresentation = 0,   ///<  Default value if octet3a not present
		unsigned defScreening = 0,      ///<  Default value if octet3a not present
		unsigned defReason =0           ///<  Default value if octet 3b not present
    ) const;

    /**Get the limitations to ChannelIdentification.
      */
	bool GetChannelIdentification(
		unsigned * interfaceType = NULL,        ///<  Interface type
		unsigned * preferredOrExclusive = NULL, ///<  Channel negotiation preference
		int      * channelNumber = NULL         ///<  Channel number
    ) const;

	void GetFastStartInfo(struct H323_Q931_Decode_Result &result, H225_ArrayOf_PASN_OctetString &m_fastStart) const;
	bool GetConnectInfo(struct H323_Q931_Decode_Result &result) const;
	bool GetCallProceedingInfo(struct H323_Q931_Decode_Result &result) const;
	bool GetAlertingInfo(struct H323_Q931_Decode_Result &result) const;
	bool GetFacilityInfo(struct H323_Q931_Decode_Result &result) const;

protected:
	unsigned callReference;
	bool fromDestination;
	unsigned protocolDiscriminator;
	MsgTypes messageType;

	std::map<IETypes, IE_ITEM> InformationElements;
};

#endif // __OPAL_Q931_H
