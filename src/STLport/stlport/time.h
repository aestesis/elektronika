/*
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

# if !defined (_STLP_OUTERMOST_HEADER_ID)
#  define _STLP_OUTERMOST_HEADER_ID 0x272
#  include <stl/_prolog.h>
# elif (_STLP_OUTERMOST_HEADER_ID == 0x272) && ! defined (_STLP_DONT_POP_0x272)
#  define _STLP_DONT_POP_0x272
# endif

#if !defined(_STLP_WINCE)
# include _STLP_NATIVE_C_HEADER(time.h)
#endif

# if (_STLP_OUTERMOST_HEADER_ID == 0x272)
#  if ! defined (_STLP_DONT_POP_0x272)
#   include <stl/_epilog.h>
#   undef  _STLP_OUTERMOST_HEADER_ID
#   endif
#   undef  _STLP_DONT_POP_0x272
# endif

// Local Variables:
// mode:C++
// End:
