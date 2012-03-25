# Microsoft Developer Studio Project File - Name="Alib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Alib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Alib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Alib.mak" CFG="Alib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Alib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Alib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Alib - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\libpng" /I "..\zlib" /I "..\..\sdk\inc\alib" /D "NDEBUG" /D "GENERATOR" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALIB_EXPORTS" /D "USE_JPEG" /D "FRAMESPERSEC" /D "EDITTEXT" /D "FLASHMP3" /D "MMX" /D WINVER=0x0500 /YX /FD /Zm300 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libpng.lib zlib.lib winmm.lib ddraw.lib dxguid.lib amstrmid.lib quartz.lib strmbase.lib msimg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib /nologo /dll /machine:I386 /out:"..\..\elektronika\alib100.dll" /libpath:"..\..\sdk\lib"
# SUBTRACT LINK32 /debug
# Begin Custom Build
InputPath=\_code\YoY\elektroSDK\elektronika\alib100.dll
SOURCE="$(InputPath)"

"..\..\sdk\lib\alib100.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy release\alib100.lib ..\..\sdk\lib\alib100.lib

# End Custom Build

!ELSEIF  "$(CFG)" == "Alib - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\libpng" /I "..\zlib" /I "..\..\sdk\inc\alib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALIB_EXPORTS" /D "USE_JPEG" /D "FRAMESPERSEC" /D "EDITTEXT" /D "FLASHMP3" /D WINVER=0x0500 /D "MMX" /FR /YX /FD /Zm300 /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libpngd.lib zlibd.lib strmiids.lib winmm.lib ddraw.lib dxguid.lib amstrmid.lib quartz.lib strmbase.lib msimg32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"..\..\elektronika\alib100.dll" /pdbtype:sept /libpath:"..\..\sdk\lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build
InputPath=\_code\YoY\elektroSDK\elektronika\alib100.dll
SOURCE="$(InputPath)"

"..\..\sdk\lib\alib100.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy debug\alib100.lib ..\..\sdk\lib\alib100.lib

# End Custom Build

!ENDIF 

# Begin Target

# Name "Alib - Win32 Release"
# Name "Alib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\button.cpp
# End Source File
# Begin Source File

SOURCE=.\clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\display.cpp
# End Source File
# Begin Source File

SOURCE=.\edit.cpp
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\filedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\filehd.cpp
# End Source File
# Begin Source File

SOURCE=.\filemem.cpp
# End Source File
# Begin Source File

SOURCE=.\flash.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\interface.cpp
# End Source File
# Begin Source File

SOURCE=.\list.cpp
# End Source File
# Begin Source File

SOURCE=.\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\movie.cpp
# End Source File
# Begin Source File

SOURCE=.\msgbox.cpp
# End Source File
# Begin Source File

SOURCE=.\node.cpp
# End Source File
# Begin Source File

SOURCE=.\object.cpp
# End Source File
# Begin Source File

SOURCE=.\oglbitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\pipe.cpp
# End Source File
# Begin Source File

SOURCE=.\plugz.cpp
# End Source File
# Begin Source File

SOURCE=.\point.cpp
# End Source File
# Begin Source File

SOURCE=.\rectangle.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\resources.cpp
# End Source File
# Begin Source File

SOURCE=.\section.cpp
# End Source File
# Begin Source File

SOURCE=.\segment.cpp
# End Source File
# Begin Source File

SOURCE=.\static.cpp
# End Source File
# Begin Source File

SOURCE=.\surface.cpp
# End Source File
# Begin Source File

SOURCE=.\thread.cpp
# End Source File
# Begin Source File

SOURCE=.\titleBar.cpp
# End Source File
# Begin Source File

SOURCE=.\tooltips.cpp
# End Source File
# Begin Source File

SOURCE=.\WFLILIB.CPP
# End Source File
# Begin Source File

SOURCE=.\window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\bitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\button.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\clipboard.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\display.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\edit.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\export.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\file.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\filedlg.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\filehd.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\filemem.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\flash.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\font.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\interface.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\layer.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\list.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\movie.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\msgbox.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\node.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\object.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\oglbitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\pipe.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\plugz.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\point.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\rectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\registry.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\resources.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\section.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\segment.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\static.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\surface.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\titleBar.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\tooltips.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\types.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\WFLILIB.H
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\ALib\window.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\cursors\10.cur
# End Source File
# Begin Source File

SOURCE=.\cursors\12.cur
# End Source File
# Begin Source File

SOURCE=.\cursors\14.cur
# End Source File
# Begin Source File

SOURCE=.\cursors\70.cur
# End Source File
# Begin Source File

SOURCE=.\alib.rc
# End Source File
# Begin Source File

SOURCE=".\button  check.png"
# End Source File
# Begin Source File

SOURCE=".\button close.png"
# End Source File
# Begin Source File

SOURCE=".\button minimize.png"
# End Source File
# Begin Source File

SOURCE=".\comic font 16.png"
# End Source File
# Begin Source File

SOURCE=.\display001.png
# End Source File
# Begin Source File

SOURCE=".\font arial bold 10.png"
# End Source File
# Begin Source File

SOURCE=".\font arial bold 12.png"
# End Source File
# Begin Source File

SOURCE=".\font arial bold 20.png"
# End Source File
# Begin Source File

SOURCE=".\font arial bold 8.png"
# End Source File
# Begin Source File

SOURCE=".\font segment 10.png"
# End Source File
# Begin Source File

SOURCE=".\font terminal 12.png"
# End Source File
# Begin Source File

SOURCE=".\font terminal 6.png"
# End Source File
# Begin Source File

SOURCE=".\font terminal 9.png"
# End Source File
# Begin Source File

SOURCE=.\font_confidential_14.png
# End Source File
# Begin Source File

SOURCE=.\font_confidential_16.png
# End Source File
# Begin Source File

SOURCE=.\font_confidential_20.png
# End Source File
# Begin Source File

SOURCE=.\font_confidential_24.png
# End Source File
# Begin Source File

SOURCE=.\font_confidential_36.png
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\fsplayer.ICO
# End Source File
# Begin Source File

SOURCE=.\paddle001.png
# End Source File
# Begin Source File

SOURCE=.\paddle002.png
# End Source File
# Begin Source File

SOURCE=.\paddle003.png
# End Source File
# Begin Source File

SOURCE=.\paddle004.png
# End Source File
# Begin Source File

SOURCE=.\paddle005.png
# End Source File
# Begin Source File

SOURCE=.\paddle006.png
# End Source File
# Begin Source File

SOURCE=.\paddle007.png
# End Source File
# Begin Source File

SOURCE=.\paddle008.png
# End Source File
# End Group
# Begin Group "flash SDK"

# PROP Default_Filter ""
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\adler32.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infblock.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infblock.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infcodes.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\inffast.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\inflate.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\inftrees.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infutil.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\infutil.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\uncompr.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\zutil.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\zlib\zutil.h
# End Source File
# End Group
# Begin Group "jpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\cderror.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\cdjpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\cjpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcapimin.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcapistd.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jccoefct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jccolor.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcdctmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jchuff.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcinit.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcmainct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcmarker.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcmaster.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcomapi.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcparam.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcphuff.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcprepct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jcsample.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdapimin.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdapistd.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdatadst.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdatasrc.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdcoefct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdcolor.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdct.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jddctmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdhuff.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdinput.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdmainct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdmarker.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdmaster.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdphuff.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdpostct.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jdsample.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jerror.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jfdctflt.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jfdctint.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jidctflt.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jidctint.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jmemmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jmemnobs.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jutils.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\jversion.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdbmp.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdcolmap.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdgif.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdppm.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdrle.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdswitch.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\rdtarga.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\wrbmp.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\wrgif.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\wrppm.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\wrrle.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\JPEG\wrtarga.cpp
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\_Display.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\_display.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\button.cur
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\debug.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\fixed.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\hand.cur
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\palette.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\PlayerWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\playerwnd.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\SimpleWin.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\Sound.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\util.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\util.h
# End Source File
# End Group
# Begin Group "sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\flashSDK\Source\bitbuf.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Bitbuf.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\curve.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\curve.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\edittext.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\edittext.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\fixed.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\fixed.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\FlashIcon.pcx
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\FlashIcon.tga
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\genericfonts.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\geom.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\global.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\global.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\jpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\jpeg.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mcontext.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\memcop.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\memcop.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\morphinter.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\morphinter.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3decifc.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3decoder.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3decoder.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3sscdef.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3streaminfo.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mpegbitstream.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mpegheader.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\raster.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\raster.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sbitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sbitmap.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sdisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sdisplay.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\shape.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sndcodec.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sndcodec.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sndmix.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sndmix.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sobject.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sobject.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\splay.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\splay.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\splayer.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\splayer.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sstroker.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\sstroker.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\stags.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\unixfonts.cpp
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\Win32\version.h
# End Source File
# Begin Source File

SOURCE=.\flashSDK\Source\mp3decoder.lib
# End Source File
# End Group
# End Group
# End Target
# End Project
