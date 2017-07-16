// GetParentProcID.cpp : Defines the entry point for the console application.
//

#include "OgrePrerequisites.h"
#include "OgreNsightChecker.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#pragma comment(lib,"Psapi.lib")


static BOOL WINAPI GetParentPID(PROCESSENTRY32& procentry)
{
	OSVERSIONINFO  osver;
	HINSTANCE      hInstLib;
	HANDLE         hSnapShot;
	BOOL           bContinue;

	// ToolHelp Function Pointers.
	HANDLE(WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
	BOOL(WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
	BOOL(WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);

	// Check to see if were running under Windows95 or
	// Windows NT.
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver))
	{
		return FALSE;
	}

	if (osver.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		return FALSE;
	}

	hInstLib = LoadLibraryA("Kernel32.DLL");
	if (hInstLib == NULL)
	{
		return FALSE;
	}

	// Get procedure addresses.
	// We are linking to these functions of Kernel32
	// explicitly, because otherwise a module using
	// this code would fail to load under Windows NT,
	// which does not have the Toolhelp32
	// functions in the Kernel 32.
	lpfCreateToolhelp32Snapshot =
		(HANDLE(WINAPI *)(DWORD, DWORD))
		GetProcAddress(hInstLib,
		"CreateToolhelp32Snapshot");
	lpfProcess32First =
		(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32))
		GetProcAddress(hInstLib, "Process32First");
	lpfProcess32Next =
		(BOOL(WINAPI *)(HANDLE, LPPROCESSENTRY32))
		GetProcAddress(hInstLib, "Process32Next");
	if (lpfProcess32Next == NULL ||
		lpfProcess32First == NULL ||
		lpfCreateToolhelp32Snapshot == NULL)
	{
		FreeLibrary(hInstLib);
		return FALSE;
	}

	// Get a handle to a Toolhelp snapshot of the systems
	// processes.
	hSnapShot = lpfCreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS, 0);
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		FreeLibrary(hInstLib);
		return FALSE;
	}

	// Get the first process' information.
	memset((LPVOID)&procentry, 0, sizeof(PROCESSENTRY32));
	procentry.dwSize = sizeof(PROCESSENTRY32);
	bContinue = lpfProcess32First(hSnapShot, &procentry);
	DWORD pid = 0;
	// While there are processes, keep looping.
	DWORD  crtpid = GetCurrentProcessId();
	while (bContinue)
	{
		if (crtpid == procentry.th32ProcessID)
			pid = procentry.th32ParentProcessID;

		procentry.dwSize = sizeof(PROCESSENTRY32);
		bContinue = !pid && lpfProcess32Next(hSnapShot, &procentry);

	}//while ends


	// Free the library.
	FreeLibrary(hInstLib);

	return pid ? TRUE : FALSE;
}

#ifdef _DEBUG
#define PARENT "msdev.exe"
const DWORD TIMEOUT = 5000;
#else
#define PARENT "idriver.exe"
const DWORD TIMEOUT = 30000;
#endif

static std::string GetProcessFileName(DWORD processID)
{
		std::string result = "";

		HANDLE hProcess = OpenProcess(
			SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, processID);
		if (hProcess != NULL)
		{
			// Here we call EnumProcessModules to get only the
			// first module in the process this is important,
			// because this will be the .EXE module for which we
			// will retrieve the full path name in a second.
			HMODULE        hMod;
			char           szFileName[MAX_PATH];
			DWORD dwSize2 = 0;
			LPTSTR pszName = NULL;
			if (EnumProcessModules(hProcess, &hMod,
				sizeof(hMod), &dwSize2))
			{
				// Get Full pathname:

				if (GetModuleFileNameEx(hProcess, hMod,
					szFileName, sizeof(szFileName)))
					result = std::string(szFileName);
			}
		}

		return result;
}

static bool IsWorkingUnderNsightImpl()
{
	PROCESSENTRY32 selfprocentry;
	if(GetParentPID(selfprocentry))
	{
		std::string parentFileName = GetProcessFileName(selfprocentry.th32ParentProcessID);
		return parentFileName.find("Nsight.Monitor") != std::string::npos;
	}
	return false;
}

#endif

namespace Ogre
{
	bool IsWorkingUnderNsight()
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		static bool isWorkingUnderNsight = IsWorkingUnderNsightImpl();
		return isWorkingUnderNsight;
#else
		return false;
#endif
	}
}
