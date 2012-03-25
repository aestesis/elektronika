# Microsoft Developer Studio Project File - Name="eleKtro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=eleKtro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "eleKtro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eleKtro.mak" CFG="eleKtro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eleKtro - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "eleKtro - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eleKtro - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ELEKTRO_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /I "..\alib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ELEKTRO_EXPORTS" /D WINVER=0x0500 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 alib100.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /machine:I386 /out:"../../elektronika/eleKtro.dll" /libpath:"..\..\sdk\lib"
# SUBTRACT LINK32 /debug
# Begin Custom Build
InputPath=\_code\YoY\elektroSDK\elektronika\eleKtro.dll
SOURCE="$(InputPath)"

"..\..\sdk\lib\eleKtro.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy release\eleKtro.lib ..\..\sdk\lib\eleKtro.lib

# End Custom Build

!ELSEIF  "$(CFG)" == "eleKtro - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ELEKTRO_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /I "..\alib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ELEKTRO_EXPORTS" /D WINVER=0x0500 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 alib100.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /debug /machine:I386 /out:"../../elektronika/eleKtro.dll" /pdbtype:sept /libpath:"..\..\sdk\lib"
# Begin Custom Build
InputPath=\_code\YoY\elektroSDK\elektronika\eleKtro.dll
SOURCE="$(InputPath)"

"..\..\sdk\lib\eleKtro.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy  debug\elektro.lib ..\..\sdk\lib\eleKtro.lib

# End Custom Build

!ENDIF 

# Begin Target

# Name "eleKtro - Win32 Release"
# Name "eleKtro - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\acontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\connect.cpp
# End Source File
# Begin Source File

SOURCE=.\ctrlButton.cpp
# End Source File
# Begin Source File

SOURCE=.\effect.cpp
# End Source File
# Begin Source File

SOURCE=.\equalizer.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\paddle.cpp
# End Source File
# Begin Source File

SOURCE=.\pattern.cpp
# End Source File
# Begin Source File

SOURCE=.\pin.cpp
# End Source File
# Begin Source File

SOURCE=.\sample.cpp
# End Source File
# Begin Source File

SOURCE=.\selcolor.cpp
# End Source File
# Begin Source File

SOURCE=.\select.cpp
# End Source File
# Begin Source File

SOURCE=.\simple.cpp
# End Source File
# Begin Source File

SOURCE=.\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\table.cpp
# End Source File
# Begin Source File

SOURCE=.\transition.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\video.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\acontrol.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\connect.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\ctrlButton.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\effect.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\elektro.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\elektrodef.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\elektroexp.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\equalizer.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\paddle.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\pattern.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\pin.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\sample.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\selcolor.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\select.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\simple.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\slider.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\table.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\transition.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\trigger.h
# End Source File
# Begin Source File

SOURCE=..\..\sdk\inc\elektro\video.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\button.png
# End Source File
# Begin Source File

SOURCE=.\elektro.rc
# End Source File
# Begin Source File

SOURCE=.\paddle010.png
# End Source File
# Begin Source File

SOURCE=.\paddle011.png
# End Source File
# Begin Source File

SOURCE=.\paddle012.png
# End Source File
# Begin Source File

SOURCE=.\paddle013.png
# End Source File
# Begin Source File

SOURCE=.\triger.png
# End Source File
# End Group
# End Target
# End Project
