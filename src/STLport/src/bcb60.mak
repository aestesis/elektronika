# ---------------------------------------------------------------------------

!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

CPP=cpp32
CC=bcc32 -P
CXX=bcc32

RC=brc32
COMP=BCB

LIB_BASENAME=stlp

# ---------------------------------------------------------------------------

LINK_OUT=
DYNLINK_OUT=
LINK = tlib /P256
# Use objs in RTL build tree
DYN_LINK = ilink32 -L$(BCB)\lib
OBJEXT=obj
DYNEXT=dll
STEXT=lib
# We use 4NT, which does not know about /Q
#RM=-rd /S /Q
RM=-rd /S
PATH_SEP=\\
MKDIR=-md
STATIC_SUFFIX=st
INSTALL_STEP=install_bc

all : platform all_dynamic all_static

!include common_macros_windows.mak

#RM=-rd /S /Q
RM=-rd /S
MKDIR=-md

DYNAMIC_DEFS=_RTLDLL;_WIN32;_DLL;_STLP_DESIGNATED_DLL
STATIC_DEFS=_LIB;_WIN32;_STLP_NO_FORCE_INSTANTIATE

LINKSTARTUP= c0d32.obj

FLAGS_COMMON= -I.;..\stlport;$(BCB)\include;$(BCB)\include\windows -jb -j1 -w -c -w-par -w-inl -w-stl -D_STLP_DONT_REDEFINE_STD

FLAGS_COMMON_static = $(FLAGS_COMMON) -D$(SYSDEFINES);$(USERDEFINES);$(STATIC_DEFS)
FLAGS_COMMON_dynamic = $(FLAGS_COMMON) -D$(SYSDEFINES);$(USERDEFINES);$(DYNAMIC_DEFS) -tWDR

# -L$(MAKEDIR)\..\lib  removed  ...it should get ilink32.cfg values instead
LDFLAGS_COMMON=-Gn -Gi -Tpd -aa -x 
LDFLAGS_COMMON_static=  $(LDFLAGS_COMMON)
LDFLAGS_COMMON_dynamic= $(LDFLAGS_COMMON)


LDFLAGS_RELEASE_static=   $(LDFLAGS_COMMON_static)  $(LINKSTARTUP)
LDFLAGS_DEBUG_static=     $(LDFLAGS_COMMON_static)  $(LINKSTARTUP)
LDFLAGS_STLDEBUG_static=  $(LDFLAGS_COMMON_static)  $(LINKSTARTUP)

LDFLAGS_RELEASE_dynamic=   $(LDFLAGS_COMMON_dynamic) $(LINKSTARTUP)
LDFLAGS_DEBUG_dynamic=     $(LDFLAGS_COMMON_dynamic) $(LINKSTARTUP)
LDFLAGS_STLDEBUG_dynamic=  $(LDFLAGS_COMMON_dynamic) $(LINKSTARTUP)

LDLIBS_RELEASE_dynamic=   import32.lib cw32i.lib
LDLIBS_DEBUG_dynamic=     import32.lib cw32i.lib
LDLIBS_STLDEBUG_dynamic=  import32.lib cw32i.lib


CXXFLAGS_RELEASE_static= $(FLAGS_COMMON_static) -O2 -n$(RELEASE_OBJDIR_static)
CXXFLAGS_DEBUG_static= $(FLAGS_COMMON_static) -v -N -x -xp -n$(DEBUG_OBJDIR_static)
CXXFLAGS_STLDEBUG_static= $(FLAGS_COMMON_static) -v -N -x -xp -n$(STLDEBUG_OBJDIR_static) -D_STLP_DEBUG

CXXFLAGS_RELEASE_dynamic= $(FLAGS_COMMON_dynamic) -O2 -n$(RELEASE_OBJDIR_dynamic)
CXXFLAGS_DEBUG_dynamic= $(FLAGS_COMMON_dynamic)  -v -N -x -xp -n$(DEBUG_OBJDIR_dynamic)
CXXFLAGS_STLDEBUG_dynamic= $(FLAGS_COMMON_dynamic)  -v -N -x -xp -n$(STLDEBUG_OBJDIR_dynamic) -D_STLP_DEBUG

RELEASE_LINK_COMMANDS_static=\
+$(RELEASE_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
+$(RELEASE_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)


DEBUG_LINK_COMMANDS_static= \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
+$(DEBUG_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)

STLDEBUG_LINK_COMMANDS_static= \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
+$(STLDEBUG_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)

#
#  Target directories
#
#

$(RELEASE_OBJDIR_dynamic) :
	$(MKDIR) $(RELEASE_OBJDIR_dynamic)
$(DEBUG_OBJDIR_dynamic) :
	$(MKDIR) $(DEBUG_OBJDIR_dynamic)
$(STLDEBUG_OBJDIR_dynamic) :
	$(MKDIR) $(STLDEBUG_OBJDIR_dynamic)
$(RELEASE_OBJDIR_static) :
	$(MKDIR) $(RELEASE_OBJDIR_static)
$(DEBUG_OBJDIR_static) :
	$(MKDIR) $(DEBUG_OBJDIR_static)
$(STLDEBUG_OBJDIR_static) :
	$(MKDIR) $(STLDEBUG_OBJDIR_static)

######   Targets ##################

all_static :   $(ALL_STATIC_LIBS)

all_dynamic :  $(ALL_DYNAMIC_LIBS)

install :  $(INSTALL_STEP)

clean : $(CLEAN_SPECIFIC)
	-$(RM) $(RELEASE_OBJDIR_static) $(DEBUG_OBJDIR_static) \
        $(STLDEBUG_OBJDIR_static) $(RELEASE_OBJDIR_dynamic) \
        $(DEBUG_OBJDIR_dynamic) $(STLDEBUG_OBJDIR_dynamic)

clobber : clean
	-$(RM) $(OUTDIR)$(PATH_SEP)$(DEBUG_NAME).* \
               $(OUTDIR)$(PATH_SEP)$(RELEASE_NAME).* $(OUTDIR)$(PATH_SEP)$(STLDEBUG_NAME).*

#create a compiler platform directory
platform:
	-$(MKDIR) $(OBJDIR_COMMON)
	-$(MKDIR) $(OBJDIR)

$(OUTDIR)$(PATH_SEP)$(RELEASE_DYNLIB) : $(RELEASE_OBJDIR_dynamic) $(DEF_FILE) $(RELEASE_OBJECTS_dynamic)
	$(DYN_LINK) $(LDFLAGS_RELEASE_dynamic) $(RELEASE_OBJECTS_dynamic) , $(OUTDIR)$(PATH_SEP)$(RELEASE_DYNLIB),,$(LDLIBS_RELEASE_dynamic)

$(OUTDIR)$(PATH_SEP)$(DEBUG_DYNLIB) : $(DEBUG_OBJDIR_dynamic) $(DEF_FILE) $(DEBUG_OBJECTS_dynamic)
	$(DYN_LINK) $(LDFLAGS_DEBUG_dynamic) $(DEBUG_OBJECTS_dynamic), $(DYNLINK_OUT)$(OUTDIR)$(PATH_SEP)$(DEBUG_DYNLIB) ,, $(LDLIBS_DEBUG_dynamic) 

$(OUTDIR)$(PATH_SEP)$(STLDEBUG_DYNLIB) : $(STLDEBUG_OBJDIR_dynamic) $(DEF_FILE) $(STLDEBUG_OBJECTS_dynamic)
	$(DYN_LINK)  $(LDFLAGS_STLDEBUG_dynamic) $(STLDEBUG_OBJECTS_dynamic) , $(DYNLINK_OUT)$(OUTDIR)$(PATH_SEP)$(STLDEBUG_DYNLIB) ,, $(LDLIBS_STLDEBUG_dynamic)

$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) : $(RELEASE_OBJDIR_static) $(DEF_FILE) $(RELEASE_OBJECTS_static)
	$(LINK) $(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) $(RELEASE_LINK_COMMANDS_static)

$(OUTDIR)$(PATH_SEP)$(DEBUG_LIB) : $(DEBUG_OBJDIR_static) $(DEF_FILE) $(DEBUG_OBJECTS_static)
	$(LINK) $(OUTDIR)$(PATH_SEP)$(DEBUG_LIB) $(DEBUG_LINK_COMMANDS_static)  

$(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB) : $(STLDEBUG_OBJDIR_static) $(DEF_FILE) $(STLDEBUG_OBJECTS_static)
	$(LINK) $(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB) $(STLDEBUG_LINK_COMMANDS_static)

#########################################

# ---------------------------------------------------------------------------

!include nmake_common.mak

.cpp.i:
    $(CPP) $(FLAGS_COMMON_static) -n. -Sr -Ss -Sd {$< }

# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
