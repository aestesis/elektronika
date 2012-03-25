# Microsoft Developer Studio Project File - Name="elektronika" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=elektronika - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "elektronika.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "elektronika.mak" CFG="elektronika - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "elektronika - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "elektronika - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "elektronika - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "elektronika___Win32_Release"
# PROP BASE Intermediate_Dir "elektronika___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "elektronika___Win32_Release"
# PROP Intermediate_Dir "elektronika___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "dvcap" /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /I "..\BaseClasses" /I "..\dxsend" /I "..\..\sdk\inc\\" /D "NDEBUG" /D "SPLASH" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D WINVER=0x0500 /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 strmbase.lib winmm.lib elektro.lib alib100.lib kernel32.lib user32.lib gdi32.lib shell32.lib uuid.lib dsound.lib dxguid.lib ole32.lib ddraw.lib ws2_32.lib oleaut32.lib strmiids.lib opengl32.lib glu32.lib advapi32.lib /nologo /subsystem:windows /machine:I386 /out:"../../elektronika/elektronika.exe" /libpath:"..\..\sdk\lib"

!ELSEIF  "$(CFG)" == "elektronika - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "elektronika___Win32_Debug"
# PROP BASE Intermediate_Dir "elektronika___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "elektronika___Win32_Debug"
# PROP Intermediate_Dir "elektronika___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "dvcap" /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /I "..\BaseClasses" /I "..\dxsend" /I "..\..\sdk\inc\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D WINVER=0x0500 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 amstrmid.lib winmm.lib elektro.lib alib100.lib kernel32.lib user32.lib gdi32.lib shell32.lib uuid.lib dsound.lib dxguid.lib ole32.lib ddraw.lib ws2_32.lib oleaut32.lib strmiids.lib opengl32.lib glu32.lib advapi32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../elektronika/elektronika.exe" /pdbtype:sept /libpath:"..\..\sdk\lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "elektronika - Win32 Release"
# Name "elektronika - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\about.cpp
# End Source File
# Begin Source File

SOURCE=.\background.cpp
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\preview.cpp
# End Source File
# Begin Source File

SOURCE=.\status.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\about.h
# End Source File
# Begin Source File

SOURCE=.\background.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\password.h
# End Source File
# Begin Source File

SOURCE=.\preview.h
# End Source File
# Begin Source File

SOURCE=.\status.h
# End Source File
# End Group
# Begin Group "Effects Files"

# PROP Default_Filter ""
# Begin Group "nekoAmp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nekoAmp\AMPBitstream.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\AMPBitstream.h
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\AMPDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\AMPDecoder.h
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\hufftbl.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\IAMPDecoder.h
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\imdct.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\Layer2.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\Layer3.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\polyphase.cpp
# End Source File
# Begin Source File

SOURCE=.\nekoAmp\staticinit.cpp
# End Source File
# End Group
# Begin Group "helpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\sdk\inc\audiovideo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\aestesis.cpp
# End Source File
# Begin Source File

SOURCE=.\aestesis.h
# End Source File
# Begin Source File

SOURCE=.\aplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\aplayer.h
# End Source File
# Begin Source File

SOURCE=.\dispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\dispatch.h
# End Source File
# Begin Source File

SOURCE=.\dvcap.cpp
# End Source File
# Begin Source File

SOURCE=.\dvcap.h
# End Source File
# Begin Source File

SOURCE=.\dxfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\dxfilter.h
# End Source File
# Begin Source File

SOURCE=.\grab.cpp
# End Source File
# Begin Source File

SOURCE=.\grab.h
# End Source File
# Begin Source File

SOURCE=.\loop.cpp
# End Source File
# Begin Source File

SOURCE=.\loop.h
# End Source File
# Begin Source File

SOURCE=.\mimix.cpp
# End Source File
# Begin Source File

SOURCE=.\mimix.h
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\network.h
# End Source File
# Begin Source File

SOURCE=.\rhytmBox.cpp
# End Source File
# Begin Source File

SOURCE=.\rhytmBox.h
# End Source File
# Begin Source File

SOURCE=.\soundInput.cpp
# End Source File
# Begin Source File

SOURCE=.\soundInput.h
# End Source File
# Begin Source File

SOURCE=.\soundOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\soundOutput.h
# End Source File
# Begin Source File

SOURCE=.\sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\sphere.h
# End Source File
# Begin Source File

SOURCE=.\v3dout.cpp
# End Source File
# Begin Source File

SOURCE=.\v3dout.h
# End Source File
# Begin Source File

SOURCE=.\vmix.cpp
# End Source File
# Begin Source File

SOURCE=.\vmix.h
# End Source File
# Begin Source File

SOURCE=.\vout.cpp
# End Source File
# Begin Source File

SOURCE=.\vout.h
# End Source File
# Begin Source File

SOURCE=.\webclient.cpp
# End Source File
# Begin Source File

SOURCE=.\webclient.h
# End Source File
# Begin Source File

SOURCE=.\websrv.cpp
# End Source File
# Begin Source File

SOURCE=.\websrv.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\aest.swf
# End Source File
# Begin Source File

SOURCE=.\_graphs\aestesis.png
# End Source File
# Begin Source File

SOURCE=.\aestesis.png
# End Source File
# Begin Source File

SOURCE=.\aestesis2.png
# End Source File
# Begin Source File

SOURCE=.\aestesis_but_colors.png
# End Source File
# Begin Source File

SOURCE=.\aestesis_but_forms.png
# End Source File
# Begin Source File

SOURCE=.\aestesis_but_sources.png
# End Source File
# Begin Source File

SOURCE=.\bouton.png
# End Source File
# Begin Source File

SOURCE=".\button disk.png"
# End Source File
# Begin Source File

SOURCE=".\button first.png"
# End Source File
# Begin Source File

SOURCE=".\button forward.png"
# End Source File
# Begin Source File

SOURCE=".\button last.png"
# End Source File
# Begin Source File

SOURCE=".\button play.png"
# End Source File
# Begin Source File

SOURCE=".\button properties.png"
# End Source File
# Begin Source File

SOURCE=".\button rec.png"
# End Source File
# Begin Source File

SOURCE=".\button rewind.png"
# End Source File
# Begin Source File

SOURCE=".\button stop.png"
# End Source File
# Begin Source File

SOURCE=.\dispatch.png
# End Source File
# Begin Source File

SOURCE=.\dispatch2.png
# End Source File
# Begin Source File

SOURCE=.\dvcap.png
# End Source File
# Begin Source File

SOURCE=.\dvcap2.png
# End Source File
# Begin Source File

SOURCE=".\elektronika License registred.txt"
# End Source File
# Begin Source File

SOURCE=".\elektronika License.txt"
# End Source File
# Begin Source File

SOURCE=.\eleKtroniKa.png
# End Source File
# Begin Source File

SOURCE=.\elektronika.rc
# End Source File
# Begin Source File

SOURCE=.\elektronika.swf
# End Source File
# Begin Source File

SOURCE=.\grabBack.png
# End Source File
# Begin Source File

SOURCE=.\grabFront.png
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\loop.png
# End Source File
# Begin Source File

SOURCE=.\loop2.png
# End Source File
# Begin Source File

SOURCE=.\loop_shape.png
# End Source File
# Begin Source File

SOURCE=.\mimix.png
# End Source File
# Begin Source File

SOURCE=.\mimix2.png
# End Source File
# Begin Source File

SOURCE=.\_graphs\mireNB220x166.png
# End Source File
# Begin Source File

SOURCE=.\network.png
# End Source File
# Begin Source File

SOURCE=.\network2.png
# End Source File
# Begin Source File

SOURCE=.\network_browse.png
# End Source File
# Begin Source File

SOURCE=.\rhytmBox.png
# End Source File
# Begin Source File

SOURCE=.\rhytmBox2.png
# End Source File
# Begin Source File

SOURCE=.\rhytmBox_loop.png
# End Source File
# Begin Source File

SOURCE=.\rhytmbox_mute.png
# End Source File
# Begin Source File

SOURCE=.\rhytmbox_open.png
# End Source File
# Begin Source File

SOURCE=.\rhytmbox_pattern.png
# End Source File
# Begin Source File

SOURCE=.\rhytmBox_pattern_switch.png
# End Source File
# Begin Source File

SOURCE=.\rhytmBox_select.png
# End Source File
# Begin Source File

SOURCE=.\rhytmbox_solo.png
# End Source File
# Begin Source File

SOURCE=.\rhytmbox_triger.png
# End Source File
# Begin Source File

SOURCE=.\select.png
# End Source File
# Begin Source File

SOURCE=.\slice01.png
# End Source File
# Begin Source File

SOURCE=.\slice02.png
# End Source File
# Begin Source File

SOURCE=.\slice03.png
# End Source File
# Begin Source File

SOURCE=.\slice04.png
# End Source File
# Begin Source File

SOURCE=.\slice05.png
# End Source File
# Begin Source File

SOURCE=.\slice06.png
# End Source File
# Begin Source File

SOURCE=.\slice07.png
# End Source File
# Begin Source File

SOURCE=.\slice08.png
# End Source File
# Begin Source File

SOURCE=.\slice09.png
# End Source File
# Begin Source File

SOURCE=.\slice10.png
# End Source File
# Begin Source File

SOURCE=.\slice11.png
# End Source File
# Begin Source File

SOURCE=.\slice12.png
# End Source File
# Begin Source File

SOURCE=.\slice13.png
# End Source File
# Begin Source File

SOURCE=.\slice14.png
# End Source File
# Begin Source File

SOURCE=.\slice15.png
# End Source File
# Begin Source File

SOURCE=.\slice16.png
# End Source File
# Begin Source File

SOURCE=.\soundInput.png
# End Source File
# Begin Source File

SOURCE=.\soundInput2.png
# End Source File
# Begin Source File

SOURCE=.\soundOutput.png
# End Source File
# Begin Source File

SOURCE=.\soundOutput2.png
# End Source File
# Begin Source File

SOURCE=".\sphere cross.png"
# End Source File
# Begin Source File

SOURCE=".\sphere crossdiag.png"
# End Source File
# Begin Source File

SOURCE=".\sphere horizontal.png"
# End Source File
# Begin Source File

SOURCE=".\sphere vertical.png"
# End Source File
# Begin Source File

SOURCE=.\sphere.png
# End Source File
# Begin Source File

SOURCE=.\sphere2.png
# End Source File
# Begin Source File

SOURCE=.\test.png
# End Source File
# Begin Source File

SOURCE=.\texture001.png
# End Source File
# Begin Source File

SOURCE=.\texture002.png
# End Source File
# Begin Source File

SOURCE=.\texture003.png
# End Source File
# Begin Source File

SOURCE=.\texture004.png
# End Source File
# Begin Source File

SOURCE=.\texture005.png
# End Source File
# Begin Source File

SOURCE=.\texture006.png
# End Source File
# Begin Source File

SOURCE=.\texture007.png
# End Source File
# Begin Source File

SOURCE=.\transslice_fade.png
# End Source File
# Begin Source File

SOURCE=.\transslice_inv.png
# End Source File
# Begin Source File

SOURCE=.\videoOut.png
# End Source File
# Begin Source File

SOURCE=.\videoOut2.png
# End Source File
# Begin Source File

SOURCE=.\vmix.png
# End Source File
# Begin Source File

SOURCE=.\vmix2.png
# End Source File
# Begin Source File

SOURCE=.\vmix_sources.png
# End Source File
# Begin Source File

SOURCE=.\vmix_trans.png
# End Source File
# Begin Source File

SOURCE=.\webclient.png
# End Source File
# Begin Source File

SOURCE=.\webclient2.png
# End Source File
# Begin Source File

SOURCE=.\websrvBack.png
# End Source File
# Begin Source File

SOURCE=.\websrvFront.png
# End Source File
# End Group
# Begin Group "transitions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\trans-color.cpp"
# End Source File
# Begin Source File

SOURCE=".\trans-color.h"
# End Source File
# Begin Source File

SOURCE=".\trans-fade.cpp"
# End Source File
# Begin Source File

SOURCE=".\trans-fade.h"
# End Source File
# Begin Source File

SOURCE=".\trans-slice.cpp"
# End Source File
# Begin Source File

SOURCE=".\trans-slice.h"
# End Source File
# End Group
# End Target
# End Project
