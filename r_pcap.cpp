#include "r_pcap.h"

#ifdef WIN32
#pragma comment(lib, "Ws2_32")
#define PLUGIN_API extern "C" __declspec(dllexport)
#else
#define PLUGIN_API extern "C"
#endif

PcapMgr pcap_mgr;
PLUGIN_API bool Rec_Plugin_Initialize(IRecorder *pRec, Utils::IniFile* ini)
{
	return pcap_mgr.Initialize(pRec, ini);
}

PLUGIN_API void Rec_Plugin_Uninitialize()
{
	pcap_mgr.Uninitialize();
}

//-----------------------------------------------------------------
PcapMgr::PcapMgr(void)
{
}

PcapMgr::~PcapMgr(void)
{
}

bool PcapMgr::Initialize(IRecorder *rec, Utils::IniFile *ini)
{
	if (rec == NULL) return false;
	pRec = rec;
	pRec->logi("--- r_pcap, Build %s %s ---", __DATE__, __TIME__);

	init_pkg_mallocs();	/*sip parser init*/

	std::string capfile = ini->ReadString("capFile");
	if (!capfile.empty())
	{
		rec->SetVirtualMode();
		PcapDev *new_dev = new PcapDev(capfile.c_str(), 0xffffff);
		if (!new_dev->OpenFile())
		{
			delete new_dev;
			return false;
		}
		PcapDevs.push_back(new_dev);
		return true;
	}

	pRec->log(INF, "Detecting network adapters...");
	pcap_if_t *alldevs = NULL;
	char errbuf[PCAP_ERRBUF_SIZE] = {0};
	/* Retrieve the local device list */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		pRec->log(CRI, "[pcap_findalldevs] %s", errbuf);
		return false;
	}

	for (pcap_if_t *dev = alldevs; dev != NULL; dev = dev->next)
	{
		pRec->log(INF, "%s", dev->name);
		pRec->log(INF, "\t%s", dev->description == NULL ? "N/A" : dev->description);
		if (dev->addresses == NULL || 0 == strcmp(dev->name, "lo")) continue;
	
		/* 获得接口第一个地址的掩码, 如果接口没有地址，那么我们假设一个C类的掩码0xffffff */
		PcapDev *new_dev = new PcapDev(dev->name, 
			dev->addresses->netmask == NULL ? 0xffffff :
			((struct sockaddr_in *)(dev->addresses->netmask))->sin_addr.s_addr);
		for (pcap_addr *addr_p = dev->addresses; addr_p != NULL; addr_p = addr_p->next)
		{
			std::string sIP = inet_ntoa(((sockaddr_in*)addr_p->addr)->sin_addr);
			new_dev->IPs.push_back(sIP);
			pRec->log(INF, "\t%s", sIP.c_str());
		}
		if (new_dev->Open()) PcapDevs.push_back(new_dev);
		else delete new_dev;
	}

	pcap_freealldevs(alldevs);
	pRec->log(INF, "%u pcapdev(s) opened.", PcapDevs.size());

	return true;
}

void PcapMgr::Uninitialize()
{
	for (const auto& dev : PcapDevs)
	{
		dev->Close();
		delete dev;
	}
	PcapDevs.clear();
	destory_pkg_mallocs();
}
