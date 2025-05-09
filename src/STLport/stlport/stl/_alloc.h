/*
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_ALLOC_H
#define _STLP_INTERNAL_ALLOC_H

# ifndef _STLP_CSTDDEF
#  include <cstddef>
# endif

#if !defined (_STLP_DEBUG_H) && (defined  (_STLP_DEBUG) || defined (_STLP_ASSERTIONS))
# include <stl/debug/_debug.h>
#endif

# ifndef _STLP_CSTDLIB
#  include <cstdlib>
# endif
# ifndef _STLP_CSTRING
#  include <cstring>
# endif

# ifndef __THROW_BAD_ALLOC
#  if !defined(_STLP_USE_EXCEPTIONS)
#   if !defined (_STLP_CSTDIO)
#    include <cstdio>
#   endif
#   if !defined (_STLP_CSTDLIB)
#    include <cstdlib>
#   endif
#   define __THROW_BAD_ALLOC puts("out of memory\n"); exit(1)
#  else /* !defined(_STLP_USE_EXCEPTIONS) */
#   define __THROW_BAD_ALLOC throw _STLP_STD::bad_alloc()
#  endif /* !defined(_STLP_USE_EXCEPTIONS) */
# endif   /* __THROW_BAD_ALLOC */

# ifndef _STLP_NEW_HEADER
#  include <new>
# endif

#if /* defined (_STLP_THREADS) && */ ! defined (_STLP_INTERNAL_THREADS_H)
# include <stl/_threads.h>
#endif

#ifndef _STLP_INTERNAL_CONSTRUCT_H
# include <stl/_construct.h>
#endif

#ifndef __ALLOC
#   define __ALLOC __sgi_alloc
#endif

# ifndef __RESTRICT
#  define __RESTRICT
# endif

#if defined (_STLP_THREADS) || (defined(_STLP_OWN_IOSTREAMS) && ! defined (_STLP_NO_THREADS) && ! defined (_NOTHREADS) )
# define _STLP_NODE_ALLOCATOR_THREADS true
#else
# define _STLP_NODE_ALLOCATOR_THREADS false
#endif

_STLP_BEGIN_NAMESPACE

# if defined (_STLP_USE_RAW_SGI_ALLOCATORS)
template <class _Tp, class _Alloc> struct __allocator;
# endif

// Malloc-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.

typedef void (* __oom_handler_type)();

template <int __inst>
class __malloc_alloc {
private:
  static void* _STLP_CALL _S_oom_malloc(size_t);
  static __oom_handler_type __oom_handler;
public:
  // this one is needed for proper simple_alloc wrapping
  typedef char value_type;
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) && defined (_STLP_USE_RAW_SGI_ALLOCATORS)
  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, __malloc_alloc<__inst> > other;
  };
# endif
  static void* _STLP_CALL allocate(size_t __n)    {
    void* __result = malloc(__n);
    if (0 == __result) __result = _S_oom_malloc(__n);
    return __result;
  }
  static void _STLP_CALL deallocate(void* __p, size_t /* __n */) { free((char*)__p); }
  static __oom_handler_type _STLP_CALL set_malloc_handler(__oom_handler_type __f) {
    __oom_handler_type __old = __oom_handler;
    __oom_handler = __f;
    return(__old);
  }
};


// New-based allocator.  Typically slower than default alloc below.
// Typically thread-safe and more storage efficient.
class _STLP_CLASS_DECLSPEC __new_alloc {
public:
  // this one is needed for proper simple_alloc wrapping
  typedef char value_type;
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) &&  defined(_STLP_USE_RAW_SGI_ALLOCATORS)
  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, __new_alloc > other;
  };
# endif
  static void* _STLP_CALL  allocate(size_t __n) {  return __stl_new(__n); }
  static void _STLP_CALL deallocate(void* __p, size_t) { __stl_delete(__p); }
};


// Allocator adaptor to check size arguments for debugging.
// Reports errors using assert.  Checking can be disabled with
// NDEBUG, but it's far better to just use the underlying allocator
// instead when no checking is desired.
// There is some evidence that this can confuse Purify.
// This adaptor can only be applied to raw allocators

template <class _Alloc>
class __debug_alloc : public _Alloc {
public:
  typedef _Alloc __allocator_type;
  typedef typename _Alloc::value_type value_type;
private:
  struct __alloc_header {
    size_t __magic: 16;
    size_t __type_size:16;
    _STLP_UINT32_T _M_size;
  }; // that is 8 bytes for sure
  // Sunpro CC has bug on enums, so extra_before/after set explicitly
  enum { __pad=8, __magic=0xdeba, __deleted_magic = 0xdebd,
	 __shred_byte= _STLP_SHRED_BYTE
  };

  enum { __extra_before = 16, __extra_after = 8 };
  // Size of space used to store size.  Note
  // that this must be large enough to preserve
  // alignment.
  static size_t _STLP_CALL __extra_before_chunk() {
    return (long)__extra_before/sizeof(value_type)+
      (size_t)((long)__extra_before%sizeof(value_type)>0);
  }
  static size_t _STLP_CALL __extra_after_chunk() {
    return (long)__extra_after/sizeof(value_type)+
      (size_t)((long)__extra_after%sizeof(value_type)>0);
  }
public:
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) && defined (_STLP_USE_RAW_SGI_ALLOCATORS)
  template <class _Tp1> struct rebind {
    typedef __allocator< _Tp1, __debug_alloc<_Alloc> > other;
  };
# endif
  __debug_alloc() {}
  ~__debug_alloc() {}
  static void * _STLP_CALL allocate(size_t);
  static void _STLP_CALL deallocate(void *, size_t);
};


// Default node allocator.
// With a reasonable compiler, this should be roughly as fast as the
// original STL class-specific allocators, but with less fragmentation.
// Default_alloc_template parameters are experimental and MAY
// DISAPPEAR in the future.  Clients should just use alloc for now.
//
// Important implementation properties:
// 1. If the client request an object of size > _MAX_BYTES, the resulting
//    object will be obtained directly from malloc.
// 2. In all other cases, we allocate an object of size exactly
//    _S_round_up(requested_size).  Thus the client has enough size
//    information that we can return the object to the proper free list
//    without permanently losing part of the object.
//

// The first template parameter specifies whether more than one thread
// may use this allocator.  It is safe to allocate an object from
// one instance of a default_alloc and deallocate it with another
// one.  This effectively transfers its ownership to the second one.
// This may have undesirable effects on reference locality.
// The second parameter is unreferenced and serves only to allow the
// creation of multiple default_alloc instances.

# if defined(__OS400__)
enum {_ALIGN = 16, _ALIGN_SHIFT=4, _MAX_BYTES = 256};
#  define  _STLP_NFREELISTS 16
# else
enum {_ALIGN = 8, _ALIGN_SHIFT=3, _MAX_BYTES = 128};
#  define  _STLP_NFREELISTS 16
# endif /* __OS400__ */

class _STLP_CLASS_DECLSPEC _Node_alloc_obj {
public:
    _Node_alloc_obj * _M_free_list_link;
};

template <bool __threads, int __inst>
class __node_alloc {
  _STLP_PRIVATE:
  static inline size_t _STLP_CALL _S_round_up(size_t __bytes) { return (((__bytes) + (size_t)_ALIGN-1) & ~((size_t)_ALIGN - 1)); }
  typedef _Node_alloc_obj _Obj;
private:
  // Returns an object of size __n, and optionally adds to size __n free list.
  static void*  _STLP_CALL _S_refill(size_t __n);
  // Allocates a chunk for nobjs of size size.  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  static char*  _STLP_CALL _S_chunk_alloc(size_t __p_size, int& __nobjs);
  // Chunk allocation state.
  static _Node_alloc_obj * _STLP_VOLATILE _S_free_list[_STLP_NFREELISTS]; 
  static char* _S_start_free;
  static char* _S_end_free;
  static size_t _S_heap_size;
  static void * _STLP_CALL _M_allocate(size_t __n);
  /* __p may not be 0 */
  static void _STLP_CALL _M_deallocate(void *__p, size_t __n);
public:
  // this one is needed for proper simple_alloc wrapping
  typedef char value_type;
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) && defined (_STLP_USE_RAW_SGI_ALLOCATORS)
  template <class _Tp1> struct rebind {
    typedef __allocator<_Tp1, __node_alloc<__threads, __inst> > other;
  };
# endif
  /* __n must be > 0      */
  static void * _STLP_CALL allocate(size_t __n) { return (__n > (size_t)_MAX_BYTES) ?  __stl_new(__n) : _M_allocate(__n); }
  /* __p may not be 0 */
  static void _STLP_CALL deallocate(void *__p, size_t __n) { if (__n > (size_t)_MAX_BYTES) __stl_delete(__p); else _M_deallocate(__p, __n); }
};

# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS __malloc_alloc<0>;
_STLP_EXPORT_TEMPLATE_CLASS __node_alloc<_STLP_NODE_ALLOCATOR_THREADS, 0>;
# endif /* _STLP_USE_TEMPLATE_EXPORT */
typedef __node_alloc<_STLP_NODE_ALLOCATOR_THREADS, 0> _Node_alloc;
# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<_Node_alloc>;
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<__new_alloc>;
_STLP_EXPORT_TEMPLATE_CLASS __debug_alloc<__malloc_alloc<0> >;
# endif

# if defined (_STLP_USE_PERTHREAD_ALLOC)

_STLP_END_NAMESPACE
// include additional header here
# include <stl/_pthread_alloc.h>
_STLP_BEGIN_NAMESPACE

#  if defined ( _STLP_DEBUG_ALLOC )
typedef __debug_alloc<__pthread_alloc> __sgi_alloc;
#  else
typedef __pthread_alloc __sgi_alloc;
#  endif /* _STLP_DEBUG_ALLOC */

typedef __pthread_alloc __single_client_alloc;
typedef __pthread_alloc __multithreaded_alloc;

# else

# if defined ( _STLP_USE_NEWALLOC )

#  if defined ( _STLP_DEBUG_ALLOC )
typedef __debug_alloc<__new_alloc> __sgi_alloc;
#  else
typedef __new_alloc __sgi_alloc;
#  endif /* _STLP_DEBUG_ALLOC */

typedef __new_alloc __single_client_alloc;
typedef __new_alloc __multithreaded_alloc;

#  elif defined (_STLP_USE_MALLOC)

#   if defined ( _STLP_DEBUG_ALLOC )
typedef __debug_alloc<__malloc_alloc<0> > __sgi_alloc;
#   else
typedef __malloc_alloc<0> __sgi_alloc;
#   endif /* _STLP_DEBUG_ALLOC */

typedef __malloc_alloc<0> __single_client_alloc;
typedef __malloc_alloc<0> __multithreaded_alloc;

# else

#   if defined ( _STLP_DEBUG_ALLOC )
typedef __debug_alloc<_Node_alloc> __sgi_alloc;
#   else
typedef _Node_alloc __sgi_alloc;
#   endif

typedef __node_alloc<false, 0> __single_client_alloc;
typedef __node_alloc<true, 0>  __multithreaded_alloc;

#  endif /* _STLP_USE_NEWALLOC */
# endif /* PTHREAD_ALLOC */

// This implements allocators as specified in the C++ standard.  
//
// Note that standard-conforming allocators use many language features
// that are not yet widely implemented.  In particular, they rely on
// member templates, partial specialization, partial ordering of function
// templates, the typename keyword, and the use of the template keyword
// to refer to a template member of a dependent type.

template <class _Tp>
class allocator {
public:

  typedef _Tp        value_type;
  typedef value_type *       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES)
  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
# endif
  allocator() _STLP_NOTHROW {}
 # if defined (_STLP_MEMBER_TEMPLATES)
  template <class _Tp1> allocator(const allocator<_Tp1>&) _STLP_NOTHROW {}
 # endif    
  allocator(const allocator<_Tp>&) _STLP_NOTHROW {}
  ~allocator() _STLP_NOTHROW {}
  pointer address(reference __x) { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }
  // __n is permitted to be 0.  The C++ standard says nothing about what the return value is when __n == 0.
  _Tp* allocate(size_type __n, const void* = 0) const { 
    return __n != 0 ? __REINTERPRET_CAST(value_type*,__sgi_alloc::allocate(__n * sizeof(value_type))) : 0;
  }
  // __p is permitted to be a null pointer, only if n==0.
  void deallocate(pointer __p, size_type __n) const {
    _STLP_ASSERT( (__p == 0) == (__n == 0) )
      if (__p != 0) __sgi_alloc::deallocate((void*)__p, __n * sizeof(value_type));
  }
  // backwards compatibility
  void deallocate(pointer __p) const {  if (__p != 0) __sgi_alloc::deallocate((void*)__p, sizeof(value_type)); }
  size_type max_size() const _STLP_NOTHROW  { return size_t(-1) / sizeof(value_type); }
  void construct(pointer __p, const _Tp& __val) const { _STLP_STD::_Construct(__p, __val); }
  void destroy(pointer __p) const { _STLP_STD::_Destroy(__p); }
# if defined(__MRC__)||defined(__SC__)
  template <class _T2> bool operator==(const allocator<_T2>&) const  { return true; }
  template <class _T2> bool operator!=(const allocator<_T2>&) const { return false; }
# endif
};

_STLP_TEMPLATE_NULL
class _STLP_CLASS_DECLSPEC allocator<void> {
public:
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;
  typedef void*       pointer;
  typedef const void* const_pointer;
# if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
  typedef void        value_type;
# endif
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES)
  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };
# endif
# if defined(__MRC__)||defined(__SC__)		//*ty 03/24/2001 - MPW compilers get confused on these operator definitions
  template <class _T2> bool operator==(const allocator<_T2>&) const  { return true; }
  template <class _T2> bool operator!=(const allocator<_T2>&) const { return false; }
# endif
};

#if !(defined(__MRC__)||defined(__SC__))		//*ty 03/24/2001 - MPW compilers get confused on these operator definitions
template <class _T1, class _T2> inline bool  _STLP_CALL operator==(const allocator<_T1>&, const allocator<_T2>&)  { return true; }
template <class _T1, class _T2> inline bool  _STLP_CALL operator!=(const allocator<_T1>&, const allocator<_T2>&) { return false; }
#endif

# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS allocator<char>;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS allocator<wchar_t>;
#  endif
# endif /* _STLP_USE_TEMPLATE_EXPORT */

// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.

// The fully general version.
template <class _Tp, class _Allocator>
struct _Alloc_traits
{
  typedef _Allocator _Orig;
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) 
  typedef typename _Allocator::_STLP_TEMPLATE rebind<_Tp> _Rebind_type;
  typedef typename _Rebind_type::other  allocator_type;
  static allocator_type create_allocator(const _Orig& __a) { return allocator_type(__a); }
# else
  // this is not actually true, used only to pass this type through
  // to dynamic overload selection in _STLP_alloc_proxy methods
  typedef _Allocator allocator_type;
# endif
};

#ifndef _STLP_FORCE_ALLOCATORS
#define _STLP_FORCE_ALLOCATORS(a,y) 
#endif

#if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION) && ! defined (_STLP_MEMBER_TEMPLATE_CLASSES)
// The version for the default allocator, for rare occasion when we have partial spec w/o member template classes
template <class _Tp, class _Tp1>
struct _Alloc_traits<_Tp, allocator<_Tp1> > {
  typedef allocator<_Tp1> _Orig;
  typedef allocator<_Tp> allocator_type;
  static allocator_type create_allocator(const allocator<_Tp1 >& __a) { return allocator_type(__a); }
};
#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

/* macro to convert the allocator for initialization
 * not using MEMBER_TEMPLATE_CLASSES as it should work given template constructor  */
#if defined (_STLP_MEMBER_TEMPLATES) || ! defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
/* if _STLP_NO_TEMPLATE_CONVERSIONS is set, the member template constructor is
 * not used implicitly to convert allocator parameter, so let us do it explicitly */
# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) && defined (_STLP_NO_TEMPLATE_CONVERSIONS)
#  define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __stl_alloc_create(__a,(_Tp*)0)
# else
#  define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __a
# endif
/* else convert, but only if partial specialization works, since else
 * Container::allocator_type won't be different */
#else 
#  define _STLP_CONVERT_ALLOCATOR(__a, _Tp) __stl_alloc_create(__a,(_Tp*)0)
#endif

# if defined (_STLP_MEMBER_TEMPLATE_CLASSES) 
template <class _Tp, class _Alloc>
inline _STLP_TYPENAME_ON_RETURN_TYPE _Alloc_traits<_Tp, _Alloc>::allocator_type  _STLP_CALL
__stl_alloc_create(const _Alloc& __a, const _Tp*) {
  typedef typename _Alloc::_STLP_TEMPLATE rebind<_Tp>::other _Rebound_type;
  return _Rebound_type(__a);
}
#else
// If custom allocators are being used without member template classes support :
// user (on purpose) is forced to define rebind/get operations !!!
template <class _Tp1, class _Tp2>
inline allocator<_Tp2>& _STLP_CALL
__stl_alloc_rebind(allocator<_Tp1>& __a, const _Tp2*) {  return (allocator<_Tp2>&)(__a); }
template <class _Tp1, class _Tp2>
inline allocator<_Tp2> _STLP_CALL
__stl_alloc_create(const allocator<_Tp1>&, const _Tp2*) { return allocator<_Tp2>(); }
#endif /* _STLP_MEMBER_TEMPLATE_CLASSES */

# ifdef _STLP_USE_RAW_SGI_ALLOCATORS
// move obsolete stuff out of the way
# include <stl/_alloc_old.h>
# endif

// inheritance is being used for EBO optimization
template <class _Value, class _Tp, class _MaybeReboundAlloc>
class _STLP_alloc_proxy : public _MaybeReboundAlloc {
private:
  typedef _MaybeReboundAlloc _Base;
  typedef _STLP_alloc_proxy<_Value, _Tp, _MaybeReboundAlloc> _Self;
public:
  _Value _M_data;
  inline _STLP_alloc_proxy(const _MaybeReboundAlloc& __a, _Value __p) : _MaybeReboundAlloc(__a), _M_data(__p) {}

# if 0
  inline _STLP_alloc_proxy(const _Self& __x) : _MaybeReboundAlloc(__x), _M_data(__x._M_data) {} 
  // construction/destruction
  inline _Self& operator = (const _Self& __x) { 
    *(_MaybeReboundAlloc*)this = *(_MaybeReboundAlloc*)__x;
    _M_data = __x._M_data; return *this; 
  } 
  inline _Self& operator = (const _Base& __x) { ((_Base&)*this) = __x; return *this; } 
# endif
  // Unified interface to perform allocate()/deallocate() with limited
  // language support
#if ! defined (_STLP_MEMBER_TEMPLATE_CLASSES)
  // else it is rebound already, and allocate() member is accessible
  inline _Tp* allocate(size_t __n) { 
    return __stl_alloc_rebind(__STATIC_CAST(_Base&,*this),(_Tp*)0).allocate(__n,0); 
  }
  inline void deallocate(_Tp* __p, size_t __n) { 
    __stl_alloc_rebind(__STATIC_CAST(_Base&, *this),(_Tp*)0).deallocate(__p, __n); 
  }
#endif
};

# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<char *,char,allocator<char> >;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS _STLP_alloc_proxy<wchar_t *,wchar_t,allocator<wchar_t> >;
#  endif
# endif /* _STLP_USE_TEMPLATE_EXPORT */

# undef _STLP_NODE_ALLOCATOR_THREADS

_STLP_END_NAMESPACE

# if defined (_STLP_EXPOSE_GLOBALS_IMPLEMENTATION) && !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_alloc.c>
# endif

#endif /* _STLP_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:

