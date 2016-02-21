#include "OgreMonitorInfo.h"
namespace Ogre
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

	void Ogre::MonitorInfo::Refresh()
	{
		mMapMonitors.clear();
		mCurrentMonitor = 0;
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc,(LPARAM)this);
	}

	unsigned short Ogre::MonitorInfo::getMonitorSequentialNumberFromHMonitor( HMONITOR hMonitor, bool allowRefresh /*= false*/ )
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

	unsigned short Ogre::MonitorInfo::getMonitorSequentialNumberFromSwapChain(IDXGISwapChain* swapChain, bool allowRefresh /*= false*/)
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

	BOOL CALLBACK MonitorInfo::MonitorEnumProc(_In_ HMONITOR hMonitor, _In_ HDC hdcMonitor, _In_ LPRECT lprcMonitor, _In_ LPARAM dwData)
	{
		MonitorInfo* _this = (MonitorInfo*)dwData;
		_this->mMapMonitors.insert(std::make_pair(hMonitor, _this->mCurrentMonitor++));
		return true;
	}

#endif
}
