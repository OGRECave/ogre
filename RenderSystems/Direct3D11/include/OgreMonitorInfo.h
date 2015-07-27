#ifndef __OGREMONITORINFO_H__
#define __OGREMONITORINFO_H__
#include "OgreD3D11Prerequisites.h"
#include "OgreSingleton.h"
namespace Ogre 
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    class MonitorInfo : public Singleton<MonitorInfo>
	{
	public :
        MonitorInfo();
		void Refresh();

		unsigned short getMonitorSequentialNumberFromHMonitor(HMONITOR hMonitor, bool allowRefresh = false);
		unsigned short getMonitorSequentialNumberFromSwapChain(IDXGISwapChain* swapChain, bool allowRefresh = false);
		const MONITORINFO * const getMonitorInfo(unsigned short monitorIndex, bool allowRefresh = false);
		const unsigned short getMonitorsCount() const;
	private:
		typedef std::map<HMONITOR, unsigned short> MapMonitorToSequentialNumber;
        typedef std::vector<MONITORINFO> ListMonitorInfo;

		MapMonitorToSequentialNumber mMapMonitors;
        ListMonitorInfo mListMonitorInfo;
        unsigned short mMonitorsCount;

		static BOOL CALLBACK MonitorEnumProc(
			_In_  HMONITOR hMonitor,
			_In_  HDC hdcMonitor,
			_In_  LPRECT lprcMonitor,
			_In_  LPARAM dwData
			);
	};
#endif
}
#endif
