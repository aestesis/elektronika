# Microsoft Developer Studio Project File - Name="amplib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=amplib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "amplib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "amplib.mak" CFG="amplib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "amplib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "amplib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "amplib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /Ox /Ot /Oa /Og /Oi /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GF /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "amplib___Win32_Debug"
# PROP BASE Intermediate_Dir "amplib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /nodefaultlib

!ENDIF 

# Begin Target

# Name "amplib - Win32 Release"
# Name "amplib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AMPBitstream.cpp
# End Source File
# Begin Source File

SOURCE=.\AMPDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\hufftbl.cpp

!IF  "$(CFG)" == "amplib - Win32 Release"

# ADD CPP /Ox /Ot /Oa /Og /Oi /Oy

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\imdct.cpp

!IF  "$(CFG)" == "amplib - Win32 Release"

# ADD CPP /GX- /Ox /Ot /Oa /Og /Oi /Oy

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\layer2.cpp

!IF  "$(CFG)" == "amplib - Win32 Release"

# ADD CPP /Ox /Ot /Oa /Og /Oi /Oy

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\layer3.cpp

!IF  "$(CFG)" == "amplib - Win32 Release"

# ADD CPP /Ox /Ot /Oa /Og /Oi /Oy

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\polyphase.cpp

!IF  "$(CFG)" == "amplib - Win32 Release"

# ADD CPP /GX- /Ox /Ot /Oa /Og /Oi /Oy /Ob1 /Gy
# SUBTRACT CPP /Gf

!ELSEIF  "$(CFG)" == "amplib - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\staticinit.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AMPBitstream.h
# End Source File
# Begin Source File

SOURCE=.\AMPDecoder.h
# End Source File
# Begin Source File

SOURCE=.\IAMPDecoder.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
