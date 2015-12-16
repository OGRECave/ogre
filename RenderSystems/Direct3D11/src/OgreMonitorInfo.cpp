#include "OgreMonitorInfo.h"
namespace Ogre
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    template<> MonitorInfo* Singleton<MonitorInfo>::msSingleton = 0;
    //---------------------------------------------------------------------
    MonitorInfo::MonitorInfo()
    {
        Refresh();
    }
    //---------------------------------------------------------------------
    void MonitorInfo::Refresh()
    {
        mMapMonitors.clear();
        mListMonitorInfo.clear();
        mMonitorsCount = 0;
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc,(LPARAM)this);
    }
    //---------------------------------------------------------------------
	const MONITORINFO * const MonitorInfo::getMonitorInfo(unsigned short monitorIndex, bool allowRefresh /*= false*/)
    {
        LPMONITORINFO result = NULL;
        if (monitorIndex >= mListMonitorInfo.size() && allowRefresh)
            Refresh();

        if (monitorIndex < mListMonitorInfo.size())
            result = &mListMonitorInfo[monitorIndex];

        return result;
    }
    //---------------------------------------------------------------------
    unsigned short MonitorInfo::getMonitorSequentialNumberFromHMonitor( HMONITOR hMonitor,bool allowRefresh /*= false*/ )
	{
		MapMonitorToSequentialNumber::const_iterator it = mMapMonitors.find(hMonitor);
		bool found = it != mMapMonitors.end();
		if (!found && allowRefresh)
		{
			Refresh();
			it = mMapMonitors.find(hMonitor);
			found = it != mMapMonitors.end();
		}

		if (found)
			return it->second;
		else
			return -1;
	}
    //---------------------------------------------------------------------
    unsigned short MonitorInfo::getMonitorSequentialNumberFromSwapChain(IDXGISwapChain* swapChain, bool allowRefresh /*= false*/)
	{
		IDXGIOutput* output;
		int monitorSequencialNumber = -1;
		if(swapChain != NULL)
		{
			HRESULT hr = swapChain->GetContainingOutput(&output);
			if(hr == S_OK)
			{
				DXGI_OUTPUT_DESC desc;
				output->GetDesc(&desc);
				monitorSequencialNumber = getMonitorSequentialNumberFromHMonitor(desc.Monitor, allowRefresh);
			}
		}
		return monitorSequencialNumber;
    }
    //---------------------------------------------------------------------
    BOOL CALLBACK MonitorInfo::MonitorEnumProc( _In_ HMONITOR hMonitor, _In_ HDC hdcMonitor, _In_ LPRECT lprcMonitor, _In_ LPARAM dwData )
    {
        MonitorInfo* _this = (MonitorInfo*)dwData;
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(monitorInfo);
        GetMonitorInfo(hMonitor, &monitorInfo);
        _this->mListMonitorInfo.push_back(monitorInfo);
        _this->mMapMonitors.insert(std::make_pair(hMonitor, _this->mMonitorsCount++));
        return true;
    }
    //---------------------------------------------------------------------
	const unsigned short MonitorInfo::getMonitorsCount() const
    {
        return mMonitorsCount;
    }

#endif
}
