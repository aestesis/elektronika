# Microsoft Developer Studio Project File - Name="dancer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dancer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dancer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dancer.mak" CFG="dancer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dancer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dancer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dancer - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DANCER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DANCER_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alib100.lib elektro.lib opengl32.lib glu32.lib /nologo /dll /machine:I386 /out:"../../elektronika/plugins/dancer.dll" /libpath:"..\..\sdk\lib"

!ELSEIF  "$(CFG)" == "dancer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dancer___Win32_Debug"
# PROP BASE Intermediate_Dir "dancer___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "dancer___Win32_Debug"
# PROP Intermediate_Dir "dancer___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DANCER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdk\inc\alib" /I "..\..\sdk\inc\elektro" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DANCER_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alib100.lib elektro.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"../../elektronika/plugins/dancer.dll" /pdbtype:sept /libpath:"..\..\sdk\lib"

!ENDIF 

# Begin Target

# Name "dancer - Win32 Release"
# Name "dancer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\animat.cpp
# End Source File
# Begin Source File

SOURCE=.\source\main.cpp
# End Source File
# Begin Source File

SOURCE=.\source\matrice.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\animat.h
# End Source File
# Begin Source File

SOURCE=.\source\main.h
# End Source File
# Begin Source File

SOURCE=.\source\matrice.h
# End Source File
# Begin Source File

SOURCE=.\source\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\source\back.png
# End Source File
# Begin Source File

SOURCE=.\source\BEN.png
# End Source File
# Begin Source File

SOURCE=.\source\BEN_JAMBES.KPR
# End Source File
# Begin Source File

SOURCE=.\source\BEN_TETE.KPR
# End Source File
# Begin Source File

SOURCE=.\source\BEN_TRONC.KPR
# End Source File
# Begin Source File

SOURCE=.\source\button.png
# End Source File
# Begin Source File

SOURCE=.\source\dancer.rc
# End Source File
# Begin Source File

SOURCE=.\source\front.png
# End Source File
# Begin Source File

SOURCE=.\source\GRADE.png
# End Source File
# Begin Source File

SOURCE=.\source\GRADE_JAMBES.KPR
# End Source File
# Begin Source File

SOURCE=.\source\GRADE_TETE.KPR
# End Source File
# Begin Source File

SOURCE=.\source\GRADE_TRONC.KPR
# End Source File
# Begin Source File

SOURCE=.\source\HELMUT.png
# End Source File
# Begin Source File

SOURCE=.\source\HELMUT_JAMBES.KPR
# End Source File
# Begin Source File

SOURCE=.\source\HELMUT_TETE.KPR
# End Source File
# Begin Source File

SOURCE=.\source\HELMUT_TRONC.KPR
# End Source File
# Begin Source File

SOURCE=.\source\Jack_jambes.kpr
# End Source File
# Begin Source File

SOURCE=.\source\Jack_tete.kpr
# End Source File
# Begin Source File

SOURCE=.\source\Jack_tronc.kpr
# End Source File
# Begin Source File

SOURCE=.\source\NEO_JACK.png
# End Source File
# Begin Source File

SOURCE=.\source\select.png
# End Source File
# Begin Source File

SOURCE=.\source\sources.png
# End Source File
# End Group
# End Target
# End Project
