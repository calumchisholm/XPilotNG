# Microsoft Developer Studio Project File - Name="xpilot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=xpilot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xpilot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilot.mak" CFG="xpilot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xpilot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xpilot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xpilot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\common\\" /I "..\..\common" /I "..\..\common\NT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\..\..\zlib\dll32\zlib.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\common" /I "..\..\common\NT" /I "..\..\..\zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nologo /verbose /incremental:no

!ENDIF 

# Begin Target

# Name "xpilot - Win32 Release"
# Name "xpilot - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\about.c
# End Source File
# Begin Source File

SOURCE=..\bitmaps.c
# End Source File
# Begin Source File

SOURCE=.\BSString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\checknames.c
# End Source File
# Begin Source File

SOURCE=..\client.c
# End Source File
# Begin Source File

SOURCE=..\..\common\cmw.c
# End Source File
# Begin Source File

SOURCE=..\colors.c
# End Source File
# Begin Source File

SOURCE=..\..\common\config.c
# End Source File
# Begin Source File

SOURCE=..\configure.c
# End Source File
# Begin Source File

SOURCE=..\datagram.c
# End Source File
# Begin Source File

SOURCE=..\default.c
# End Source File
# Begin Source File

SOURCE=..\..\common\error.c
# End Source File
# Begin Source File

SOURCE=..\gfx2d.c
# End Source File
# Begin Source File

SOURCE=..\guimap.c
# End Source File
# Begin Source File

SOURCE=..\guiobjects.c
# End Source File
# Begin Source File

SOURCE=..\join.c
# End Source File
# Begin Source File

SOURCE=..\..\common\list.c
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\math.c
# End Source File
# Begin Source File

SOURCE=..\..\common\net.c
# End Source File
# Begin Source File

SOURCE=..\netclient.c
# End Source File
# Begin Source File

SOURCE=..\paint.c
# End Source File
# Begin Source File

SOURCE=..\paintdata.c
# End Source File
# Begin Source File

SOURCE=..\painthud.c
# End Source File
# Begin Source File

SOURCE=..\paintmap.c
# End Source File
# Begin Source File

SOURCE=..\paintobjects.c
# End Source File
# Begin Source File

SOURCE=..\paintradar.c
# End Source File
# Begin Source File

SOURCE=..\..\common\portability.c
# End Source File
# Begin Source File

SOURCE=..\query.c
# End Source File
# Begin Source File

SOURCE=..\..\common\randommt.c
# End Source File
# Begin Source File

SOURCE=.\RecordDummy.c
# End Source File
# Begin Source File

SOURCE=..\shipshape.c
# End Source File
# Begin Source File

SOURCE=..\..\common\socklib.c
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\strcasecmp.c
# End Source File
# Begin Source File

SOURCE=..\syslimit.c
# End Source File
# Begin Source File

SOURCE=..\talkmacros.c
# End Source File
# Begin Source File

SOURCE=.\TalkWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\textinterface.c
# End Source File
# Begin Source File

SOURCE=..\texture.c
# End Source File
# Begin Source File

SOURCE=..\usleep.c
# End Source File
# Begin Source File

SOURCE=..\widget.c
# End Source File
# Begin Source File

SOURCE=.\winAbout.cpp
# End Source File
# Begin Source File

SOURCE=.\winAudio.c
# End Source File
# Begin Source File

SOURCE=.\winBitmap.c
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winNet.c
# End Source File
# Begin Source File

SOURCE=.\wintalk.c
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winX.c
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winX11.c
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winXKey.c
# End Source File
# Begin Source File

SOURCE=.\winXThread.c
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\wsockerrs.c
# End Source File
# Begin Source File

SOURCE=..\xevent.c
# End Source File
# Begin Source File

SOURCE=..\xeventhandlers.c
# End Source File
# Begin Source File

SOURCE=..\xinit.c
# End Source File
# Begin Source File

SOURCE=..\xpilot.c
# End Source File
# Begin Source File

SOURCE=.\xpilot.rc
# End Source File
# Begin Source File

SOURCE=.\xpilotDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\XPilotNT.cpp
# End Source File
# Begin Source File

SOURCE=.\xpilotView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\common\audio.h
# End Source File
# Begin Source File

SOURCE=..\..\common\bit.h
# End Source File
# Begin Source File

SOURCE=..\bitmaps.h
# End Source File
# Begin Source File

SOURCE=.\BSString.h
# End Source File
# Begin Source File

SOURCE=..\..\common\checknames.h
# End Source File
# Begin Source File

SOURCE=..\client.h
# End Source File
# Begin Source File

SOURCE=..\..\common\cmw.h
# End Source File
# Begin Source File

SOURCE=..\..\common\commonproto.h
# End Source File
# Begin Source File

SOURCE=..\..\common\config.h
# End Source File
# Begin Source File

SOURCE=..\configure.h
# End Source File
# Begin Source File

SOURCE=..\connectparam.h
# End Source File
# Begin Source File

SOURCE=..\..\common\const.h
# End Source File
# Begin Source File

SOURCE=..\datagram.h
# End Source File
# Begin Source File

SOURCE=..\..\common\draw.h
# End Source File
# Begin Source File

SOURCE=..\..\common\error.h
# End Source File
# Begin Source File

SOURCE=..\gfx2d.h
# End Source File
# Begin Source File

SOURCE=..\guimap.h
# End Source File
# Begin Source File

SOURCE=..\guiobjects.h
# End Source File
# Begin Source File

SOURCE=..\icon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\item.h
# End Source File
# Begin Source File

SOURCE=..\..\common\keys.h
# End Source File
# Begin Source File

SOURCE=..\..\common\list.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\..\common\net.h
# End Source File
# Begin Source File

SOURCE=..\netclient.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pack.h
# End Source File
# Begin Source File

SOURCE=..\..\common\packet.h
# End Source File
# Begin Source File

SOURCE=..\paint.h
# End Source File
# Begin Source File

SOURCE=..\paintdata.h
# End Source File
# Begin Source File

SOURCE=..\..\common\portability.h
# End Source File
# Begin Source File

SOURCE=..\protoclient.h
# End Source File
# Begin Source File

SOURCE=..\record.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\common\rules.h
# End Source File
# Begin Source File

SOURCE=..\..\common\setup.h
# End Source File
# Begin Source File

SOURCE=..\..\common\socklib.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\talk.h
# End Source File
# Begin Source File

SOURCE=.\TalkWindow.h
# End Source File
# Begin Source File

SOURCE=..\texture.h
# End Source File
# Begin Source File

SOURCE=..\..\common\types.h
# End Source File
# Begin Source File

SOURCE=..\..\common\version.h
# End Source File
# Begin Source File

SOURCE=..\widget.h
# End Source File
# Begin Source File

SOURCE=.\winAbout.h
# End Source File
# Begin Source File

SOURCE=.\winAudio.h
# End Source File
# Begin Source File

SOURCE=.\winBitmap.h
# End Source File
# Begin Source File

SOURCE=.\winClient.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winNet.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winX.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winX_.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nt\winXKey.h
# End Source File
# Begin Source File

SOURCE=.\winXThread.h
# End Source File
# Begin Source File

SOURCE=..\xeventhandlers.h
# End Source File
# Begin Source File

SOURCE=..\xinit.h
# End Source File
# Begin Source File

SOURCE=.\xpilotDoc.h
# End Source File
# Begin Source File

SOURCE=.\XPilotNT.h
# End Source File
# Begin Source File

SOURCE=.\xpilotView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
