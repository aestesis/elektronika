/*
 * libxslt.h: internal header only used during the compilation of libxslt
 *
 * See COPYRIGHT for the status of this software
 *
 * Author: daniel@veillard.com
 */

#ifndef __XSLT_LIBXSLT_H__
#define __XSLT_LIBXSLT_H__

#if defined(WIN32) && !defined (__CYGWIN__)
#include <win32config.h>
#else
#include "config.h"
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#include "xsltwin32config.h"
#else
#include "xsltconfig.h"
#include <libxml/xmlversion.h>
#endif

#if !defined LIBXSLT_PUBLIC
#if (defined (__CYGWIN__) || defined _MSC_VER) && !defined IN_LIBXSLT && !defined LIBXSLT_STATIC
#define LIBXSLT_PUBLIC __declspec(dllimport)
#else
#define LIBXSLT_PUBLIC 
#endif
#endif

#endif /* ! __XSLT_LIBXSLT_H__ */
