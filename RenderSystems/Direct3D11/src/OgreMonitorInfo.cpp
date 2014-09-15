#include "OgreMonitorInfo.h"
namespace Ogre
{
	void Ogre::MonitorInfo::Refresh()
	{
		mMapMonitors.clear();
		mCurrentMonitor = 0;
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc,(LPARAM)this);
	}

	unsigned short Ogre::MonitorInfo::getMonitorSequentialNumberFromHMonitor( HMONITOR hMonitor,bool allowRefresh /*= false*/ )
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

	BOOL CALLBACK MonitorInfo::MonitorEnumProc( _In_ HMONITOR hMonitor, _In_ HDC hdcMonitor, _In_ LPRECT lprcMonitor, _In_ LPARAM dwData )
	{
		MonitorInfo* _this = (MonitorInfo*)dwData;
		_this->mMapMonitors.insert(PairMonitorToSequentialNumber(hMonitor, _this->mCurrentMonitor++));
		return true;
	}

}
