# Microsoft Developer Studio Project File - Name="FlashWin32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=FlashWin32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FlashWin32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FlashWin32.mak" CFG="FlashWin32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FlashWin32 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FlashWin32 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FlashWin32 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "USE_JPEG" /D "FRAMESPERSEC" /D "SOUND" /D "EDITTEXT" /D "FLASHMP3" /FR /YX /FD /Zm150 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib advapi32.lib imm32.lib mp3decoder.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libcmt"

!ELSEIF  "$(CFG)" == "FlashWin32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_JPEG" /D "FRAMESPERSEC" /D "SOUND" /D "EDITTEXT" /D "FLASHMP3" /FR /YX /FD /Zm150 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib advapi32.lib imm32.lib mp3decoder.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmt" /pdbtype:sept
# SUBTRACT LINK32 /profile /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "FlashWin32 - Win32 Release"
# Name "FlashWin32 - Win32 Debug"
# Begin Group "ZLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infblock.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\infblock.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infcodes.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infutil.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\infutil.h
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.cpp
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.h
# End Source File
# End Group
# Begin Group "JPeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\JPEG\cderror.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\cdjpeg.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jcomapi.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdapimin.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdapistd.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdcoefct.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdcolor.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdct.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jddctmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdhuff.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdinput.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdmainct.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdmarker.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdmaster.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdpostct.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jdsample.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jfdctint.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jidctint.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jmemmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jmemnobs.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\JPEG\jutils.cpp
# End Source File
# Begin Source File

SOURCE=.\JPEG\jversion.h
# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Win32\button.cur
# End Source File
# Begin Source File

SOURCE=.\Win32\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Debug.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Display.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Fixed.h
# End Source File
# Begin Source File

SOURCE=.\Win32\hand.cur
# End Source File
# Begin Source File

SOURCE=.\Win32\Palette.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Palette.h
# End Source File
# Begin Source File

SOURCE=.\Win32\PlayerWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\PlayerWnd.h
# End Source File
# Begin Source File

SOURCE=.\Win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\Win32\SimpleWin.rc
# End Source File
# Begin Source File

SOURCE=.\Win32\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Sound.h
# End Source File
# Begin Source File

SOURCE=.\Win32\Util.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32\Util.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bitbuf.cpp
# End Source File
# Begin Source File

SOURCE=.\Bitbuf.h
# End Source File
# Begin Source File

SOURCE=.\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\curve.cpp
# End Source File
# Begin Source File

SOURCE=.\CURVE.H
# End Source File
# Begin Source File

SOURCE=.\edittext.cpp
# End Source File
# Begin Source File

SOURCE=.\edittext.h
# End Source File
# Begin Source File

SOURCE=.\fixed.cpp
# End Source File
# Begin Source File

SOURCE=.\fixed.h
# End Source File
# Begin Source File

SOURCE=.\FlashIcon.pcx
# End Source File
# Begin Source File

SOURCE=.\FlashIcon.tga
# End Source File
# Begin Source File

SOURCE=.\Win32\fsplayer.ico
# End Source File
# Begin Source File

SOURCE=.\genericfonts.h
# End Source File
# Begin Source File

SOURCE=.\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\GEOM.H
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\Global.h
# End Source File
# Begin Source File

SOURCE=.\JPEG.CPP
# End Source File
# Begin Source File

SOURCE=.\JPEG.H
# End Source File
# Begin Source File

SOURCE=.\mcontext.h
# End Source File
# Begin Source File

SOURCE=.\MEMCOP.CPP
# End Source File
# Begin Source File

SOURCE=.\MEMCOP.H
# End Source File
# Begin Source File

SOURCE=.\MorphInter.cpp
# End Source File
# Begin Source File

SOURCE=.\MorphInter.h
# End Source File
# Begin Source File

SOURCE=.\mp3decifc.h
# End Source File
# Begin Source File

SOURCE=.\mp3decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\mp3decoder.h
# End Source File
# Begin Source File

SOURCE=.\mp3sscdef.h
# End Source File
# Begin Source File

SOURCE=.\mp3streaminfo.h
# End Source File
# Begin Source File

SOURCE=.\mpegbitstream.h
# End Source File
# Begin Source File

SOURCE=.\mpegheader.h
# End Source File
# Begin Source File

SOURCE=.\RASTER.CPP
# End Source File
# Begin Source File

SOURCE=.\RASTER.H
# End Source File
# Begin Source File

SOURCE=.\SBITMAP.CPP
# End Source File
# Begin Source File

SOURCE=.\SBITMAP.H
# End Source File
# Begin Source File

SOURCE=.\sdisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\sdisplay.h
# End Source File
# Begin Source File

SOURCE=.\SHAPE.H
# End Source File
# Begin Source File

SOURCE=.\sndcodec.cpp
# End Source File
# Begin Source File

SOURCE=.\sndcodec.h
# End Source File
# Begin Source File

SOURCE=.\sndmix.cpp
# End Source File
# Begin Source File

SOURCE=.\sndmix.h
# End Source File
# Begin Source File

SOURCE=.\SOBJECT.CPP
# End Source File
# Begin Source File

SOURCE=.\Sobject.h
# End Source File
# Begin Source File

SOURCE=.\SPLAY.CPP
# End Source File
# Begin Source File

SOURCE=.\Splay.h
# End Source File
# Begin Source File

SOURCE=.\splayer.cpp
# End Source File
# Begin Source File

SOURCE=.\splayer.h
# End Source File
# Begin Source File

SOURCE=.\SSTROKER.CPP
# End Source File
# Begin Source File

SOURCE=.\SSTROKER.H
# End Source File
# Begin Source File

SOURCE=.\STAGS.H
# End Source File
# Begin Source File

SOURCE=.\unixfonts.cpp
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Target
# End Project
