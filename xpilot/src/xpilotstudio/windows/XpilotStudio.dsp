# Microsoft Developer Studio Project File - Name="XpilotStudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=XpilotStudio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XpilotStudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XpilotStudio.mak" CFG="XpilotStudio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XpilotStudio - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "XpilotStudio - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XpilotStudio - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib expat.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "XpilotStudio - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib expat.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "XpilotStudio - Win32 Release"
# Name "XpilotStudio - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\document.c
# End Source File
# Begin Source File

SOURCE=..\draw.c
# End Source File
# Begin Source File

SOURCE=..\errors.c
# End Source File
# Begin Source File

SOURCE=..\geometry.c
# End Source File
# Begin Source File

SOURCE=..\helper.c
# End Source File
# Begin Source File

SOURCE=..\map.c
# End Source File
# Begin Source File

SOURCE=..\map_file.c
# End Source File
# Begin Source File

SOURCE=..\polygon.c
# End Source File
# Begin Source File

SOURCE=..\ship.c
# End Source File
# Begin Source File

SOURCE=.\win_draw.c
# End Source File
# Begin Source File

SOURCE=.\win_file.c
# End Source File
# Begin Source File

SOURCE=.\win_helper.c
# End Source File
# Begin Source File

SOURCE=.\win_mapeditor.c
# End Source File
# Begin Source File

SOURCE=.\win_prefs.c
# End Source File
# Begin Source File

SOURCE=.\win_shipeditor.c
# End Source File
# Begin Source File

SOURCE=.\win_toolbar.c
# End Source File
# Begin Source File

SOURCE=.\win_xpstudio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\const.h
# End Source File
# Begin Source File

SOURCE=..\default_colors.h
# End Source File
# Begin Source File

SOURCE=..\default_settings.h
# End Source File
# Begin Source File

SOURCE=..\main.h
# End Source File
# Begin Source File

SOURCE=..\map.h
# End Source File
# Begin Source File

SOURCE=..\objects.h
# End Source File
# Begin Source File

SOURCE=..\proto.h
# End Source File
# Begin Source File

SOURCE=.\win_mapeditor.h
# End Source File
# Begin Source File

SOURCE=.\win_proto.h
# End Source File
# Begin Source File

SOURCE=.\win_resdefines.h
# End Source File
# Begin Source File

SOURCE=.\win_xpstudio.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\blocksbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mainicon.ico
# End Source File
# Begin Source File

SOURCE=.\res\maptoolsbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\polaritysubbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\shipsymsbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\shiptoolsbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\win_logo.bmp
# End Source File
# Begin Source File

SOURCE=.\win_xpstudio.rc
# End Source File
# Begin Source File

SOURCE=.\res\wormholesubbar.bmp
# End Source File
# End Group
# End Target
# End Project
