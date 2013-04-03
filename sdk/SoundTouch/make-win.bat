@REM 
@REM SoundTouch & SoundStretch Build script for Win32 platform
@REM 
@REM You'll need Visual C++ 6.0 installed to compile - also execute the 
@REM "vcvars32.bat" in VC install directotry before running this one.
@REM 
@REM Copyright (c) Olli Parviainen
@REM File Created: 09/Sep/2003
@REM
@REM $Id: make-win.bat,v 1.5 2003/12/25 11:44:43 Olli Exp $ 

md bin
md lib
msdev source\SoundTouch\SoundTouch.dsw /MAKE ALL
msdev source\example\bpm\bpm.dsw /MAKE ALL
msdev source\example\SoundStretch\SoundStretch.dsw /MAKE "SoundStretch - Win32 Release"
