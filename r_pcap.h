#ifndef PCAP_MANAGER_H
#define PCAP_MANAGER_H

#ifdef WIN32
#include <winsock2.h>
#endif

#include <Utils/IniFile.h>
#include <IRecorder.h>
#include <pcap_dev.h>

class PcapMgr
{
public:
	PcapMgr(void);
	~PcapMgr(void);

	bool Initialize(IRecorder *pRec, Utils::IniFile *ini);
	void Uninitialize();
	std::list<PcapDev*> PcapDevs;
	IRecorder *pRec;
};

extern PcapMgr pcap_mgr;

#endif
