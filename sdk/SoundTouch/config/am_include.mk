## vim:tw=78
## Process this file with automake to create Makefile.in
##
## $Id: am_include.mk,v 1.1 2003/12/25 11:36:07 Olli Exp $
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

## These are common definitions used in all Makefiles
## It is actually included when a makefile.am is coverted to Makefile.in
## by automake, so it's ok to have @MACROS@ that will be set by configure


## INCLUDES is automatically added to CXXFLAGS at compile time. The
## $(top_srcdir) macro is set by configure. It's important to use $(top_srcdir)
## in case a user decides to build in a separate directory from the base package
## directory. Using absolute, or relative paths is a bad idea.
INCLUDES=-I$(top_srcdir)/include


## CXXFLAGS is also automatically added to the $(CXX) macro at compile time, and
## is passed down to the children as well
## I list these here in case there needs to always be some flags passed to the compiler
CXXFLAGS=@CXXFLAGS@


## LDFLAGS will be added at link time
## note that the -l flags are set by the LIBS macro that is set by configure
## This line doesn't need to be here, configure will add LDFLAGS on it's own,
## but I'm leaving here as a place to add LDFLAGS if so desired
LDFLAGS=@LDFLAGS@


# doc directory
pkgdocdir=$(prefix)/doc/@PACKAGE@


