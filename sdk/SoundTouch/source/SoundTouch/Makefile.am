## Process this file with automake to create Makefile.in
##
## $Id: Makefile.am,v 1.1 2003/12/25 11:36:08 Olli Exp $
##
## Copyright (C) 2003 - David W. Durham
## 
## This file is part of SoundTouch, an audio processing library for pitch/time adjustments
## 
## SoundTouch is free software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License as published by the Free Software
## Foundation; either version 2 of the License, or (at your option) any later
## version.
## 
## SoundTouch is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
## A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the Free Software Foundation, Inc., 59 Temple
## Place - Suite 330, Boston, MA  02111-1307, USA


include $(top_srcdir)/config/am_include.mk


# set to something if you want other stuff to be included in the distribution tarball
EXTRA_DIST=3dnow_win.cpp cpu_detect_x86_win.cpp mmx_win.cpp SoundTouch.dsp SoundTouch.dsw sse_win.cpp

noinst_HEADERS=AAFilter.h cpu_detect.h cpu_detect_x86_gcc.cpp FIRFilter.h RateTransposer.h TDStretch.h

lib_LTLIBRARIES=libSoundTouch.la
# the mmx_gcc.cpp and cpu_detect_x86_gcc.cpp may need to be conditionally included here from things discovered in configure.ac
libSoundTouch_la_SOURCES=AAFilter.cpp FIRFilter.cpp FIFOSampleBuffer.cpp mmx_gcc.cpp RateTransposer.cpp SoundTouch.cpp TDStretch.cpp cpu_detect_x86_gcc.cpp


# ??? test for -fcheck-new in configure.ac
# other compiler flags to add
AM_CXXFLAGS=-O3 -fcheck-new -I../../include

# other linking flags to add
#libSoundTouch_la_LIBADD=

