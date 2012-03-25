# Microsoft Developer Studio Generated NMAKE File, Based on IscMetaBalls1.dsp
!IF "$(CFG)" == ""
CFG=FreeFrame - Win32 Debug
!MESSAGE No configuration specified. Defaulting to FreeFrame - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FreeFrame - Win32 Release" && "$(CFG)" != "FreeFrame - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IscMetaBalls1.mak" CFG="FreeFrame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FreeFrame - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FreeFrame - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FreeFrame - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : ".\plugins\IscMetaBalls1.dll" "$(OUTDIR)\IscMetaBalls1.bsc"


CLEAN :
	-@erase "$(INTDIR)\FreeFrame.obj"
	-@erase "$(INTDIR)\FreeFrame.sbr"
	-@erase "$(INTDIR)\graphics.obj"
	-@erase "$(INTDIR)\graphics.sbr"
	-@erase "$(INTDIR)\IscMetaBalls1.obj"
	-@erase "$(INTDIR)\IscMetaBalls1.sbr"
	-@erase "$(INTDIR)\MarchingCubes.obj"
	-@erase "$(INTDIR)\MarchingCubes.sbr"
	-@erase "$(INTDIR)\Metaballs.obj"
	-@erase "$(INTDIR)\Metaballs.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\window.sbr"
	-@erase "$(OUTDIR)\IscMetaBalls1.bsc"
	-@erase "$(OUTDIR)\IscMetaBalls1.exp"
	-@erase "$(OUTDIR)\IscMetaBalls1.lib"
	-@erase ".\plugins\IscMetaBalls1.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEFRAME_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IscMetaBalls1.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\FreeFrame.sbr" \
	"$(INTDIR)\graphics.sbr" \
	"$(INTDIR)\IscMetaBalls1.sbr" \
	"$(INTDIR)\MarchingCubes.sbr" \
	"$(INTDIR)\Metaballs.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\window.sbr"

"$(OUTDIR)\IscMetaBalls1.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winmm.lib d3d8.lib d3dx8.lib Advapi32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\IscMetaBalls1.pdb" /machine:I386 /def:".\IscMetaBalls1.def" /out:"plugins/IscMetaBalls1.dll" /implib:"$(OUTDIR)\IscMetaBalls1.lib" 
DEF_FILE= \
	".\IscMetaBalls1.def"
LINK32_OBJS= \
	"$(INTDIR)\FreeFrame.obj" \
	"$(INTDIR)\graphics.obj" \
	"$(INTDIR)\IscMetaBalls1.obj" \
	"$(INTDIR)\MarchingCubes.obj" \
	"$(INTDIR)\Metaballs.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\window.obj"

".\plugins\IscMetaBalls1.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FreeFrame - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : ".\plugins\IscMetaBalls1Debug.dll" "$(OUTDIR)\IscMetaBalls1.bsc"


CLEAN :
	-@erase "$(INTDIR)\FreeFrame.obj"
	-@erase "$(INTDIR)\FreeFrame.sbr"
	-@erase "$(INTDIR)\graphics.obj"
	-@erase "$(INTDIR)\graphics.sbr"
	-@erase "$(INTDIR)\IscMetaBalls1.obj"
	-@erase "$(INTDIR)\IscMetaBalls1.sbr"
	-@erase "$(INTDIR)\MarchingCubes.obj"
	-@erase "$(INTDIR)\MarchingCubes.sbr"
	-@erase "$(INTDIR)\Metaballs.obj"
	-@erase "$(INTDIR)\Metaballs.sbr"
	-@erase "$(INTDIR)\timer.obj"
	-@erase "$(INTDIR)\timer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(INTDIR)\window.sbr"
	-@erase "$(OUTDIR)\IscMetaBalls1.bsc"
	-@erase "$(OUTDIR)\IscMetaBalls1Debug.exp"
	-@erase "$(OUTDIR)\IscMetaBalls1Debug.lib"
	-@erase "$(OUTDIR)\IscMetaBalls1Debug.pdb"
	-@erase ".\plugins\IscMetaBalls1Debug.dll"
	-@erase ".\plugins\IscMetaBalls1Debug.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEFRAME_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IscMetaBalls1.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\FreeFrame.sbr" \
	"$(INTDIR)\graphics.sbr" \
	"$(INTDIR)\IscMetaBalls1.sbr" \
	"$(INTDIR)\MarchingCubes.sbr" \
	"$(INTDIR)\Metaballs.sbr" \
	"$(INTDIR)\timer.sbr" \
	"$(INTDIR)\window.sbr"

"$(OUTDIR)\IscMetaBalls1.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\IscMetaBalls1Debug.pdb" /debug /machine:I386 /def:".\IscMetaBalls1.def" /out:"plugins/IscMetaBalls1Debug.dll" /implib:"$(OUTDIR)\IscMetaBalls1Debug.lib" /pdbtype:sept 
DEF_FILE= \
	".\IscMetaBalls1.def"
LINK32_OBJS= \
	"$(INTDIR)\FreeFrame.obj" \
	"$(INTDIR)\graphics.obj" \
	"$(INTDIR)\IscMetaBalls1.obj" \
	"$(INTDIR)\MarchingCubes.obj" \
	"$(INTDIR)\Metaballs.obj" \
	"$(INTDIR)\timer.obj" \
	"$(INTDIR)\window.obj"

".\plugins\IscMetaBalls1Debug.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("IscMetaBalls1.dep")
!INCLUDE "IscMetaBalls1.dep"
!ELSE 
!MESSAGE Warning: cannot find "IscMetaBalls1.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FreeFrame - Win32 Release" || "$(CFG)" == "FreeFrame - Win32 Debug"
SOURCE=.\FreeFrame.cpp

"$(INTDIR)\FreeFrame.obj"	"$(INTDIR)\FreeFrame.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\graphics.cpp

"$(INTDIR)\graphics.obj"	"$(INTDIR)\graphics.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\IscMetaBalls1.cpp

"$(INTDIR)\IscMetaBalls1.obj"	"$(INTDIR)\IscMetaBalls1.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MarchingCubes.cpp

"$(INTDIR)\MarchingCubes.obj"	"$(INTDIR)\MarchingCubes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Metaballs.cpp

"$(INTDIR)\Metaballs.obj"	"$(INTDIR)\Metaballs.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\timer.cpp

"$(INTDIR)\timer.obj"	"$(INTDIR)\timer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\window.cpp

"$(INTDIR)\window.obj"	"$(INTDIR)\window.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

