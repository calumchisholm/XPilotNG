# Microsoft Developer Studio Generated NMAKE File, Based on xpilot.dsp
!IF "$(CFG)" == ""
CFG=xpilot - Win32 Release
!MESSAGE No configuration specified. Defaulting to xpilot - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "xpilot - Win32 Release" && "$(CFG)" != "xpilot - Win32 Debug"\
 && "$(CFG)" != "xpilot - Win32 ReleasePentium"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xpilot.mak" CFG="xpilot - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xpilot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xpilot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "xpilot - Win32 ReleasePentium" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "xpilot - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ELSE 

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\bitmaps.obj"
	-@erase "$(INTDIR)\bitmaps.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx2d.obj"
	-@erase "$(INTDIR)\gfx2d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\randommt.obj"
	-@erase "$(INTDIR)\randommt.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winBitmap.obj"
	-@erase "$(INTDIR)\winBitmap.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\xpilot.exe"
	-@erase "$(OUTDIR)\xpilot.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /I "..\..\common" /D "NDEBUG" /D "_MBCS" /D\
 "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING" /D\
 PAINT_FREE=0 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xpilot.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\bitmaps.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx2d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\randommt.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winBitmap.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xeventhandlers.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\xpilot.pdb" /map:"$(INTDIR)\xpilot.map" /machine:I386\
 /out:"$(OUTDIR)\xpilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\bitmaps.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx2d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\randommt.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winBitmap.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xeventhandlers.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ELSE 

ALL : "$(OUTDIR)\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\bitmaps.obj"
	-@erase "$(INTDIR)\bitmaps.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx2d.obj"
	-@erase "$(INTDIR)\gfx2d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\randommt.obj"
	-@erase "$(INTDIR)\randommt.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winBitmap.obj"
	-@erase "$(INTDIR)\winBitmap.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\xpilot.exe"
	-@erase "$(OUTDIR)\xpilot.ilk"
	-@erase "$(OUTDIR)\xpilot.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\common" /D "_DEBUG" /D\
 "_MEMPOD" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D "WINDOWSCALING"\
 /D PAINT_FREE=0 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xpilot.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\bitmaps.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx2d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\randommt.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winBitmap.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xeventhandlers.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\xpilot.pdb" /debug /machine:I386 /out:"$(OUTDIR)\xpilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\bitmaps.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx2d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\randommt.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winBitmap.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xeventhandlers.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

"$(OUTDIR)\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

OUTDIR=.\xpilot__
INTDIR=.\xpilot__
# Begin Custom Macros
OutDir=.\.\xpilot__
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\Release\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ELSE 

ALL : ".\Release\xpilot.exe" "$(OUTDIR)\xpilot.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\bitmaps.obj"
	-@erase "$(INTDIR)\bitmaps.sbr"
	-@erase "$(INTDIR)\BSString.obj"
	-@erase "$(INTDIR)\BSString.sbr"
	-@erase "$(INTDIR)\caudio.obj"
	-@erase "$(INTDIR)\caudio.sbr"
	-@erase "$(INTDIR)\checknames.obj"
	-@erase "$(INTDIR)\checknames.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\colors.obj"
	-@erase "$(INTDIR)\colors.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\configure.obj"
	-@erase "$(INTDIR)\configure.sbr"
	-@erase "$(INTDIR)\datagram.obj"
	-@erase "$(INTDIR)\datagram.sbr"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\default.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\gfx2d.obj"
	-@erase "$(INTDIR)\gfx2d.sbr"
	-@erase "$(INTDIR)\guimap.obj"
	-@erase "$(INTDIR)\guimap.sbr"
	-@erase "$(INTDIR)\guiobjects.obj"
	-@erase "$(INTDIR)\guiobjects.sbr"
	-@erase "$(INTDIR)\join.obj"
	-@erase "$(INTDIR)\join.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\math.sbr"
	-@erase "$(INTDIR)\net.obj"
	-@erase "$(INTDIR)\net.sbr"
	-@erase "$(INTDIR)\netclient.obj"
	-@erase "$(INTDIR)\netclient.sbr"
	-@erase "$(INTDIR)\paint.obj"
	-@erase "$(INTDIR)\paint.sbr"
	-@erase "$(INTDIR)\paintdata.obj"
	-@erase "$(INTDIR)\paintdata.sbr"
	-@erase "$(INTDIR)\painthud.obj"
	-@erase "$(INTDIR)\painthud.sbr"
	-@erase "$(INTDIR)\paintmap.obj"
	-@erase "$(INTDIR)\paintmap.sbr"
	-@erase "$(INTDIR)\paintobjects.obj"
	-@erase "$(INTDIR)\paintobjects.sbr"
	-@erase "$(INTDIR)\paintradar.obj"
	-@erase "$(INTDIR)\paintradar.sbr"
	-@erase "$(INTDIR)\portability.obj"
	-@erase "$(INTDIR)\portability.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\randommt.obj"
	-@erase "$(INTDIR)\randommt.sbr"
	-@erase "$(INTDIR)\RecordDummy.obj"
	-@erase "$(INTDIR)\RecordDummy.sbr"
	-@erase "$(INTDIR)\shipshape.obj"
	-@erase "$(INTDIR)\shipshape.sbr"
	-@erase "$(INTDIR)\socklib.obj"
	-@erase "$(INTDIR)\socklib.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\syslimit.obj"
	-@erase "$(INTDIR)\syslimit.sbr"
	-@erase "$(INTDIR)\talkmacros.obj"
	-@erase "$(INTDIR)\talkmacros.sbr"
	-@erase "$(INTDIR)\TalkWindow.obj"
	-@erase "$(INTDIR)\TalkWindow.sbr"
	-@erase "$(INTDIR)\textinterface.obj"
	-@erase "$(INTDIR)\textinterface.sbr"
	-@erase "$(INTDIR)\texture.obj"
	-@erase "$(INTDIR)\texture.sbr"
	-@erase "$(INTDIR)\usleep.obj"
	-@erase "$(INTDIR)\usleep.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\widget.obj"
	-@erase "$(INTDIR)\widget.sbr"
	-@erase "$(INTDIR)\winAbout.obj"
	-@erase "$(INTDIR)\winAbout.sbr"
	-@erase "$(INTDIR)\winAudio.obj"
	-@erase "$(INTDIR)\winAudio.sbr"
	-@erase "$(INTDIR)\winBitmap.obj"
	-@erase "$(INTDIR)\winBitmap.sbr"
	-@erase "$(INTDIR)\winNet.obj"
	-@erase "$(INTDIR)\winNet.sbr"
	-@erase "$(INTDIR)\winX.obj"
	-@erase "$(INTDIR)\winX.sbr"
	-@erase "$(INTDIR)\winX11.obj"
	-@erase "$(INTDIR)\winX11.sbr"
	-@erase "$(INTDIR)\winXKey.obj"
	-@erase "$(INTDIR)\winXKey.sbr"
	-@erase "$(INTDIR)\winXThread.obj"
	-@erase "$(INTDIR)\winXThread.sbr"
	-@erase "$(INTDIR)\wsockerrs.obj"
	-@erase "$(INTDIR)\wsockerrs.sbr"
	-@erase "$(INTDIR)\xevent.obj"
	-@erase "$(INTDIR)\xevent.sbr"
	-@erase "$(INTDIR)\xeventhandlers.obj"
	-@erase "$(INTDIR)\xeventhandlers.sbr"
	-@erase "$(INTDIR)\xinit.obj"
	-@erase "$(INTDIR)\xinit.sbr"
	-@erase "$(INTDIR)\xpilot.obj"
	-@erase "$(INTDIR)\xpilot.res"
	-@erase "$(INTDIR)\xpilot.sbr"
	-@erase "$(INTDIR)\xpilotDoc.obj"
	-@erase "$(INTDIR)\xpilotDoc.sbr"
	-@erase "$(INTDIR)\XPilotNT.obj"
	-@erase "$(INTDIR)\XPilotNT.sbr"
	-@erase "$(INTDIR)\xpilotView.obj"
	-@erase "$(INTDIR)\xpilotView.sbr"
	-@erase "$(OUTDIR)\xpilot.bsc"
	-@erase "$(OUTDIR)\XPilot.map"
	-@erase ".\Release\xpilot.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MD /W3 /GX /Zd /O2 /I "..\..\common" /D "NDEBUG" /D\
 "_MBCS" /D "x_BETAEXPIRE" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "X_SOUND" /D\
 "WINDOWSCALING" /D PAINT_FREE=0 /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\xpilot__/
CPP_SBRS=.\xpilot__/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xpilot.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xpilot.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\bitmaps.sbr" \
	"$(INTDIR)\BSString.sbr" \
	"$(INTDIR)\caudio.sbr" \
	"$(INTDIR)\checknames.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\colors.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\configure.sbr" \
	"$(INTDIR)\datagram.sbr" \
	"$(INTDIR)\default.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\gfx2d.sbr" \
	"$(INTDIR)\guimap.sbr" \
	"$(INTDIR)\guiobjects.sbr" \
	"$(INTDIR)\join.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\math.sbr" \
	"$(INTDIR)\net.sbr" \
	"$(INTDIR)\netclient.sbr" \
	"$(INTDIR)\paint.sbr" \
	"$(INTDIR)\paintdata.sbr" \
	"$(INTDIR)\painthud.sbr" \
	"$(INTDIR)\paintmap.sbr" \
	"$(INTDIR)\paintobjects.sbr" \
	"$(INTDIR)\paintradar.sbr" \
	"$(INTDIR)\portability.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\randommt.sbr" \
	"$(INTDIR)\RecordDummy.sbr" \
	"$(INTDIR)\shipshape.sbr" \
	"$(INTDIR)\socklib.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\syslimit.sbr" \
	"$(INTDIR)\talkmacros.sbr" \
	"$(INTDIR)\TalkWindow.sbr" \
	"$(INTDIR)\textinterface.sbr" \
	"$(INTDIR)\texture.sbr" \
	"$(INTDIR)\usleep.sbr" \
	"$(INTDIR)\widget.sbr" \
	"$(INTDIR)\winAbout.sbr" \
	"$(INTDIR)\winAudio.sbr" \
	"$(INTDIR)\winBitmap.sbr" \
	"$(INTDIR)\winNet.sbr" \
	"$(INTDIR)\winX.sbr" \
	"$(INTDIR)\winX11.sbr" \
	"$(INTDIR)\winXKey.sbr" \
	"$(INTDIR)\winXThread.sbr" \
	"$(INTDIR)\wsockerrs.sbr" \
	"$(INTDIR)\xevent.sbr" \
	"$(INTDIR)\xeventhandlers.sbr" \
	"$(INTDIR)\xinit.sbr" \
	"$(INTDIR)\xpilot.sbr" \
	"$(INTDIR)\xpilotDoc.sbr" \
	"$(INTDIR)\XPilotNT.sbr" \
	"$(INTDIR)\xpilotView.sbr"

"$(OUTDIR)\xpilot.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\XPilot.pdb" /map:"$(INTDIR)\XPilot.map" /machine:I386\
 /out:".\Release\XPilot.exe" 
LINK32_OBJS= \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\bitmaps.obj" \
	"$(INTDIR)\BSString.obj" \
	"$(INTDIR)\caudio.obj" \
	"$(INTDIR)\checknames.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\colors.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\configure.obj" \
	"$(INTDIR)\datagram.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\gfx2d.obj" \
	"$(INTDIR)\guimap.obj" \
	"$(INTDIR)\guiobjects.obj" \
	"$(INTDIR)\join.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\net.obj" \
	"$(INTDIR)\netclient.obj" \
	"$(INTDIR)\paint.obj" \
	"$(INTDIR)\paintdata.obj" \
	"$(INTDIR)\painthud.obj" \
	"$(INTDIR)\paintmap.obj" \
	"$(INTDIR)\paintobjects.obj" \
	"$(INTDIR)\paintradar.obj" \
	"$(INTDIR)\portability.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\randommt.obj" \
	"$(INTDIR)\RecordDummy.obj" \
	"$(INTDIR)\shipshape.obj" \
	"$(INTDIR)\socklib.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\syslimit.obj" \
	"$(INTDIR)\talkmacros.obj" \
	"$(INTDIR)\TalkWindow.obj" \
	"$(INTDIR)\textinterface.obj" \
	"$(INTDIR)\texture.obj" \
	"$(INTDIR)\usleep.obj" \
	"$(INTDIR)\widget.obj" \
	"$(INTDIR)\winAbout.obj" \
	"$(INTDIR)\winAudio.obj" \
	"$(INTDIR)\winBitmap.obj" \
	"$(INTDIR)\winNet.obj" \
	"$(INTDIR)\winX.obj" \
	"$(INTDIR)\winX11.obj" \
	"$(INTDIR)\winXKey.obj" \
	"$(INTDIR)\winXThread.obj" \
	"$(INTDIR)\wsockerrs.obj" \
	"$(INTDIR)\xevent.obj" \
	"$(INTDIR)\xeventhandlers.obj" \
	"$(INTDIR)\xinit.obj" \
	"$(INTDIR)\xpilot.obj" \
	"$(INTDIR)\xpilot.res" \
	"$(INTDIR)\xpilotDoc.obj" \
	"$(INTDIR)\XPilotNT.obj" \
	"$(INTDIR)\xpilotView.obj"

".\Release\xpilot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "xpilot - Win32 Release" || "$(CFG)" == "xpilot - Win32 Debug"\
 || "$(CFG)" == "xpilot - Win32 ReleasePentium"
SOURCE=..\about.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_ABOUT=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\about.obj"	"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_ABOUT=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_ABOUT=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\about.obj"	"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_ABOUT=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_ABOUT=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\about.obj"	"$(INTDIR)\about.sbr" : $(SOURCE) $(DEP_CPP_ABOUT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\bitmaps.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_BITMA=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xpmread.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	

"$(INTDIR)\bitmaps.obj"	"$(INTDIR)\bitmaps.sbr" : $(SOURCE) $(DEP_CPP_BITMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_BITMA=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xpmread.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	

"$(INTDIR)\bitmaps.obj"	"$(INTDIR)\bitmaps.sbr" : $(SOURCE) $(DEP_CPP_BITMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_BITMA=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xpmread.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	

"$(INTDIR)\bitmaps.obj"	"$(INTDIR)\bitmaps.sbr" : $(SOURCE) $(DEP_CPP_BITMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\BSString.cpp
DEP_CPP_BSSTR=\
	".\BSString.h"\
	".\stdafx.h"\
	

"$(INTDIR)\BSString.obj"	"$(INTDIR)\BSString.sbr" : $(SOURCE) $(DEP_CPP_BSSTR)\
 "$(INTDIR)"


SOURCE=..\caudio.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CAUDI=\
	"..\..\common\version.h"\
	

"$(INTDIR)\caudio.obj"	"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CAUDI=\
	"..\..\common\audio.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	

"$(INTDIR)\caudio.obj"	"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CAUDI=\
	"..\..\common\audio.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	

"$(INTDIR)\caudio.obj"	"$(INTDIR)\caudio.sbr" : $(SOURCE) $(DEP_CPP_CAUDI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\checknames.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CHECK=\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\checknames.obj"	"$(INTDIR)\checknames.sbr" : $(SOURCE)\
 $(DEP_CPP_CHECK) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CHECK=\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\checknames.obj"	"$(INTDIR)\checknames.sbr" : $(SOURCE)\
 $(DEP_CPP_CHECK) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CHECK=\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\checknames.obj"	"$(INTDIR)\checknames.sbr" : $(SOURCE)\
 $(DEP_CPP_CHECK) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\client.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CLIEN=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winClient.h"\
	

"$(INTDIR)\client.obj"	"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CLIEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_CLIEN=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\client.obj"	"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CLIEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_CLIEN=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\client.obj"	"$(INTDIR)\client.sbr" : $(SOURCE) $(DEP_CPP_CLIEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\colors.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_COLOR=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\xinit.h"\
	

"$(INTDIR)\colors.obj"	"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_COLOR=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\xinit.h"\
	
NODEP_CPP_COLOR=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\colors.obj"	"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_COLOR=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\xinit.h"\
	
NODEP_CPP_COLOR=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\colors.obj"	"$(INTDIR)\colors.sbr" : $(SOURCE) $(DEP_CPP_COLOR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\config.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CONFI=\
	"..\..\common\config.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CONFI=\
	"..\..\common\config.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CONFI=\
	"..\..\common\config.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) $(DEP_CPP_CONFI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\configure.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_CONFIG=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\configure.obj"	"$(INTDIR)\configure.sbr" : $(SOURCE)\
 $(DEP_CPP_CONFIG) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_CONFIG=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_CONFIG=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\configure.obj"	"$(INTDIR)\configure.sbr" : $(SOURCE)\
 $(DEP_CPP_CONFIG) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_CONFIG=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_CONFIG=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\configure.obj"	"$(INTDIR)\configure.sbr" : $(SOURCE)\
 $(DEP_CPP_CONFIG) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\datagram.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_DATAG=\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	

"$(INTDIR)\datagram.obj"	"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_DATAG=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_DATAG=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\datagram.obj"	"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_DATAG=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_DATAG=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\datagram.obj"	"$(INTDIR)\datagram.sbr" : $(SOURCE) $(DEP_CPP_DATAG)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\default.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_DEFAU=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\default.obj"	"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_DEFAU=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_DEFAU=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\default.obj"	"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_DEFAU=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_DEFAU=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\default.obj"	"$(INTDIR)\default.sbr" : $(SOURCE) $(DEP_CPP_DEFAU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\error.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_ERROR=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	

"$(INTDIR)\error.obj"	"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_ERROR=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

"$(INTDIR)\error.obj"	"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_ERROR=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

"$(INTDIR)\error.obj"	"$(INTDIR)\error.sbr" : $(SOURCE) $(DEP_CPP_ERROR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\gfx2d.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_GFX2D=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\gfx2d.h"\
	

"$(INTDIR)\gfx2d.obj"	"$(INTDIR)\gfx2d.sbr" : $(SOURCE) $(DEP_CPP_GFX2D)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_GFX2D=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\gfx2d.h"\
	

"$(INTDIR)\gfx2d.obj"	"$(INTDIR)\gfx2d.sbr" : $(SOURCE) $(DEP_CPP_GFX2D)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_GFX2D=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\gfx2d.h"\
	

"$(INTDIR)\gfx2d.obj"	"$(INTDIR)\gfx2d.sbr" : $(SOURCE) $(DEP_CPP_GFX2D)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\guimap.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	

"$(INTDIR)\guimap.obj"	"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	
NODEP_CPP_GUIMA=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\guimap.obj"	"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_GUIMA=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	
NODEP_CPP_GUIMA=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\guimap.obj"	"$(INTDIR)\guimap.sbr" : $(SOURCE) $(DEP_CPP_GUIMA)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\guiobjects.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_GUIOB=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	

"$(INTDIR)\guiobjects.obj"	"$(INTDIR)\guiobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_GUIOB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_GUIOB=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	
NODEP_CPP_GUIOB=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\guiobjects.obj"	"$(INTDIR)\guiobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_GUIOB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_GUIOB=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winBitmap.h"\
	".\winClient.h"\
	
NODEP_CPP_GUIOB=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\guiobjects.obj"	"$(INTDIR)\guiobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_GUIOB) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\join.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_JOIN_=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	

"$(INTDIR)\join.obj"	"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_JOIN_=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	
NODEP_CPP_JOIN_=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\join.obj"	"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_JOIN_=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	
NODEP_CPP_JOIN_=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\join.obj"	"$(INTDIR)\join.sbr" : $(SOURCE) $(DEP_CPP_JOIN_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_MAINF=\
	"..\..\common\NT\winX.h"\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\winXXPilot.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_MAINF=\
	"..\..\common\NT\winX.h"\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\winXXPilot.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_MAINF=\
	"..\..\common\NT\winX.h"\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\winXXPilot.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF)\
 "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\math.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_MATH_=\
	"..\..\common\commonproto.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\math.obj"	"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_MATH_=\
	"..\..\common\commonproto.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\math.obj"	"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_MATH_=\
	"..\..\common\commonproto.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\math.obj"	"$(INTDIR)\math.sbr" : $(SOURCE) $(DEP_CPP_MATH_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\net.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_NET_C=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\packet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	

"$(INTDIR)\net.obj"	"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_NET_C=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\packet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\net.obj"	"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_NET_C=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\packet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\net.obj"	"$(INTDIR)\net.sbr" : $(SOURCE) $(DEP_CPP_NET_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\netclient.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_NETCL=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\netclient.obj"	"$(INTDIR)\netclient.sbr" : $(SOURCE)\
 $(DEP_CPP_NETCL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_NETCL=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_NETCL=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\netclient.obj"	"$(INTDIR)\netclient.sbr" : $(SOURCE)\
 $(DEP_CPP_NETCL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_NETCL=\
	"..\..\common\audio.h"\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\pack.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_NETCL=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\netclient.obj"	"$(INTDIR)\netclient.sbr" : $(SOURCE)\
 $(DEP_CPP_NETCL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\paint.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINT=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paint.obj"	"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paint.obj"	"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paint.obj"	"$(INTDIR)\paint.sbr" : $(SOURCE) $(DEP_CPP_PAINT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\paintdata.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTD=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\xinit.h"\
	

"$(INTDIR)\paintdata.obj"	"$(INTDIR)\paintdata.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTD=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTD=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintdata.obj"	"$(INTDIR)\paintdata.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTD=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\dbuff.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTD=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintdata.obj"	"$(INTDIR)\paintdata.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\painthud.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTH=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	

"$(INTDIR)\painthud.obj"	"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTH=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINTH=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\painthud.obj"	"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTH=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINTH=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\painthud.obj"	"$(INTDIR)\painthud.sbr" : $(SOURCE) $(DEP_CPP_PAINTH)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\paintmap.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTM=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\guimap.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	

"$(INTDIR)\paintmap.obj"	"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTM=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\guimap.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTM=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintmap.obj"	"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTM=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\guimap.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	
NODEP_CPP_PAINTM=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintmap.obj"	"$(INTDIR)\paintmap.sbr" : $(SOURCE) $(DEP_CPP_PAINTM)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\paintobjects.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTO=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\gfx3d.h"\
	"..\guiobjects.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	

"$(INTDIR)\paintobjects.obj"	"$(INTDIR)\paintobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTO=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\gfx3d.h"\
	"..\guiobjects.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINTO=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintobjects.obj"	"$(INTDIR)\paintobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTO=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\gfx3d.h"\
	"..\guiobjects.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\record.h"\
	"..\texture.h"\
	"..\xinit.h"\
	".\winClient.h"\
	
NODEP_CPP_PAINTO=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\paintobjects.obj"	"$(INTDIR)\paintobjects.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\paintradar.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PAINTR=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paintradar.obj"	"$(INTDIR)\paintradar.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PAINTR=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paintradar.obj"	"$(INTDIR)\paintradar.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PAINTR=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\xinit.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\paintradar.obj"	"$(INTDIR)\paintradar.sbr" : $(SOURCE)\
 $(DEP_CPP_PAINTR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\portability.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_PORTA=\
	"..\..\common\config.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\portability.obj"	"$(INTDIR)\portability.sbr" : $(SOURCE)\
 $(DEP_CPP_PORTA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_PORTA=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	
NODEP_CPP_PORTA=\
	"..\..\common\username.h"\
	

"$(INTDIR)\portability.obj"	"$(INTDIR)\portability.sbr" : $(SOURCE)\
 $(DEP_CPP_PORTA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_PORTA=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	
NODEP_CPP_PORTA=\
	"..\..\common\username.h"\
	

"$(INTDIR)\portability.obj"	"$(INTDIR)\portability.sbr" : $(SOURCE)\
 $(DEP_CPP_PORTA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\query.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_QUERY=\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	

"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_QUERY=\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_QUERY=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_QUERY=\
	"..\..\common\config.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_QUERY=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) $(DEP_CPP_QUERY)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\randommt.c

"$(INTDIR)\randommt.obj"	"$(INTDIR)\randommt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\RecordDummy.c
DEP_CPP_RECOR=\
	"..\..\common\NT\winX.h"\
	"..\record.h"\
	

"$(INTDIR)\RecordDummy.obj"	"$(INTDIR)\RecordDummy.sbr" : $(SOURCE)\
 $(DEP_CPP_RECOR) "$(INTDIR)"


SOURCE=..\..\common\shipshape.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_SHIPS=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	".\winClient.h"\
	

"$(INTDIR)\shipshape.obj"	"$(INTDIR)\shipshape.sbr" : $(SOURCE)\
 $(DEP_CPP_SHIPS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_SHIPS=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

"$(INTDIR)\shipshape.obj"	"$(INTDIR)\shipshape.sbr" : $(SOURCE)\
 $(DEP_CPP_SHIPS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_SHIPS=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\version.h"\
	"..\..\server\NT\winServer.h"\
	".\winClient.h"\
	

"$(INTDIR)\shipshape.obj"	"$(INTDIR)\shipshape.sbr" : $(SOURCE)\
 $(DEP_CPP_SHIPS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\socklib.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_SOCKL=\
	"..\..\common\NT\winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	

"$(INTDIR)\socklib.obj"	"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_SOCKL=\
	"..\..\common\cmw.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_SOCKL=\
	"..\..\common\termnet.h"\
	

"$(INTDIR)\socklib.obj"	"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_SOCKL=\
	"..\..\common\cmw.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_SOCKL=\
	"..\..\common\termnet.h"\
	

"$(INTDIR)\socklib.obj"	"$(INTDIR)\socklib.sbr" : $(SOURCE) $(DEP_CPP_SOCKL)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Splash.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_SPLAS=\
	"..\..\common\version.h"\
	".\Splash.h"\
	".\stdafx.h"\
	

"$(INTDIR)\Splash.obj"	"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_SPLAS=\
	"..\..\common\version.h"\
	".\Splash.h"\
	".\stdafx.h"\
	

"$(INTDIR)\Splash.obj"	"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_SPLAS=\
	"..\..\common\version.h"\
	".\Splash.h"\
	".\stdafx.h"\
	

"$(INTDIR)\Splash.obj"	"$(INTDIR)\Splash.sbr" : $(SOURCE) $(DEP_CPP_SPLAS)\
 "$(INTDIR)"


!ENDIF 

SOURCE=..\syslimit.c

"$(INTDIR)\syslimit.obj"	"$(INTDIR)\syslimit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\talkmacros.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_TALKM=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\talkmacros.obj"	"$(INTDIR)\talkmacros.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_TALKM=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_TALKM=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\talkmacros.obj"	"$(INTDIR)\talkmacros.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_TALKM=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\rules.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\netclient.h"\
	"..\protoclient.h"\
	"..\talk.h"\
	".\winAudio.h"\
	".\winClient.h"\
	".\winXThread.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_TALKM=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\talkmacros.obj"	"$(INTDIR)\talkmacros.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\TalkWindow.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_TALKW=\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\TalkWindow.obj"	"$(INTDIR)\TalkWindow.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_TALKW=\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\TalkWindow.obj"	"$(INTDIR)\TalkWindow.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_TALKW=\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\TalkWindow.obj"	"$(INTDIR)\TalkWindow.sbr" : $(SOURCE)\
 $(DEP_CPP_TALKW) "$(INTDIR)"


!ENDIF 

SOURCE=..\textinterface.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	

"$(INTDIR)\textinterface.obj"	"$(INTDIR)\textinterface.sbr" : $(SOURCE)\
 $(DEP_CPP_TEXTI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_TEXTI=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\textinterface.obj"	"$(INTDIR)\textinterface.sbr" : $(SOURCE)\
 $(DEP_CPP_TEXTI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_TEXTI=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_TEXTI=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\textinterface.obj"	"$(INTDIR)\textinterface.sbr" : $(SOURCE)\
 $(DEP_CPP_TEXTI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\texture.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_TEXTU=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\texture.h"\
	"..\xinit.h"\
	"..\xpmread.h"\
	

"$(INTDIR)\texture.obj"	"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_TEXTU=\
	"..\..\..\lib\textures\ball.xpm"\
	"..\..\..\lib\textures\rock4.xpm"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\texture.h"\
	"..\xinit.h"\
	"..\xpmread.h"\
	

"$(INTDIR)\texture.obj"	"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_TEXTU=\
	"..\..\..\lib\textures\ball.xpm"\
	"..\..\..\lib\textures\rock4.xpm"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\texture.h"\
	"..\xinit.h"\
	"..\xpmread.h"\
	

"$(INTDIR)\texture.obj"	"$(INTDIR)\texture.sbr" : $(SOURCE) $(DEP_CPP_TEXTU)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\usleep.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_USLEE=\
	"..\..\common\NT\winNet.h"\
	

"$(INTDIR)\usleep.obj"	"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_USLEE=\
	"..\..\common\NT\winNet.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\usleep.obj"	"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_USLEE=\
	"..\..\common\NT\winNet.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\usleep.obj"	"$(INTDIR)\usleep.sbr" : $(SOURCE) $(DEP_CPP_USLEE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\widget.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WIDGE=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\widget.obj"	"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WIDGE=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_WIDGE=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\widget.obj"	"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WIDGE=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_WIDGE=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\widget.obj"	"$(INTDIR)\widget.sbr" : $(SOURCE) $(DEP_CPP_WIDGE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\winAbout.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINAB=\
	"..\..\common\version.h"\
	".\stdafx.h"\
	".\winAbout.h"\
	

"$(INTDIR)\winAbout.obj"	"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINAB=\
	"..\..\common\version.h"\
	".\stdafx.h"\
	".\winAbout.h"\
	

"$(INTDIR)\winAbout.obj"	"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINAB=\
	"..\..\common\version.h"\
	".\stdafx.h"\
	".\winAbout.h"\
	

"$(INTDIR)\winAbout.obj"	"$(INTDIR)\winAbout.sbr" : $(SOURCE) $(DEP_CPP_WINAB)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\winAudio.c

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winAudio.obj"	"$(INTDIR)\winAudio.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINAU=\
	".\winAudio.h"\
	

"$(INTDIR)\winAudio.obj"	"$(INTDIR)\winAudio.sbr" : $(SOURCE) $(DEP_CPP_WINAU)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINAU=\
	".\winAudio.h"\
	

"$(INTDIR)\winAudio.obj"	"$(INTDIR)\winAudio.sbr" : $(SOURCE) $(DEP_CPP_WINAU)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\winBitmap.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINBI=\
	"..\..\common\bit.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\..\common\setup.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	".\winBitmap.h"\
	

"$(INTDIR)\winBitmap.obj"	"$(INTDIR)\winBitmap.sbr" : $(SOURCE)\
 $(DEP_CPP_WINBI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINBI=\
	"..\..\common\bit.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\error.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\..\common\setup.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	".\winBitmap.h"\
	
NODEP_CPP_WINBI=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\winBitmap.obj"	"$(INTDIR)\winBitmap.sbr" : $(SOURCE)\
 $(DEP_CPP_WINBI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINBI=\
	"..\..\common\bit.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\error.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\..\common\setup.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\gfx2d.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	".\winBitmap.h"\
	
NODEP_CPP_WINBI=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\winBitmap.obj"	"$(INTDIR)\winBitmap.sbr" : $(SOURCE)\
 $(DEP_CPP_WINBI) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\NT\winNet.c

!IF  "$(CFG)" == "xpilot - Win32 Release"


"$(INTDIR)\winNet.obj"	"$(INTDIR)\winNet.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINNE=\
	"..\..\common\error.h"\
	

"$(INTDIR)\winNet.obj"	"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINNE=\
	"..\..\common\error.h"\
	

"$(INTDIR)\winNet.obj"	"$(INTDIR)\winNet.sbr" : $(SOURCE) $(DEP_CPP_WINNE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\NT\winX.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINX_=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXThread.h"\
	

"$(INTDIR)\winX.obj"	"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINX_=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\error.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\winX.obj"	"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINX_=\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\error.h"\
	"..\..\common\item.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	"..\client.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXThread.h"\
	
NODEP_CPP_WINX_=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\winX.obj"	"$(INTDIR)\winX.sbr" : $(SOURCE) $(DEP_CPP_WINX_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\NT\winX11.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINX1=\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winClient.h"\
	

"$(INTDIR)\winX11.obj"	"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINX1=\
	"..\..\common\error.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winClient.h"\
	

"$(INTDIR)\winX11.obj"	"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINX1=\
	"..\..\common\error.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winClient.h"\
	

"$(INTDIR)\winX11.obj"	"$(INTDIR)\winX11.sbr" : $(SOURCE) $(DEP_CPP_WINX1)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\NT\winXKey.c
DEP_CPP_WINXK=\
	"..\..\common\NT\winX.h"\
	

"$(INTDIR)\winXKey.obj"	"$(INTDIR)\winXKey.sbr" : $(SOURCE) $(DEP_CPP_WINXK)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\winXThread.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_WINXT=\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winXThread.h"\
	

"$(INTDIR)\winXThread.obj"	"$(INTDIR)\winXThread.sbr" : $(SOURCE)\
 $(DEP_CPP_WINXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_WINXT=\
	"..\..\common\error.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winXThread.h"\
	

"$(INTDIR)\winXThread.obj"	"$(INTDIR)\winXThread.sbr" : $(SOURCE)\
 $(DEP_CPP_WINXT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_WINXT=\
	"..\..\common\error.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\nt\winx_.h"\
	".\winXThread.h"\
	

"$(INTDIR)\winXThread.obj"	"$(INTDIR)\winXThread.sbr" : $(SOURCE)\
 $(DEP_CPP_WINXT) "$(INTDIR)"


!ENDIF 

SOURCE=..\..\common\NT\wsockerrs.c

"$(INTDIR)\wsockerrs.obj"	"$(INTDIR)\wsockerrs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\xevent.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XEVEN=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xeventhandlers.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xevent.obj"	"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XEVEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xeventhandlers.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xevent.obj"	"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XEVEN=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xeventhandlers.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xevent.obj"	"$(INTDIR)\xevent.sbr" : $(SOURCE) $(DEP_CPP_XEVEN)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\xeventhandlers.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xeventhandlers.obj"	"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE)\
 $(DEP_CPP_XEVENT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xeventhandlers.obj"	"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE)\
 $(DEP_CPP_XEVENT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XEVENT=\
	"..\..\common\bit.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\NT\winXKey.h"\
	"..\..\common\packet.h"\
	"..\..\common\portability.h"\
	"..\..\common\setup.h"\
	"..\..\common\version.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\paintdata.h"\
	"..\record.h"\
	"..\talk.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winAudio.h"\
	".\winClient.h"\
	

"$(INTDIR)\xeventhandlers.obj"	"$(INTDIR)\xeventhandlers.sbr" : $(SOURCE)\
 $(DEP_CPP_XEVENT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\xinit.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XINIT=\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\icon.h"\
	"..\items\itemafterburner.xbm"\
	"..\items\itemarmor.xbm"\
	"..\items\itemautopilot.xbm"\
	"..\items\itemcloakingdevice.xbm"\
	"..\items\itemdeflector.xbm"\
	"..\items\itemecm.xbm"\
	"..\items\itememergencyshield.xbm"\
	"..\items\itememergencythrust.xbm"\
	"..\items\itemenergypack.xbm"\
	"..\items\itemhyperjump.xbm"\
	"..\items\itemlaser.xbm"\
	"..\items\itemminepack.xbm"\
	"..\items\itemmirror.xbm"\
	"..\items\itemphasingdevice.xbm"\
	"..\items\itemrearshot.xbm"\
	"..\items\itemrocketpack.xbm"\
	"..\items\itemsensorpack.xbm"\
	"..\items\itemtank.xbm"\
	"..\items\itemtractorbeam.xbm"\
	"..\items\itemtransporter.xbm"\
	"..\items\itemwideangleshot.xbm"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	

"$(INTDIR)\xinit.obj"	"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XINIT=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\icon.h"\
	"..\items\itemafterburner.xbm"\
	"..\items\itemarmor.xbm"\
	"..\items\itemautopilot.xbm"\
	"..\items\itemcloakingdevice.xbm"\
	"..\items\itemdeflector.xbm"\
	"..\items\itemecm.xbm"\
	"..\items\itememergencyshield.xbm"\
	"..\items\itememergencythrust.xbm"\
	"..\items\itemenergypack.xbm"\
	"..\items\itemhyperjump.xbm"\
	"..\items\itemlaser.xbm"\
	"..\items\itemminepack.xbm"\
	"..\items\itemmirror.xbm"\
	"..\items\itemphasingdevice.xbm"\
	"..\items\itemrearshot.xbm"\
	"..\items\itemrocketpack.xbm"\
	"..\items\itemsensorpack.xbm"\
	"..\items\itemtank.xbm"\
	"..\items\itemtractorbeam.xbm"\
	"..\items\itemtransporter.xbm"\
	"..\items\itemwideangleshot.xbm"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_XINIT=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\xinit.obj"	"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XINIT=\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\draw.h"\
	"..\..\common\item.h"\
	"..\..\common\keys.h"\
	"..\..\common\NT\winX.h"\
	"..\..\common\portability.h"\
	"..\..\common\version.h"\
	"..\bitmaps.h"\
	"..\client.h"\
	"..\configure.h"\
	"..\dbuff.h"\
	"..\gfx2d.h"\
	"..\icon.h"\
	"..\items\itemafterburner.xbm"\
	"..\items\itemarmor.xbm"\
	"..\items\itemautopilot.xbm"\
	"..\items\itemcloakingdevice.xbm"\
	"..\items\itemdeflector.xbm"\
	"..\items\itemecm.xbm"\
	"..\items\itememergencyshield.xbm"\
	"..\items\itememergencythrust.xbm"\
	"..\items\itemenergypack.xbm"\
	"..\items\itemhyperjump.xbm"\
	"..\items\itemlaser.xbm"\
	"..\items\itemminepack.xbm"\
	"..\items\itemmirror.xbm"\
	"..\items\itemphasingdevice.xbm"\
	"..\items\itemrearshot.xbm"\
	"..\items\itemrocketpack.xbm"\
	"..\items\itemsensorpack.xbm"\
	"..\items\itemtank.xbm"\
	"..\items\itemtractorbeam.xbm"\
	"..\items\itemtransporter.xbm"\
	"..\items\itemwideangleshot.xbm"\
	"..\netclient.h"\
	"..\paint.h"\
	"..\protoclient.h"\
	"..\widget.h"\
	"..\xinit.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	
NODEP_CPP_XINIT=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\xinit.obj"	"$(INTDIR)\xinit.sbr" : $(SOURCE) $(DEP_CPP_XINIT)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\xpilot.c

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XPILO=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	

"$(INTDIR)\xpilot.obj"	"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XPILO=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_XPILO=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\xpilot.obj"	"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XPILO=\
	"..\..\common\bit.h"\
	"..\..\common\checknames.h"\
	"..\..\common\cmw.h"\
	"..\..\common\config.h"\
	"..\..\common\const.h"\
	"..\..\common\net.h"\
	"..\..\common\NT\winNet.h"\
	"..\..\common\pack.h"\
	"..\..\common\portability.h"\
	"..\..\common\socklib.h"\
	"..\..\common\version.h"\
	"..\connectparam.h"\
	"..\protoclient.h"\
	".\winClient.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_XPILO=\
	"..\strcasecmp.h"\
	

"$(INTDIR)\xpilot.obj"	"$(INTDIR)\xpilot.sbr" : $(SOURCE) $(DEP_CPP_XPILO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\xpilot.rc
DEP_RSC_XPILOT=\
	".\res\Splsh16.bmp"\
	".\res\xpilot.ico"\
	".\res\xpilot.rc2"\
	

"$(INTDIR)\xpilot.res" : $(SOURCE) $(DEP_RSC_XPILOT) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\xpilotDoc.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XPILOTD=\
	".\stdafx.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\xpilotDoc.obj"	"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XPILOTD=\
	".\stdafx.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\xpilotDoc.obj"	"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XPILOTD=\
	".\stdafx.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	

"$(INTDIR)\xpilotDoc.obj"	"$(INTDIR)\xpilotDoc.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTD) "$(INTDIR)"


!ENDIF 

SOURCE=.\XPilotNT.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XPILOTN=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\winAbout.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\XPilotNT.obj"	"$(INTDIR)\XPilotNT.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XPILOTN=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\winAbout.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\XPilotNT.obj"	"$(INTDIR)\XPilotNT.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XPILOTN=\
	".\MainFrm.h"\
	".\Splash.h"\
	".\stdafx.h"\
	".\TalkWindow.h"\
	".\winAbout.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\XPilotNT.obj"	"$(INTDIR)\XPilotNT.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTN) "$(INTDIR)"


!ENDIF 

SOURCE=.\xpilotView.cpp

!IF  "$(CFG)" == "xpilot - Win32 Release"

DEP_CPP_XPILOTV=\
	"..\..\common\NT\winX.h"\
	".\BSString.h"\
	".\stdafx.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\xpilotView.obj"	"$(INTDIR)\xpilotView.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 Debug"

DEP_CPP_XPILOTV=\
	"..\..\common\NT\winX.h"\
	".\BSString.h"\
	".\stdafx.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\xpilotView.obj"	"$(INTDIR)\xpilotView.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xpilot - Win32 ReleasePentium"

DEP_CPP_XPILOTV=\
	"..\..\common\NT\winX.h"\
	".\BSString.h"\
	".\stdafx.h"\
	".\winClient.h"\
	".\winXXPilot.h"\
	".\xpilotDoc.h"\
	".\XPilotNT.h"\
	".\xpilotView.h"\
	

"$(INTDIR)\xpilotView.obj"	"$(INTDIR)\xpilotView.sbr" : $(SOURCE)\
 $(DEP_CPP_XPILOTV) "$(INTDIR)"


!ENDIF 


!ENDIF 

