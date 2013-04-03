# Microsoft Developer Studio Project File - Name="Snake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Snake - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Snake.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Snake.mak" CFG="Snake - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Snake - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Snake - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Snake - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /MD /W3 /GX /O2 /I "./BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "INC_OLE2" /D "STRICT" /D _X86_=1 /D "_USRDLL" /D "_MT" /FD /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /I "./BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "INC_OLE2" /D "STRICT" /D _X86_=1 /D "_USRDLL" /D "_MT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "./BaseClasses" /d "NDEBUG"
# ADD RSC /l 0x409 /i "./BaseClasses" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib     winmm.lib  strmbase.lib /nologo /stack:0x200000,0x200000 /entry:"DllEntryPoint@12" /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/Snake.ax" /libpath:"./Release" /OPT:NOREF
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib     winmm.lib  strmbase.lib /nologo /stack:0x200000,0x200000 /entry:"DllEntryPoint@12" /subsystem:windows /dll /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Release/Snake.ax" /libpath:"./Release" /OPT:NOREF
# Begin Custom Build - Registering DirectShow Filter...
OutDir=.\Release
TargetPath=.\Release\Snake.ax
TargetName=Snake
InputPath=.\Release\Snake.ax
SOURCE="$(InputPath)"

"$(OutDir)\$(TargetName).trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\$(TargetName).trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Snake - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /MDd /W3 /Gm /GX /ZI /Od /I "./BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "INC_OLE2" /D "STRICT" /D _X86_=1 /D "_USRDLL" /D "_MT" /D "DEBUG" /FD /GZ /c
# ADD CPP /nologo /Gz /MDd /W3 /Gm /GX /ZI /Od /I "./BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "INC_OLE2" /D "STRICT" /D _X86_=1 /D "_USRDLL" /D "_MT" /D "DEBUG" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "./BaseClasses" /d "_DEBUG"
# ADD RSC /l 0x409 /i "./BaseClasses" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib     winmm.lib  strmbasd.lib /nologo /stack:0x200000,0x200000 /entry:"DllEntryPoint@12" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Debug/Snake.ax" /pdbtype:sept /libpath:"./Debug"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib     winmm.lib  strmbasd.lib /nologo /stack:0x200000,0x200000 /entry:"DllEntryPoint@12" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"Debug/Snake.ax" /pdbtype:sept /libpath:"./Debug"
# Begin Custom Build - Registering DirectShow Filter...
OutDir=.\Debug
TargetPath=.\Debug\Snake.ax
TargetName=Snake
InputPath=.\Debug\Snake.ax
SOURCE="$(InputPath)"

"$(OutDir)\$(TargetName).trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\$(TargetName).trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Snake - Win32 Release"
# Name "Snake - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Auxiliary.cpp
# End Source File
# Begin Source File

SOURCE=.\fSnake.cpp
# End Source File
# Begin Source File

SOURCE=.\fSnake_op.cpp
# End Source File
# Begin Source File

SOURCE=.\Snake.cpp
# End Source File
# Begin Source File

SOURCE=.\Snake.def
# End Source File
# Begin Source File

SOURCE=.\Snake.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Auxiliary.h
# End Source File
# Begin Source File

SOURCE=.\fSnake.h
# End Source File
# Begin Source File

SOURCE=.\fSnake_op.h
# End Source File
# Begin Source File

SOURCE=.\Snake.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
