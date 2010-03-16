# Microsoft Developer Studio Project File - Name="Lwo2Mesh" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Lwo2Mesh - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LWO2MESH.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LWO2MESH.mak" CFG="Lwo2Mesh - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Lwo2Mesh - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Lwo2Mesh - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Lwo2Mesh - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\bin\Release"
# PROP BASE Intermediate_Dir "..\obj\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\Release"
# PROP Intermediate_Dir "..\obj\Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "TIXML_USE_STL" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /machine:I386
# ADD LINK32 ogremain.lib /nologo /subsystem:console /profile /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libcmtd.lib" /libpath:"..\..\..\OgreMain\lib\Release"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Lwo2Mesh - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin\Debug"
# PROP BASE Intermediate_Dir "..\obj\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\Debug"
# PROP Intermediate_Dir "..\obj\Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W1 /Gm /GX /Zi /Od /I "..\include" /I "..\..\..\OgreMain\include" /I "..\..\..\Dependencies\include" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "TIXML_USE_STL" /FD /GZ /Zm500 /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ogremain_d.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmt.lib" /libpath:"..\..\..\OgreMain\lib\Debug"
# SUBTRACT LINK32 /verbose /profile

!ENDIF 

# Begin Target

# Name "Lwo2Mesh - Win32 Release"
# Name "Lwo2Mesh - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\BitArray.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lwEnvelope.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lwLayer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lwo2mesh.cpp

!IF  "$(CFG)" == "Lwo2Mesh - Win32 Release"

# ADD CPP /Ob2

!ELSEIF  "$(CFG)" == "Lwo2Mesh - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\lwPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\src\lwReader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Vector3.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\BitArray.h
# End Source File
# Begin Source File

SOURCE=..\include\lwClip.h
# End Source File
# Begin Source File

SOURCE=..\include\lwEnvelope.h
# End Source File
# Begin Source File

SOURCE=..\include\lwLayer.h
# End Source File
# Begin Source File

SOURCE=..\include\lwo.h
# End Source File
# Begin Source File

SOURCE=..\include\lwo2mesh.h
# End Source File
# Begin Source File

SOURCE=..\include\lwObject.h
# End Source File
# Begin Source File

SOURCE=..\include\lwPolygon.h
# End Source File
# Begin Source File

SOURCE=..\include\lwReader.h
# End Source File
# Begin Source File

SOURCE=..\include\Point.h
# End Source File
# Begin Source File

SOURCE=..\include\Vector3.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
