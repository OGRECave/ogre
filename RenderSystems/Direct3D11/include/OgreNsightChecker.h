#ifndef __NSIGHTCHECKER_H__
#define __NSIGHTCHECKER_H__

#include <string.h>
#include <tlhelp32.h>
namespace Ogre
{ 
	class NsightChecker
	{
	private:
		static BOOL WINAPI GetParentPID(PROCESSENTRY32& procentry);
		static std::string GetProcessFileName(DWORD processID);
	public:
		static bool IsWorkingUnderNsight();
	};
}

#endif
