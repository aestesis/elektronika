/*
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */ 
# include "stlport_prefix.h"

#include <istream>
// #include <stl/_istream.h>

#include <stl/_fstream.h>
#include <stdio_streambuf>
#include "aligned_buffer.h"

// boris : note this is repeated in <iostream>
#ifndef _STLP_USE_NAMESPACES
// in case of SGI iostreams, we have to rename our streams not to clash with those
// provided in native lib
# define cin _STLP_cin
# define cout _STLP_cout
# define cerr _STLP_cerr
# define clog _STLP_clog
#endif

_STLP_BEGIN_NAMESPACE

#if defined (__BORLANDC__) && ! defined (_STLP_USE_GLIBC)
using _STLP_VENDOR_CSTD::_streams;
#endif

// This file handles iostream initialization.  It is inherently
// nonportable, since the C++ language definition provides no mechanism
// for controlling order of initialization of nonlocal objects.  
// Initialization has three parts, which must be performed in the following
// order:
//  (1) Initialize the locale system
//  (2) Call the constructors for the eight global stream objects.
//  (3) Create streambufs for the global stream objects, and initialize
//      the stream objects by calling the init() member function.


#if defined (_STLP_MSVC) || defined(__MWERKS__) || defined (__ICL) || defined (__ISCPP__)

// Definitions of the eight global I/O objects that are declared in 
// <iostream>. For VC++ we use the init_seg pragma to put the global I/O
// objects into an intitialization segement that will not
// be executed. We then explicitly invoke the constructors
// with placement new in ios_base::_S_initialize() 

#if defined(__MWERKS__)
# pragma suppress_init_code on
#else
# pragma init_seg("STLPORT_NO_INIT")
#endif

_STLP_DECLSPEC istream cin(0);
_STLP_DECLSPEC ostream cout(0);
_STLP_DECLSPEC ostream cerr(0);
_STLP_DECLSPEC ostream clog(0);

_STLP_DECLSPEC wistream wcin(0);
_STLP_DECLSPEC wostream wcout(0);
_STLP_DECLSPEC wostream wcerr(0);
_STLP_DECLSPEC wostream wclog(0);

#if defined(__MWERKS__)
# pragma suppress_init_code off
#endif

#else


// Definitions of the eight global I/O objects that are declared in 
// <iostream>.  Disgusting hack: we deliberately define them with the
// wrong types so that the constructors don't get run automatically.
// We need special tricks to make sure that these objects are struct-
// aligned rather than byte-aligned.

// This is not portable.  Declaring a variable with different types in
// two translations units is "undefined", according to the C++ standard.
// Most compilers, however, silently accept this instead of diagnosing
// it as an error.

_STLP_DECLSPEC _Stl_aligned_buffer<istream> cin;
_STLP_DECLSPEC _Stl_aligned_buffer<ostream> cout;
_STLP_DECLSPEC _Stl_aligned_buffer<ostream> cerr;
_STLP_DECLSPEC _Stl_aligned_buffer<ostream> clog;

# ifndef _STLP_NO_WCHAR_T

_STLP_DECLSPEC _Stl_aligned_buffer<wistream> wcin;
_STLP_DECLSPEC _Stl_aligned_buffer<wostream> wcout;
_STLP_DECLSPEC _Stl_aligned_buffer<wostream> wcerr;
_STLP_DECLSPEC _Stl_aligned_buffer<wostream> wclog;

# endif

#endif /* STL_MSVC || __MWERKS__ */

// Member functions from class ios_base and ios_base::Init

long ios_base::Init::_S_count = 0;
// by default, those are synced
bool ios_base::_S_was_synced = true;

ios_base::Init::Init() {
    if (_S_count++ == 0)
      ios_base::_S_initialize();
}

ios_base::Init::~Init() {
    if (--_S_count == 0)
      ios_base::_S_uninitialize();
}


filebuf*
_Stl_create_filebuf(FILE* f, ios_base::openmode mode )
{
  basic_filebuf<char, char_traits<char> >* result;
  
  result = new basic_filebuf<char, char_traits<char> >();

  _STLP_TRY {
    result->_M_open(_FILE_fd(f), mode);
  }
  _STLP_CATCH_ALL {}

  if (!result->is_open()) {
    delete result;
    result = 0;
  }
  return result;
}

# ifndef _STLP_NO_WCHAR_T

wfilebuf*
_Stl_create_wfilebuf(FILE* f, ios_base::openmode mode )
{
  basic_filebuf<wchar_t, char_traits<wchar_t> >* result;
  
  result = new basic_filebuf<wchar_t, char_traits<wchar_t> >();

  _STLP_TRY {
    result->_M_open(_FILE_fd(f), mode);
  }
  _STLP_CATCH_ALL {}

  if (!result->is_open()) {
    delete result;
    result = 0;
  }
  return result;
}

# endif

void  _STLP_CALL ios_base::_S_initialize()
{
# if !defined(_STLP_HAS_NO_NAMESPACES) && !defined(_STLP_WINCE)
  using _SgI::stdio_istreambuf;
  using _SgI::stdio_ostreambuf;
# endif
  _STLP_TRY {
    // Run constructors for the four narrow stream objects.
    // check with locale system
    if (_Loc_init::_S_count++ == 0) {
      locale::_S_initialize();
    }
#if !defined(_STLP_WINCE)
    istream* ptr_cin  = new((void*)&cin)  istream(0);
    ostream* ptr_cout = new((void*)&cout) ostream(0);
    ostream* ptr_cerr = new((void*)&cerr) ostream(0);
    ostream* ptr_clog = new((void*)&clog) ostream(0);

    // Initialize the four narrow stream objects.
    if (_S_was_synced) {
      ptr_cin->init(new stdio_istreambuf(stdin));
      ptr_cout->init(new stdio_ostreambuf(stdout));
      ptr_cerr->init(new stdio_ostreambuf(stderr));
      ptr_clog->init(new stdio_ostreambuf(stderr));
    } else {
      ptr_cin->init(_Stl_create_filebuf(stdin, ios_base::in));
      ptr_cin->init(_Stl_create_filebuf(stdout, ios_base::out));
      ptr_cin->init(_Stl_create_filebuf(stderr, ios_base::out));
      ptr_cin->init(_Stl_create_filebuf(stderr, ios_base::out)); 
    }
    ptr_cin->tie(ptr_cout);
    ptr_cerr->setf(ios_base::unitbuf);

# ifndef _STLP_NO_WCHAR_T

    // Run constructors for the four wide stream objects.
    wistream* ptr_wcin  = new(&wcin)  wistream(0);
    wostream* ptr_wcout = new(&wcout) wostream(0);
    wostream* ptr_wcerr = new(&wcerr) wostream(0);
    wostream* ptr_wclog = new(&wclog) wostream(0);
    
    wfilebuf* win  = _Stl_create_wfilebuf(stdin, ios_base::in);
    wfilebuf* wout = _Stl_create_wfilebuf(stdout, ios_base::out);;
    wfilebuf* werr = _Stl_create_wfilebuf(stderr, ios_base::out);
    wfilebuf* wlog = _Stl_create_wfilebuf(stderr, ios_base::out);
    
    ptr_wcin->init(win);
    ptr_wcout->init(wout);
    ptr_wcerr->init(werr);
    ptr_wclog->init(wlog);

    ptr_wcin->tie(ptr_wcout);
    ptr_wcerr->setf(ios_base::unitbuf);
    
# endif /*  _STLP_NO_WCHAR_T */
#endif /* _STLP_WINCE */

  }

  _STLP_CATCH_ALL {}
}

void _STLP_CALL ios_base::_S_uninitialize()
{
  // Note that destroying output streambufs flushes the buffers.

  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&cin);
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&cout);
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&cerr);
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&clog);

# ifndef _STLP_NO_WCHAR_T
  wistream* ptr_wcin  = __REINTERPRET_CAST(wistream*,&wcin);
  wostream* ptr_wcout = __REINTERPRET_CAST(wostream*,&wcout);
  wostream* ptr_wcerr = __REINTERPRET_CAST(wostream*,&wcerr);
  wostream* ptr_wclog = __REINTERPRET_CAST(wostream*,&wclog);
# endif

  // we don't want any exceptions being thrown here
  ptr_cin->exceptions(0);
  ptr_cout->exceptions(0);
  ptr_cerr->exceptions(0);
  ptr_clog->exceptions(0);

  delete ptr_cin->rdbuf(0);
  delete ptr_cout->rdbuf(0);
  delete ptr_cerr->rdbuf(0);
  delete ptr_clog->rdbuf(0);

  _Destroy(ptr_cin);
  _Destroy(ptr_cout);
  _Destroy(ptr_cerr);
  _Destroy(ptr_clog);

# ifndef _STLP_NO_WCHAR_T
  // we don't want any exceptions being thrown here
  ptr_wcin->exceptions(0);
  ptr_wcout->exceptions(0);
  ptr_wcerr->exceptions(0);
  ptr_wclog->exceptions(0);

  delete ptr_wcin->rdbuf(0);
  delete ptr_wcout->rdbuf(0);
  delete ptr_wcerr->rdbuf(0);
  delete ptr_wclog->rdbuf(0);

  _Destroy(ptr_wcin);
  _Destroy(ptr_wcout);
  _Destroy(ptr_wcerr);
  _Destroy(ptr_wclog);

# endif
    if (--_Loc_init::_S_count == 0) {
      locale::_S_uninitialize();
    }
}


bool _STLP_CALL ios_base::sync_with_stdio(bool sync) {
#if !defined(STLP_WINCE)
# ifndef _STLP_HAS_NO_NAMESPACES
  using _SgI::stdio_istreambuf;
  using _SgI::stdio_ostreambuf;
# endif
  
  bool was_synced =  _S_was_synced;

  // if by any chance we got there before std streams initialization,
  // just set the sync flag and exit
  if (Init::_S_count == 0) {
    _S_was_synced = sync;
    return was_synced;
  }

  istream* ptr_cin  = __REINTERPRET_CAST(istream*,&cin);
  ostream* ptr_cout = __REINTERPRET_CAST(ostream*,&cout);
  ostream* ptr_cerr = __REINTERPRET_CAST(ostream*,&cerr);
  ostream* ptr_clog = __REINTERPRET_CAST(ostream*,&clog);

  streambuf* old_cin  = ptr_cin->rdbuf();
  streambuf* old_cout = ptr_cout->rdbuf();
  streambuf* old_cerr = ptr_cerr->rdbuf();
  streambuf* old_clog = ptr_clog->rdbuf();

  streambuf* new_cin  = 0;
  streambuf* new_cout = 0;
  streambuf* new_cerr = 0;
  streambuf* new_clog = 0;

  _STLP_TRY {
    if (sync && !was_synced) {
      new_cin  = new stdio_istreambuf(stdin);
      new_cout = new stdio_ostreambuf(stdout);
      new_cerr = new stdio_ostreambuf(stderr);
      new_clog = new stdio_ostreambuf(stderr);
    }
    else if (!sync && was_synced) {
      new_cin  = _Stl_create_filebuf(stdin, ios_base::in);
      new_cout = _Stl_create_filebuf(stdout, ios_base::out);
      new_cerr = _Stl_create_filebuf(stderr, ios_base::out);
      new_clog = _Stl_create_filebuf(stderr, ios_base::out);
    }
  }
  _STLP_CATCH_ALL {}

  if (new_cin && new_cout && new_cerr && new_clog) {
    ptr_cin->rdbuf(new_cin);
    ptr_cout->rdbuf(new_cout);
    ptr_cerr->rdbuf(new_cerr);
    ptr_clog->rdbuf(new_clog);

    delete old_cin;
    delete old_cout;
    delete old_cerr;
    delete old_clog;
  }
  else {
    delete new_cin;
    delete new_cout;
    delete new_cerr;
    delete new_clog;
  }

  return was_synced;
#else
  return false;
#endif /* _STLP_WINCE */
}

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:
