#ifndef SKINNY_H
#define SKINNY_H
/*
 Code       Station Message ID Message
 0x0000   Keep Alive Message
 0x0001   Station Register Message
 0x0002   Station IP Port Message
 0x0003   Station Key Pad Button Message
 0x0004   Station Embloc Call Message
 0x0005   Station Stimulus Message
 0x0006   Station Off Hook Message
 0x0007   Station On Hook Message
 0x0008   Station Hook Flash Message
 0x0009   Station Forward Status Request Message
 0x11     Station Media Port List Message
 0x000A   Station Speed Dial Status Request Message
 0x000B   Station Line Status Request Message
 0x000C   Station Configuration Status Request Message
 0x000D   Station Time Date Request Message
 0x000E   Station Button Template Request Message
 0x000F   Station Version Request Message
 0x0010   Station Capabilities Response Message
 0x0012   Station Server Request Message
 0x0020   Station Alarm Message
 0x0021   Station Multicast Media Reception Ack Message
 0x0024   Station Off Hook With Calling Party Number Message
 0x22     Station Open Receive Channel Ack Message
 0x23     Station Connection Statistics Response Message
 0x25     Station Soft Key Template Request Message
 0x26     Station Soft Key Set Request Message
 0x27     Station Soft Key Event Message
 0x28     Station Unregister Message
 0x0081   Station Keep Alive Message
 0x0082   Station Start Tone Message
 0x0083   Station Stop Tone Message
 0x0085   Station Set Ringer Message
 0x0086   Station Set Lamp Message
 0x0087   Station Set Hook Flash Detect Message
 0x0088   Station Set Speaker Mode Message
 0x0089   Station Set Microphone Mode Message
 0x008A   Station Start Media Transmission
 0x008B   Station Stop Media Transmission
 0x008F   Station Call Information Message
 0x009D   Station Register Reject Message
 0x009F   Station Reset Message
 0x0090   Station Forward Status Message
 0x0091   Station Speed Dial Status Message
 0x0092   Station Line Status Message
 0x0093   Station Configuration Status Message
 0x0094   Station Define Time & Date Message
 0x0095   Station Start Session Transmission Message
 0x0096   Station Stop Session Transmission Message
 0x0097   Station Button Template Message
 0x0098   Station Version Message
 0x0099   Station Display Text Message
 0x009A   Station Clear Display Message
 0x009B   Station Capabilities Request Message
 0x009C   Station Enunciator Command Message
 0x009E   Station Server Respond Message
 0x0101   Station Start Multicast Media Reception Message
 0x0102   Station Start Multicast Media Transmission Message
 0x0103   Station Stop Multicast Media Reception Message
 0x0104   Station Stop Multicast Media Transmission Message
 0x105    Station Open Receive Channel Message
 0x0106   Station Close Receive Channel Message
 0x107    Station Connection Statistics Request Message
 0x0108   Station Soft Key Template Respond Message
 0x109    Station Soft Key Set Respond Message
 0x0110   Station Select Soft Keys Message
 0x0111   Station Call State Message
		  
		  1—Off Hook
		  2—On Hook
		  3—Ring Out
		  4—Ring In
		  5—Connected
		  6—Busy
		  7—Line In Use
		  8—Hold
		  9—Call Waiting
		  10—Call Transfer
		  11—Call Park
		  12—Call Proceed
		  13—In Use Remotely
		  14—Invalid Number

 0x0112   Station Display Prompt Message
 0x0113   Station Clear Prompt Message
 0x0114   Station Display Notify Message
 0x0115   Station Clear Notify Message
 0x0116   Station Activate Call Plane Message
 0x0117   Station Deactivate Call Plane Message
 0x118    Station Unregister Ack Message
*/

enum SkinnyCallState
{
	CS_Off_Hook = 1, 
	CS_On_Hook = 2,
	CS_Ring_Out = 3,
	CS_Ring_In = 4,
	CS_Connected = 5,
	CS_Busy = 6,
	CS_Line_In_Use = 7,
	CS_Hold = 8,
	CS_Call_Waiting = 9,
	CS_Call_Transfer = 10,
	CS_Call_Park = 11,
	CS_Call_Proceed = 12,
	CS_In_Use_Remotely = 13,
	CS_Invalid_Number = 14
};

#pragma pack(1)    //进入字节对齐方式

struct skinny_header
{
	u_int datalen;			//从messageid开始的数据长度
	u_int version;
	u_int messageid;
};

#define SKINNY_0x3_KEYPAD_BUTTON 0x3
struct skinny_0x3_keypad_button
{
	u_int KeypadButton;
	u_int Flag;
	u_int CallId;
};

#define SKINNY_0x8f_CALL_INFO 0x8f
struct skinny_0x8f_call_info
{
	char  CallingPartyName[40];
	char  CallingParty[24];
	char  CalledPartyName[40];
	char  CalledParty[24];
	u_int LineInstance;
	u_int CallId;
	u_int CallType;				//OutBoundCall(2)
	char  OrignalCalledPartyName[40];
	char  OrignalCalledParty[24];
	char  LastRedirectingPartyName[40];
	char  LastRedirectingParty[24];
	u_int OriginalCdpnRedirectReason;
	u_int LastRedirectingReason;
	char  CgpnVoiceMailbox[24];
	char  CdpnVoiceMailbox[24];
	char  OriginalCdpnVoiceMailbox[24];
	char  LastRedirectingVoiceMailbox[24];
	u_int CallInstance;
	u_int CallSecurityStatus;	//CallSecurityStatusNotAuthenticated (1)
	u_int partyPIRestrictionBits;
};

#define SKINNY_0x14a_CALL_INFO 0x14a
struct skinny_0x14a_call_info
{
	u_int LineInstance;
	u_int CallId;
	char  X1[24];
	char  Calling_Called[32];
};

#define SKINNY_0x105_OPEN_RECEIVE_CHANNEL 0x105
struct skinny_0x105_open_receive_channel
{
	u_int ConferenceId;
	u_int PassThruPartyId;
	u_int PacketMS;				//20 ms
	u_int PayloadCapability;	//G.711 u-law 64k (4)
	u_int EchoCancelType;		//Media_EchoCancellation_Off (0)
	u_int G723BitRate;			//Unknown (0)
	u_int CallId;
};

#define SKINNY_0x106_CLOSE_RECEIVE_CHANNEL 0x106
struct skinny_0x106_close_receive_channel
{
	u_int ConferenceId;
	u_int PassThruPartyId;
	u_int CallId;
};

#define SKINNY_0x8a_START_MEDIA_TRANSMISSION 0x8a
struct skinny_0x8a_start_media_transmission
{
	u_int ConferenceId;
	u_int PassThruPartyId;
	ip_address RemoteIpAddress;
	u_int RemotePort;
	u_int PacketMS;				//20 ms
	u_int PayloadCapability;	//G.711 u-law 64k (4)
	u_int Precedence;
	u_int SilenceSuppression;	//Media_SilenceSuppression_Off (0x00000000)
	u_int MaxFramesPerPacket;
	u_int G723BitRate;			//Unknown (0)
	u_int CallId;
};

struct CCM7_skinny_0x8a_start_media_transmission
{
	u_int ConferenceId;
	u_int PassThruPartyId;
	u_int I1;
	ip_address RemoteIpAddress;
	char  X1[12]; 
	u_int RemotePort;
	u_int PacketMS;				//20 ms
	u_int PayloadCapability;	//G.711 u-law 64k (4)
	u_int Precedence;
	u_int SilenceSuppression;	//Media_SilenceSuppression_Off (0x00000000)
	u_int MaxFramesPerPacket;
	u_int G723BitRate;			//Unknown (0)
	u_int CallId;
};

#define SKINNY_0x22_OPEN_RECEIVE_CHANNEL_ACK 0x22
struct skinny_0x22_open_receive_channel_ack
{
	u_int OpenReceiveChannelStatus;	// orcOk (0)
	ip_address IpAddress;
	u_int Port;
	u_int PassThruPartyId;
	u_int CallId; //some CCM have no callid, only PassThruPartyID
};

struct CCM7_skinny_0x22_open_receive_channel_ack
{
	u_int OpenReceiveChannelStatus;	// orcOk (0)
	u_int I1;
	ip_address IpAddress;
	char  X1[12]; 
	u_int Port;
	u_int PassThruPartyId;
	u_int CallId;
};

#define SKINNY_0x111_CALL_STATE	0x111
struct skinny_0x111_call_state
{
	u_int CallState;
	u_int LineInstance;
	u_int CallId;
};

#pragma pack()    //恢复默认对齐方式

#endif
