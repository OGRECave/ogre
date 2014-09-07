#ifndef __OGREMONITORINFO_H__
#define __OGREMONITORINFO_H__
#include "OgreD3D11Prerequisites.h"
namespace Ogre 
{
	class MonitorInfo
	{
	public :
		MonitorInfo() : mCurrentMonitor(0) {}
		void Refresh();
		unsigned short getMonitorSequentialNumberFromHMonitor(HMONITOR hMonitor,bool allowRefresh = false);
	private:
		typedef std::map<HMONITOR, unsigned short> MapMonitorToSequentialNumber;
		typedef std::pair<HMONITOR, unsigned short> PairMonitorToSequentialNumber;
		MapMonitorToSequentialNumber mMapMonitors;
		unsigned short mCurrentMonitor;

		static BOOL CALLBACK MonitorEnumProc(
			_In_  HMONITOR hMonitor,
			_In_  HDC hdcMonitor,
			_In_  LPRECT lprcMonitor,
			_In_  LPARAM dwData
			);
	};
}
#endif