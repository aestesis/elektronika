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

// rope<_CharT,_Alloc> is a sequence of _CharT.
// Ropes appear to be mutable, but update operations
// really copy enough of the data structure to leave the original
// valid.  Thus ropes can be logically copied by just copying
// a pointer value.

#ifndef _STLP_INTERNAL_ROPE_H
# define _STLP_INTERNAL_ROPE_H

# ifndef _STLP_INTERNAL_ALGOBASE_H
#  include <stl/_algobase.h>
# endif

# ifndef _STLP_IOSFWD
#  include <iosfwd>
# endif

# ifndef _STLP_INTERNAL_ALLOC_H
#  include <stl/_alloc.h>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_H
#  include <stl/_iterator.h>
# endif

# ifndef _STLP_INTERNAL_ALGO_H
#  include <stl/_algo.h>
# endif

# ifndef _STLP_INTERNAL_FUNCTION_H
#  include <stl/_function.h>
# endif

# ifndef _STLP_INTERNAL_NUMERIC_H
#  include <stl/_numeric.h>
# endif

# ifndef _STLP_INTERNAL_HASH_FUN_H
#  include <stl/_hash_fun.h>
# endif

# ifdef __GC
#   define __GC_CONST const
# else
# include <stl/_threads.h>
#   define __GC_CONST   // constant except for deallocation
# endif
# ifdef _STLP_SGI_THREADS
#    include <mutex.h>
# endif

#ifdef _STLP_MEMBER_TEMPLATE_CLASSES 
#  define _STLP_CREATE_ALLOCATOR(__atype,__a, _Tp) (_Alloc_traits<_Tp,__atype>::create_allocator(__a)) 
#elif defined(__MRC__)||defined(__SC__) 
#  define _STLP_CREATE_ALLOCATOR(__atype,__a, _Tp) __stl_alloc_create<_Tp,__atype>(__a,(_Tp*)0) 
#else 
#  define _STLP_CREATE_ALLOCATOR(__atype,__a, _Tp) __stl_alloc_create(__a,(_Tp*)0) 
#endif 

_STLP_BEGIN_NAMESPACE

// First a lot of forward declarations.  The standard seems to require
// much stricter "declaration before use" than many of the implementations
// that preceded it.
template<class _CharT, _STLP_DEFAULT_ALLOCATOR_SELECT(_CharT) > class rope;
template<class _CharT, class _Alloc> struct _Rope_RopeConcatenation;
template<class _CharT, class _Alloc> struct _Rope_RopeRep;
template<class _CharT, class _Alloc> struct _Rope_RopeLeaf;
template<class _CharT, class _Alloc> struct _Rope_RopeFunction;
template<class _CharT, class _Alloc> struct _Rope_RopeSubstring;
template<class _CharT, class _Alloc> class _Rope_iterator;
template<class _CharT, class _Alloc> class _Rope_const_iterator;
template<class _CharT, class _Alloc> class _Rope_char_ref_proxy;
template<class _CharT, class _Alloc> class _Rope_char_ptr_proxy;

// Some helpers, so we can use power on ropes.
// See below for why this isn't local to the implementation.

// This uses a nonstandard refcount convention.
// The result has refcount 0.
template<class _CharT, class _Alloc>
struct _Rope_Concat_fn
  : public binary_function<rope<_CharT,_Alloc>, rope<_CharT,_Alloc>,
  rope<_CharT,_Alloc> > {
  rope<_CharT,_Alloc> operator() (const rope<_CharT,_Alloc>& __x,
                                  const rope<_CharT,_Alloc>& __y) {
    return __x + __y;
  }
};

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>
__identity_element(_Rope_Concat_fn<_CharT, _Alloc>)
{
  return rope<_CharT,_Alloc>();
}

// The _S_eos function is used for those functions that
// convert to/from C-like strings to detect the end of the string.

// The end-of-C-string character.
// This is what the draft standard says it should be.
template <class _CharT>
inline _CharT _S_eos(_CharT*) { return _CharT(); }

// fbp : some compilers fail to zero-initialize builtins ;(
inline const char _S_eos(const char*) { return 0; }
# ifdef _STLP_HAS_WCHAR_T
inline const wchar_t _S_eos(const wchar_t*) { return 0; }
# endif

// Test for basic character types.
// For basic character types leaves having a trailing eos.
template <class _CharT>
inline bool _S_is_basic_char_type(_CharT*) { return false; }
template <class _CharT>
inline bool _S_is_one_byte_char_type(_CharT*) { return false; }

inline bool _S_is_basic_char_type(char*) { return true; }
inline bool _S_is_one_byte_char_type(char*) { return true; }
# ifdef _STLP_HAS_WCHAR_T
inline bool _S_is_basic_char_type(wchar_t*) { return true; }
# endif

// Store an eos iff _CharT is a basic character type.
// Do not reference _S_eos if it isn't.
template <class _CharT>
inline void _S_cond_store_eos(_CharT&) {}

inline void _S_cond_store_eos(char& __c) { __c = 0; }
# ifdef _STLP_HAS_WCHAR_T
inline void _S_cond_store_eos(wchar_t& __c) { __c = 0; }
# endif

// char_producers are logically functions that generate a section of
// a string.  These can be convereted to ropes.  The resulting rope
// invokes the char_producer on demand.  This allows, for example,
// files to be viewed as ropes without reading the entire file.
template <class _CharT>
class char_producer {
public:
  virtual ~char_producer() {};
  virtual void operator()(size_t __start_pos, size_t __len, 
                          _CharT* __buffer) = 0;
  // Buffer should really be an arbitrary output iterator.
  // That way we could flatten directly into an ostream, etc.
  // This is thoroughly impossible, since iterator types don't
  // have runtime descriptions.
};

// Sequence buffers:
//
// Sequence must provide an append operation that appends an
// array to the sequence.  Sequence buffers are useful only if
// appending an entire array is cheaper than appending element by element.
// This is true for many string representations.
// This should  perhaps inherit from ostream<sequence::value_type>
// and be implemented correspondingly, so that they can be used
// for formatted.  For the sake of portability, we don't do this yet.
//
// For now, sequence buffers behave as output iterators.  But they also
// behave a little like basic_ostringstream<sequence::value_type> and a
// little like containers.

template<class _Sequence
# if !(defined (_STLP_NON_TYPE_TMPL_PARAM_BUG) || \
       defined ( _STLP_NO_DEFAULT_NON_TYPE_PARAM ))
, size_t _Buf_sz = 100
#   if defined(__sgi) && !defined(__GNUC__)
#	 define __TYPEDEF_WORKAROUND
,class _V = typename _Sequence::value_type
#   endif /* __sgi */
# endif /* _STLP_NON_TYPE_TMPL_PARAM_BUG */
>
// The 3rd parameter works around a common compiler bug.
class sequence_buffer : public iterator <output_iterator_tag, void, void, void, void> {
public:
#       ifndef __TYPEDEF_WORKAROUND
  typedef typename _Sequence::value_type value_type;
  typedef sequence_buffer<_Sequence
# if !(defined (_STLP_NON_TYPE_TMPL_PARAM_BUG) || \
       defined ( _STLP_NO_DEFAULT_NON_TYPE_PARAM ))
  , _Buf_sz
  > _Self;
# else /* _STLP_NON_TYPE_TMPL_PARAM_BUG */
  > _Self;
  enum { _Buf_sz = 100}; 
# endif /* _STLP_NON_TYPE_TMPL_PARAM_BUG */
  // # endif
#	else /* __TYPEDEF_WORKAROUND */
  typedef _V value_type;
  typedef sequence_buffer<_Sequence, _Buf_sz, _V> _Self;
#	endif /* __TYPEDEF_WORKAROUND */
protected:
  _Sequence* _M_prefix;
  value_type _M_buffer[_Buf_sz];
  size_t     _M_buf_count;
public:
  void flush() {
    _M_prefix->append(_M_buffer, _M_buffer + _M_buf_count);
    _M_buf_count = 0;
  }
  ~sequence_buffer() { flush(); }
  sequence_buffer() : _M_prefix(0), _M_buf_count(0) {}
  sequence_buffer(const _Self& __x) {
    _M_prefix = __x._M_prefix;
    _M_buf_count = __x._M_buf_count;
    copy(__x._M_buffer, __x._M_buffer + __x._M_buf_count, _M_buffer);
  }
  sequence_buffer(_Self& __x) {
    __x.flush();
    _M_prefix = __x._M_prefix;
    _M_buf_count = 0;
  }
  sequence_buffer(_Sequence& __s) : _M_prefix(&__s), _M_buf_count(0) {}
  _Self& operator= (_Self& __x) {
    __x.flush();
    _M_prefix = __x._M_prefix;
    _M_buf_count = 0;
    return *this;
  }
  _Self& operator= (const _Self& __x) {
    _M_prefix = __x._M_prefix;
    _M_buf_count = __x._M_buf_count;
    copy(__x._M_buffer, __x._M_buffer + __x._M_buf_count, _M_buffer);
    return *this;
  }
  void push_back(value_type __x)
  {
    if (_M_buf_count < _Buf_sz) {
      _M_buffer[_M_buf_count] = __x;
      ++_M_buf_count;
    } else {
      flush();
      _M_buffer[0] = __x;
      _M_buf_count = 1;
    }
  }
  void append(value_type* __s, size_t __len)
  {
    if (__len + _M_buf_count <= _Buf_sz) {
      size_t __i = _M_buf_count;
      size_t __j = 0;
      for (; __j < __len; __i++, __j++) {
        _M_buffer[__i] = __s[__j];
      }
      _M_buf_count += __len;
    } else if (0 == _M_buf_count) {
      _M_prefix->append(__s, __s + __len);
    } else {
      flush();
      append(__s, __len);
    }
  }
  _Self& write(value_type* __s, size_t __len)
  {
    append(__s, __len);
    return *this;
  }
  _Self& put(value_type __x)
  {
    push_back(__x);
    return *this;
  }
  _Self& operator=(const value_type& __rhs)
  {
    push_back(__rhs);
    return *this;
  }
  _Self& operator*() { return *this; }
  _Self& operator++() { return *this; }
  _Self& operator++(int) { return *this; }
};

// The following should be treated as private, at least for now.
template<class _CharT>
class _Rope_char_consumer {
public:
  // If we had member templates, these should not be virtual.
  // For now we need to use run-time parametrization where
  // compile-time would do.  _Hence this should all be private
  // for now.
  // The symmetry with char_producer is accidental and temporary.
  virtual ~_Rope_char_consumer() {};
  virtual bool operator()(const _CharT* __buffer, size_t __len) = 0;
};

//
// What follows should really be local to rope.  Unfortunately,
// that doesn't work, since it makes it impossible to define generic
// equality on rope iterators.  According to the draft standard, the
// template parameters for such an equality operator cannot be inferred
// from the occurence of a member class as a parameter.
// (SGI compilers in fact allow this, but the __result wouldn't be
// portable.)
// Similarly, some of the static member functions are member functions
// only to avoid polluting the global namespace, and to circumvent
// restrictions on type inference for template functions.
//

//
// The internal data structure for representing a rope.  This is
// private to the implementation.  A rope is really just a pointer
// to one of these.
//
// A few basic functions for manipulating this data structure
// are members of _RopeRep.  Most of the more complex algorithms
// are implemented as rope members.
//
// Some of the static member functions of _RopeRep have identically
// named functions in rope that simply invoke the _RopeRep versions.
//
// A macro to introduce various allocation and deallocation functions
// These need to be defined differently depending on whether or not
// we are using standard conforming allocators, and whether the allocator
// instances have real state.  Thus this macro is invoked repeatedly
// with different definitions of __ROPE_DEFINE_ALLOC.

#if defined (_STLP_MEMBER_TEMPLATE_CLASSES)
# define __ROPE_DEFINE_ALLOC(_Tp, __name, _M_proxy) \
        typedef typename \
          _Alloc_traits<_Tp,_Alloc>::allocator_type __name##Allocator;

#define __ROPE_DEFINE_ALLOCS(__a, _M_proxy) \
        __ROPE_DEFINE_ALLOC(_CharT,_Data, _M_proxy) /* character data */ \
        typedef _Rope_RopeConcatenation<_CharT,__a> __C; \
        __ROPE_DEFINE_ALLOC(__C,_C, _M_proxy) \
        typedef _Rope_RopeLeaf<_CharT,__a> __L; \
        __ROPE_DEFINE_ALLOC(__L,_L, _M_proxy) \
        typedef _Rope_RopeFunction<_CharT,__a> __F; \
        __ROPE_DEFINE_ALLOC(__F,_F, _M_proxy) \
        typedef _Rope_RopeSubstring<_CharT,__a> __S; \
        __ROPE_DEFINE_ALLOC(__S,_S,_M_proxy)
#else
#define __ROPE_DEFINE_ALLOC(_Tp, __name, _M_proxy) 
#define __ROPE_DEFINE_ALLOCS(__a, _M_proxy)
#endif


template<class _CharT, class _Alloc>
struct _Rope_RopeRep
# ifndef __GC
  : public _Refcount_Base
# endif
{
  typedef _Rope_RopeRep<_CharT, _Alloc> _Self;
public:
#  define __ROPE_MAX_DEPTH  45
#  define __ROPE_DEPTH_SIZE 46
  enum { _S_max_rope_depth = __ROPE_MAX_DEPTH };
  enum _Tag {_S_leaf, _S_concat, _S_substringfn, _S_function};
  // Apparently needed by VC++
  // The data fields of leaves are allocated with some
  // extra space, to accomodate future growth and for basic
  // character types, to hold a trailing eos character.
  enum { _S_alloc_granularity = 8 };

  
  _Tag _M_tag:8;
  bool _M_is_balanced:8;

  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Alloc_traits<_CharT,_Alloc>::allocator_type
  allocator_type;
  
  allocator_type get_allocator() const { return allocator_type(_M_size);  }

  unsigned char _M_depth;
  __GC_CONST _CharT* _M_c_string;
  _STLP_alloc_proxy<size_t, _CharT, allocator_type> _M_size;

# ifdef _STLP_NO_ARROW_OPERATOR
  _Rope_RopeRep() : _Refcount_Base(1), _M_size(allocator_type(), 0) {}
# endif

  /* Flattened version of string, if needed.  */
  /* typically 0.                             */
  /* If it's not 0, then the memory is owned  */
  /* by this node.                            */
  /* In the case of a leaf, this may point to */
  /* the same memory as the data field.       */
  _Rope_RopeRep(_Tag __t, int __d, bool __b, size_t _p_size,
                allocator_type __a) :
#         ifndef __GC
    _Refcount_Base(1),
#	  endif
    _M_tag(__t), _M_is_balanced(__b), _M_depth(__d), _M_c_string(0), _M_size(__a, _p_size)
  { }
#   ifdef __GC
  void _M_incr () {}
#   endif

  // fbp : moved from RopeLeaf
  static size_t _S_rounded_up_size(size_t __n) {
    size_t __size_with_eos;
    
    if (_S_is_basic_char_type((_CharT*)0)) {
      __size_with_eos = __n + 1;
    } else {
      __size_with_eos = __n;
    }
#       ifdef __GC
    return __size_with_eos;
#       else
    // Allow slop for in-place expansion.
    return (__size_with_eos + _S_alloc_granularity-1)
      &~ (_S_alloc_granularity-1);
#       endif
  }

  static void _S_free_string(__GC_CONST _CharT* __s, size_t __len,
                             allocator_type __a) {

    if (!_S_is_basic_char_type((_CharT*)0)) {
      _Destroy(__s, __s + __len);
    }
    //  This has to be a static member, so this gets a bit messy
#   ifdef _STLP_MEMBER_TEMPLATE_CLASSES
    __a.deallocate(__s, _S_rounded_up_size(__len));		//*ty 03/24/2001 - restored not to use __stl_alloc_rebind() since it is not defined under _STLP_MEMBER_TEMPLATE_CLASSES
#   else
    __stl_alloc_rebind (__a, (_CharT*)0).deallocate(__s, _S_rounded_up_size(__len));
#   endif
  }
  
  // Deallocate data section of a leaf.
  // This shouldn't be a member function.
  // But its hard to do anything else at the
  // moment, because it's templatized w.r.t.
  // an allocator.
  // Does nothing if __GC is defined.
#   ifndef __GC
  void _M_free_c_string();
  void _M_free_tree();
  // Deallocate t. Assumes t is not 0.
  void _M_unref_nonnil()
  {
    _M_decr(); if (!_M_ref_count) _M_free_tree();
  }
  void _M_ref_nonnil()
  {
    _M_incr();
  }
  static void _S_unref(_Self* __t)
  {
    if (0 != __t) {
      __t->_M_unref_nonnil();
    }
  }
  static void _S_ref(_Self* __t)
  {
    if (0 != __t) __t->_M_incr();
  }
  static void _S_free_if_unref(_Self* __t)
  {
    if (0 != __t && 0 == __t->_M_ref_count) __t->_M_free_tree();
  }
#   else /* __GC */
  void _M_unref_nonnil() {}
  void _M_ref_nonnil() {}
  static void _S_unref(_Self*) {}
  static void _S_ref(_Self*) {}
  static void _S_free_if_unref(_Self*) {}
#   endif

  __ROPE_DEFINE_ALLOCS(_Alloc, _M_size)
    };

template<class _CharT, class _Alloc>
struct _Rope_RopeLeaf : public _Rope_RopeRep<_CharT,_Alloc> {
public:
  __GC_CONST _CharT* _M_data; /* Not necessarily 0 terminated. */
                                /* The allocated size is         */
                                /* _S_rounded_up_size(size), except */
                                /* in the GC case, in which it   */
                                /* doesn't matter.               */
  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Rope_RopeRep<_CharT,_Alloc>::allocator_type allocator_type;
  _Rope_RopeLeaf(__GC_CONST _CharT* __d, size_t _p_size, allocator_type __a)
    : _Rope_RopeRep<_CharT,_Alloc>(_Rope_RopeRep<_CharT,_Alloc>::_S_leaf, 0, true, _p_size, __a), 
    _M_data(__d)
  {
    _STLP_ASSERT(_p_size > 0)
    if (_S_is_basic_char_type((_CharT *)0)) {
      // already eos terminated.
      this->_M_c_string = __d;
    }
  }

# ifdef _STLP_NO_ARROW_OPERATOR
  _Rope_RopeLeaf() {}
  _Rope_RopeLeaf(const _Rope_RopeLeaf<_CharT, _Alloc>& ) {}
# endif
  
// The constructor assumes that d has been allocated with
  // the proper allocator and the properly padded size.
  // In contrast, the destructor deallocates the data:
# ifndef __GC
  ~_Rope_RopeLeaf() {
    if (_M_data != this->_M_c_string) {
      this->_M_free_c_string();
    }
    _S_free_string(_M_data, this->_M_size._M_data, this->get_allocator());
  }
# endif
};

template<class _CharT, class _Alloc>
struct _Rope_RopeConcatenation : public _Rope_RopeRep<_CharT,_Alloc> {
public:
  _Rope_RopeRep<_CharT,_Alloc>* _M_left;
  _Rope_RopeRep<_CharT,_Alloc>* _M_right;
  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Rope_RopeRep<_CharT,_Alloc>::allocator_type allocator_type;
  _Rope_RopeConcatenation(_Rope_RopeRep<_CharT,_Alloc>* __l,
                          _Rope_RopeRep<_CharT,_Alloc>* __r,
                          allocator_type __a)
    :   _Rope_RopeRep<_CharT,_Alloc>(
                                     _Rope_RopeRep<_CharT,_Alloc>::_S_concat, 
				     (max)(__l->_M_depth, __r->_M_depth) + 1, false,
                                     __l->_M_size._M_data + __r->_M_size._M_data, __a), _M_left(__l), _M_right(__r)
  {}
# ifdef _STLP_NO_ARROW_OPERATOR
  _Rope_RopeConcatenation() {}
  _Rope_RopeConcatenation(const _Rope_RopeConcatenation<_CharT, _Alloc>&) {}
# endif

# ifndef __GC
  ~_Rope_RopeConcatenation() {
    this->_M_free_c_string();
    _M_left->_M_unref_nonnil();
    _M_right->_M_unref_nonnil();
  }
# endif
};

template<class _CharT, class _Alloc>
struct _Rope_RopeFunction : public _Rope_RopeRep<_CharT,_Alloc> {
public:
  char_producer<_CharT>* _M_fn;
#   ifndef __GC
  bool _M_delete_when_done; // Char_producer is owned by the
                                // rope and should be explicitly
                                // deleted when the rope becomes
                                // inaccessible.
#   else
  // In the GC case, we either register the rope for
  // finalization, or not.  Thus the field is unnecessary;
  // the information is stored in the collector data structures.
  // We do need a finalization procedure to be invoked by the
  // collector.
  static void _S_fn_finalization_proc(void * __tree, void *) {
    delete ((_Rope_RopeFunction *)__tree) -> _M_fn;
  }
#   endif
  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Rope_RopeRep<_CharT,_Alloc>::allocator_type allocator_type;
# ifdef _STLP_NO_ARROW_OPERATOR
  _Rope_RopeFunction() {}
  _Rope_RopeFunction(const _Rope_RopeFunction<_CharT, _Alloc>& ) {}
# endif

  _Rope_RopeFunction(char_producer<_CharT>* __f, size_t _p_size,
                     bool __d, allocator_type __a)
    :
    _Rope_RopeRep<_CharT,_Alloc>(_Rope_RopeRep<_CharT,_Alloc>::_S_function, 0, true, _p_size, __a),
    _M_fn(__f)
#       ifndef __GC
    , _M_delete_when_done(__d)
#       endif
  {
    _STLP_ASSERT(_p_size > 0)
#       ifdef __GC
    if (__d) {
      GC_REGISTER_FINALIZER(
                            this, _Rope_RopeFunction::_S_fn_finalization_proc, 0, 0, 0);
    }
#       endif
  }
# ifndef __GC
  ~_Rope_RopeFunction() {
    this->_M_free_c_string();
    if (_M_delete_when_done) {
      delete _M_fn;
    }
  }
# endif
};
// Substring results are usually represented using just
// concatenation nodes.  But in the case of very long flat ropes
// or ropes with a functional representation that isn't practical.
// In that case, we represent the __result as a special case of
// RopeFunction, whose char_producer points back to the rope itself.
// In all cases except repeated substring operations and
// deallocation, we treat the __result as a RopeFunction.
template<class _CharT, class _Alloc>
# if  ( defined (__IBMCPP__) && (__IBMCPP__ == 500) )  // JFA 10-Aug-2000 for some reason xlC cares about the order
struct _Rope_RopeSubstring : public char_producer<_CharT> , public _Rope_RopeFunction<_CharT,_Alloc>
# else
struct _Rope_RopeSubstring : public _Rope_RopeFunction<_CharT,_Alloc>,
                             public char_producer<_CharT>
# endif
{
public:
  // XXX this whole class should be rewritten.
  typedef _Rope_RopeRep<_CharT,_Alloc> _Base;
  _Rope_RopeRep<_CharT,_Alloc>* _M_base;      // not 0
  size_t _M_start;
  virtual void operator()(size_t __start_pos, size_t __req_len,
                          _CharT* __buffer) {
    switch(_M_base->_M_tag) {
    case _Base::_S_function:
    case _Base::_S_substringfn:
      {
        char_producer<_CharT>* __fn =
          ((_Rope_RopeFunction<_CharT,_Alloc>*)_M_base)->_M_fn;
        _STLP_ASSERT(__start_pos + __req_len <= this->_M_size._M_data)
        _STLP_ASSERT(_M_start + this->_M_size._M_data <= _M_base->_M_size._M_data)
        (*__fn)(__start_pos + _M_start, __req_len, __buffer);
      }
      break;
    case _Base::_S_leaf:
      {
        __GC_CONST _CharT* __s =
          ((_Rope_RopeLeaf<_CharT,_Alloc>*)_M_base)->_M_data;
        uninitialized_copy_n(__s + __start_pos + _M_start, __req_len,
                             __buffer);
      }
      break;
    default:
      _STLP_ASSERT(false)
        ;
    }
  }

  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Rope_RopeRep<_CharT,_Alloc>::allocator_type allocator_type;

  _Rope_RopeSubstring(_Rope_RopeRep<_CharT,_Alloc>* __b, size_t __s,
                      size_t __l, allocator_type __a)
    : _Rope_RopeFunction<_CharT,_Alloc>(this, __l, false, __a),
	_M_base(__b),
    _M_start(__s)
       
  {
    _STLP_ASSERT(__l > 0)
    _STLP_ASSERT(__s + __l <= __b->_M_size._M_data)
#       ifndef __GC
    _M_base->_M_ref_nonnil();
#       endif
    this->_M_tag = _Base::_S_substringfn;
  }
  virtual ~_Rope_RopeSubstring()
  { 
#       ifndef __GC
    _M_base->_M_unref_nonnil();
#       endif
  }
};

// Self-destructing pointers to Rope_rep.
// These are not conventional smart pointers.  Their
// only purpose in life is to ensure that unref is called
// on the pointer either at normal exit or if an exception
// is raised.  It is the caller's responsibility to
// adjust reference counts when these pointers are initialized
// or assigned to.  (This convention significantly reduces
// the number of potentially expensive reference count
// updates.)
#ifndef __GC
template<class _CharT, class _Alloc>
struct _Rope_self_destruct_ptr {
  _Rope_RopeRep<_CharT,_Alloc>* _M_ptr;
  ~_Rope_self_destruct_ptr() 
  { _Rope_RopeRep<_CharT,_Alloc>::_S_unref(_M_ptr); }
#   ifdef _STLP_USE_EXCEPTIONS
  _Rope_self_destruct_ptr() : _M_ptr(0) {};
#   else
  _Rope_self_destruct_ptr() {};
#   endif
  _Rope_self_destruct_ptr(_Rope_RopeRep<_CharT,_Alloc>* __p) : _M_ptr(__p) {}
  _Rope_RopeRep<_CharT,_Alloc>& operator*() { return *_M_ptr; }
  _Rope_RopeRep<_CharT,_Alloc>* operator->() { return _M_ptr; }
  operator _Rope_RopeRep<_CharT,_Alloc>*() { return _M_ptr; }
  _Rope_self_destruct_ptr<_CharT, _Alloc>& 
  operator= (_Rope_RopeRep<_CharT,_Alloc>* __x)
  { _M_ptr = __x; return *this; }
};
#endif

// Dereferencing a nonconst iterator has to return something
// that behaves almost like a reference.  It's not possible to
// return an actual reference since assignment requires extra
// work.  And we would get into the same problems as with the
// CD2 version of basic_string.
template<class _CharT, class _Alloc>
class _Rope_char_ref_proxy {
  typedef _Rope_char_ref_proxy<_CharT, _Alloc> _Self;
  friend class rope<_CharT,_Alloc>;
  friend class _Rope_iterator<_CharT,_Alloc>;
  friend class _Rope_char_ptr_proxy<_CharT,_Alloc>;
#   ifdef __GC
  typedef _Rope_RopeRep<_CharT,_Alloc>* _Self_destruct_ptr;
#   else
  typedef _Rope_self_destruct_ptr<_CharT,_Alloc> _Self_destruct_ptr;
#   endif
  typedef _Rope_RopeRep<_CharT,_Alloc> _RopeRep;
  typedef rope<_CharT,_Alloc> _My_rope;
  size_t _M_pos;
  _CharT _M_current;
  bool _M_current_valid;
  _My_rope* _M_root;     // The whole rope.
public:
  _Rope_char_ref_proxy(_My_rope* __r, size_t __p) :
    _M_pos(__p), _M_current_valid(false), _M_root(__r) {}
  _Rope_char_ref_proxy(const _Self& __x) :
    _M_pos(__x._M_pos), _M_current_valid(false), _M_root(__x._M_root) {}
  // Don't preserve cache if the reference can outlive the
  // expression.  We claim that's not possible without calling
  // a copy constructor or generating reference to a proxy
  // reference.  We declare the latter to have undefined semantics.
  _Rope_char_ref_proxy(_My_rope* __r, size_t __p,
                       _CharT __c) :
    _M_pos(__p), _M_current(__c), _M_current_valid(true), _M_root(__r) {}
  inline operator _CharT () const;
  _Self& operator= (_CharT __c);
  _Rope_char_ptr_proxy<_CharT, _Alloc> operator& () const;
  _Self& operator= (const _Self& __c) {
    return operator=((_CharT)__c); 
  }
};

#ifdef _STLP_FUNCTION_TMPL_PARTIAL_ORDER
template<class _CharT, class __Alloc>
inline void swap(_Rope_char_ref_proxy <_CharT, __Alloc > __a,
                 _Rope_char_ref_proxy <_CharT, __Alloc > __b) {
  _CharT __tmp = __a;
  __a = __b;
  __b = __tmp;
}
#else
// There is no really acceptable way to handle this.  The default
// definition of swap doesn't work for proxy references.
// It can't really be made to work, even with ugly hacks, since
// the only unusual operation it uses is the copy constructor, which
// is needed for other purposes.  We provide a macro for
// full specializations, and instantiate the most common case.
# define _ROPE_SWAP_SPECIALIZATION(_CharT, __Alloc) \
    inline void swap(_Rope_char_ref_proxy <_CharT, __Alloc > __a, \
                     _Rope_char_ref_proxy <_CharT, __Alloc > __b) { \
        _CharT __tmp = __a; \
        __a = __b; \
        __b = __tmp; \
    }

_ROPE_SWAP_SPECIALIZATION(char,_STLP_DEFAULT_ALLOCATOR(char) )

#endif /* !_STLP_FUNCTION_TMPL_PARTIAL_ORDER */

  template<class _CharT, class _Alloc>
class _Rope_char_ptr_proxy {
  // XXX this class should be rewritten.
public:
  typedef _Rope_char_ptr_proxy<_CharT, _Alloc> _Self;
  friend class _Rope_char_ref_proxy<_CharT,_Alloc>;
  size_t _M_pos;
  rope<_CharT,_Alloc>* _M_root;     // The whole rope.

  _Rope_char_ptr_proxy(const _Rope_char_ref_proxy<_CharT,_Alloc>& __x) 
    : _M_pos(__x._M_pos), _M_root(__x._M_root) {}
  _Rope_char_ptr_proxy(const _Self& __x)
    : _M_pos(__x._M_pos), _M_root(__x._M_root) {}
  _Rope_char_ptr_proxy() {}
  _Rope_char_ptr_proxy(_CharT* __x) : _M_pos(0), _M_root(0) {
    _STLP_ASSERT(0 == __x)
  }
  _Self& 
  operator= (const _Self& __x) {
    _M_pos = __x._M_pos;
    _M_root = __x._M_root;
    return *this;
  }

  _Rope_char_ref_proxy<_CharT,_Alloc> operator*() const {
    return _Rope_char_ref_proxy<_CharT,_Alloc>(_M_root, _M_pos);
  }
};


// Rope iterators:
// Unlike in the C version, we cache only part of the stack
// for rope iterators, since they must be efficiently copyable.
// When we run out of cache, we have to reconstruct the iterator
// value.
// Pointers from iterators are not included in reference counts.
// Iterators are assumed to be thread private.  Ropes can
// be shared.

template<class _CharT, class _Alloc>
class _Rope_iterator_base
/*   : public random_access_iterator<_CharT, ptrdiff_t>  */
{
  friend class rope<_CharT,_Alloc>;
  typedef _Rope_iterator_base<_CharT, _Alloc> _Self;
public:
  typedef _Rope_RopeRep<_CharT,_Alloc> _RopeRep;
  // Borland doesnt want this to be protected.
  //  protected:
  enum { _S_path_cache_len = 4 }; // Must be <= 9.
  enum { _S_iterator_buf_len = 15 };
  size_t _M_current_pos;
  _RopeRep* _M_root;     // The whole rope.
  size_t _M_leaf_pos;    // Starting position for current leaf
  __GC_CONST _CharT* _M_buf_start;
  // Buffer possibly
  // containing current char.
  __GC_CONST _CharT* _M_buf_ptr;
  // Pointer to current char in buffer.
  // != 0 ==> buffer valid.
  __GC_CONST _CharT* _M_buf_end;
  // One past __last valid char in buffer.
  // What follows is the path cache.  We go out of our
  // way to make this compact.
  // Path_end contains the bottom section of the path from
  // the root to the current leaf.
  const _RopeRep* _M_path_end[_S_path_cache_len];
  int _M_leaf_index;     // Last valid __pos in path_end;
  // _M_path_end[0] ... _M_path_end[leaf_index-1]
  // point to concatenation nodes.
  unsigned char _M_path_directions;
  // (path_directions >> __i) & 1 is 1
  // iff we got from _M_path_end[leaf_index - __i - 1]
  // to _M_path_end[leaf_index - __i] by going to the
  // __right. Assumes path_cache_len <= 9.
  _CharT _M_tmp_buf[_S_iterator_buf_len];
  // Short buffer for surrounding chars.
  // This is useful primarily for 
  // RopeFunctions.  We put the buffer
  // here to avoid locking in the
  // multithreaded case.
  // The cached path is generally assumed to be valid
  // only if the buffer is valid.
  static void _S_setbuf(_Rope_iterator_base<_CharT, _Alloc>& __x);
  // Set buffer contents given
  // path cache.
  static void _S_setcache(_Rope_iterator_base<_CharT, _Alloc>& __x);
  // Set buffer contents and
  // path cache.
  static void _S_setcache_for_incr(_Rope_iterator_base<_CharT, _Alloc>& __x);
  // As above, but assumes path
  // cache is valid for previous posn.
  _Rope_iterator_base() {}
  _Rope_iterator_base(_RopeRep* __root, size_t __pos)
    : _M_current_pos(__pos),_M_root(__root),  _M_buf_ptr(0) {}
  void _M_incr(size_t __n);
  void _M_decr(size_t __n);
public:
  size_t index() const { return _M_current_pos; }
  _Rope_iterator_base(const _Self& __x) {
    if (0 != __x._M_buf_ptr) {
      *this = __x;
    } else {
      _M_current_pos = __x._M_current_pos;
      _M_root = __x._M_root;
      _M_buf_ptr = 0;
    }
  }
};

template<class _CharT, class _Alloc> class _Rope_iterator;

template<class _CharT, class _Alloc>
class _Rope_const_iterator : public _Rope_iterator_base<_CharT,_Alloc> {
  friend class rope<_CharT,_Alloc>;
  typedef  _Rope_const_iterator<_CharT, _Alloc> _Self;
  typedef _Rope_iterator_base<_CharT,_Alloc> _Base;
  //  protected:
public:
#   ifndef _STLP_HAS_NO_NAMESPACES
  typedef _Rope_RopeRep<_CharT,_Alloc> _RopeRep;
  // The one from the base class may not be directly visible.
#   endif
  _Rope_const_iterator(const _RopeRep* __root, size_t __pos):
    _Rope_iterator_base<_CharT,_Alloc>(
                                       __CONST_CAST(_RopeRep*,__root), __pos)
    // Only nonconst iterators modify root ref count
  {}
public:
  typedef _CharT reference;   // Really a value.  Returning a reference
                                // Would be a mess, since it would have
                                // to be included in refcount.
  typedef const _CharT* pointer;
  typedef _CharT value_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag iterator_category;

public:
  _Rope_const_iterator() {};
  _Rope_const_iterator(const _Self& __x) :
    _Rope_iterator_base<_CharT,_Alloc>(__x) { }
  _Rope_const_iterator(const _Rope_iterator<_CharT,_Alloc>& __x): 
    _Rope_iterator_base<_CharT,_Alloc>(__x) {}
  _Rope_const_iterator(const rope<_CharT,_Alloc>& __r, size_t __pos) :
    _Rope_iterator_base<_CharT,_Alloc>(__r._M_tree_ptr._M_data, __pos) {}
  _Self& operator= (const _Self& __x) {
    if (0 != __x._M_buf_ptr) {
      *(__STATIC_CAST(_Base*,this)) = __x;
    } else {
      this->_M_current_pos = __x._M_current_pos;
      this->_M_root = __x._M_root;
      this->_M_buf_ptr = 0;
    }
    return(*this);
  }
  reference operator*() {
    if (0 == this->_M_buf_ptr) _S_setcache(*this);
    return *(this->_M_buf_ptr);
  }
  _Self& operator++() {
    __GC_CONST _CharT* __next;
    if (0 != this->_M_buf_ptr && (__next = this->_M_buf_ptr + 1) < this->_M_buf_end) {
      this->_M_buf_ptr = __next;
      ++this->_M_current_pos;
    } else {
      this->_M_incr(1);
    }
    return *this;
  }
  _Self& operator+=(ptrdiff_t __n) {
    if (__n >= 0) {
      this->_M_incr(__n);
    } else {
      this->_M_decr(-__n);
    }
    return *this;
  }
  _Self& operator--() {
    this->_M_decr(1);
    return *this;
  }
  _Self& operator-=(ptrdiff_t __n) {
    if (__n >= 0) {
      this->_M_decr(__n);
    } else {
      this->_M_incr(-__n);
    }
    return *this;
  }
  _Self operator++(int) {
    size_t __old_pos = this->_M_current_pos;
    this->_M_incr(1);
    return _Rope_const_iterator<_CharT,_Alloc>(this->_M_root, __old_pos);
    // This makes a subsequent dereference expensive.
    // Perhaps we should instead copy the iterator
    // if it has a valid cache?
  }
  _Self operator--(int) {
    size_t __old_pos = this->_M_current_pos;
    this->_M_decr(1);
    return _Rope_const_iterator<_CharT,_Alloc>(this->_M_root, __old_pos);
  }
  inline reference operator[](size_t __n);
};

template<class _CharT, class _Alloc>
class _Rope_iterator : public _Rope_iterator_base<_CharT,_Alloc> {
  friend class rope<_CharT,_Alloc>;
  typedef _Rope_iterator<_CharT, _Alloc> _Self;
  typedef _Rope_iterator_base<_CharT,_Alloc> _Base;
  typedef _Rope_RopeRep<_CharT,_Alloc> _RopeRep;
  //  protected:
public:
  rope<_CharT,_Alloc>* _M_root_rope;
  // root is treated as a cached version of this,
  // and is used to detect changes to the underlying
  // rope.
  // Root is included in the reference count.
  // This is necessary so that we can detect changes reliably.
  // Unfortunately, it requires careful bookkeeping for the
  // nonGC case.
  _Rope_iterator(rope<_CharT,_Alloc>* __r, size_t __pos);
  
  void _M_check();
public:
  typedef _Rope_char_ref_proxy<_CharT,_Alloc>  reference;
  typedef _Rope_char_ref_proxy<_CharT,_Alloc>* pointer;
  typedef _CharT value_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag iterator_category;
public:
  ~_Rope_iterator() 		//*TY 5/6/00 - added dtor to balance reference count
  {
    _RopeRep::_S_unref(this->_M_root);
  }
  
  rope<_CharT,_Alloc>& container() { return *_M_root_rope; }
  _Rope_iterator() {
    this->_M_root = 0;  // Needed for reference counting.
  };
  _Rope_iterator(const  _Self& __x) :
    _Rope_iterator_base<_CharT,_Alloc>(__x) {
    _M_root_rope = __x._M_root_rope;
    _RopeRep::_S_ref(this->_M_root);
  }
  _Rope_iterator(rope<_CharT,_Alloc>& __r, size_t __pos);
  _Self& operator= (const  _Self& __x) {
    _RopeRep* __old = this->_M_root;
    
    _RopeRep::_S_ref(__x._M_root);
    if (0 != __x._M_buf_ptr) {
      _M_root_rope = __x._M_root_rope;
      *(__STATIC_CAST(_Base*,this)) = __x;
    } else {
      this->_M_current_pos = __x._M_current_pos;
      this->_M_root = __x._M_root;
      _M_root_rope = __x._M_root_rope;
      this->_M_buf_ptr = 0;
    }
    _RopeRep::_S_unref(__old);
    return(*this);
  }
  reference operator*() {
    _M_check();
    if (0 == this->_M_buf_ptr) {
      return _Rope_char_ref_proxy<_CharT,_Alloc>(
                                                 _M_root_rope, this->_M_current_pos);
    } else {
      return _Rope_char_ref_proxy<_CharT,_Alloc>(
                                                 _M_root_rope, this->_M_current_pos, *(this->_M_buf_ptr));
    }
  }
  _Self& operator++() {
    this->_M_incr(1);
    return *this;
  }
  _Self& operator+=(ptrdiff_t __n) {
    if (__n >= 0) {
      this->_M_incr(__n);
    } else {
      this->_M_decr(-__n);
    }
    return *this;
  }
  _Self& operator--() {
    this->_M_decr(1);
    return *this;
  }
  _Self& operator-=(ptrdiff_t __n) {
    if (__n >= 0) {
      this->_M_decr(__n);
    } else {
      this->_M_incr(-__n);
    }
    return *this;
  }
  _Self operator++(int) {
    size_t __old_pos = this->_M_current_pos;
    this->_M_incr(1);
    return _Rope_iterator<_CharT,_Alloc>(_M_root_rope, __old_pos);
  }
  _Self operator--(int) {
    size_t __old_pos = this->_M_current_pos;
    this->_M_decr(1);
    return _Rope_iterator<_CharT,_Alloc>(_M_root_rope, __old_pos);
  }
  reference operator[](ptrdiff_t __n) {
    return _Rope_char_ref_proxy<_CharT,_Alloc>(
                                               _M_root_rope, this->_M_current_pos + __n);
  }
};

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
template <class _CharT, class _Alloc>
inline random_access_iterator_tag
iterator_category(const _Rope_iterator<_CharT,_Alloc>&) {  return random_access_iterator_tag();}
template <class _CharT, class _Alloc>
inline _CharT* value_type(const _Rope_iterator<_CharT,_Alloc>&) { return 0; }
template <class _CharT, class _Alloc>
inline ptrdiff_t* distance_type(const _Rope_iterator<_CharT,_Alloc>&) { return 0; }
template <class _CharT, class _Alloc>
inline random_access_iterator_tag
iterator_category(const _Rope_const_iterator<_CharT,_Alloc>&) { return random_access_iterator_tag(); }
template <class _CharT, class _Alloc>
inline _CharT* value_type(const _Rope_const_iterator<_CharT,_Alloc>&) { return 0; }
template <class _CharT, class _Alloc>
inline ptrdiff_t* distance_type(const _Rope_const_iterator<_CharT,_Alloc>&) { return 0; }
#endif

template <class _CharT, class _Alloc>
class rope {
  typedef rope<_CharT,_Alloc> _Self;
public:
  typedef _CharT value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef _CharT const_reference;
  typedef const _CharT* const_pointer;
  typedef _Rope_iterator<_CharT,_Alloc> iterator;
  typedef _Rope_const_iterator<_CharT,_Alloc> const_iterator;
  typedef _Rope_char_ref_proxy<_CharT,_Alloc> reference;
  typedef _Rope_char_ptr_proxy<_CharT,_Alloc> pointer;
  
  friend class _Rope_iterator<_CharT,_Alloc>;
  friend class _Rope_const_iterator<_CharT,_Alloc>;
  friend struct _Rope_RopeRep<_CharT,_Alloc>;
  friend class _Rope_iterator_base<_CharT,_Alloc>;
  friend class _Rope_char_ptr_proxy<_CharT,_Alloc>;
  friend class _Rope_char_ref_proxy<_CharT,_Alloc>;
  friend struct _Rope_RopeSubstring<_CharT,_Alloc>;

  _STLP_DECLARE_RANDOM_ACCESS_REVERSE_ITERATORS;
  
protected:
  typedef __GC_CONST _CharT* _Cstrptr;
  
  static _CharT _S_empty_c_str[1];
  
  static bool _S_is0(_CharT __c) { return __c == _S_eos((_CharT*)0); }
  enum { _S_copy_max = 23 };
  // For strings shorter than _S_copy_max, we copy to
  // concatenate.
  
public:
  typedef _Rope_RopeRep<_CharT, _Alloc> _RopeRep;
  _STLP_FORCE_ALLOCATORS(_CharT, _Alloc)
  typedef typename _Alloc_traits<_CharT,_Alloc>::allocator_type  allocator_type;
  allocator_type get_allocator() const { return allocator_type(_M_tree_ptr); }
public:
  // The only data member of a rope:
  _STLP_alloc_proxy<_RopeRep*, _CharT, allocator_type> _M_tree_ptr;

  typedef _Rope_RopeConcatenation<_CharT,_Alloc> _RopeConcatenation;
  typedef _Rope_RopeLeaf<_CharT,_Alloc> _RopeLeaf;
  typedef _Rope_RopeFunction<_CharT,_Alloc> _RopeFunction;
  typedef _Rope_RopeSubstring<_CharT,_Alloc> _RopeSubstring;



  // Retrieve a character at the indicated position.
  static _CharT _S_fetch(_RopeRep* __r, size_type __pos);

#       ifndef __GC
  // Obtain a pointer to the character at the indicated position.
  // The pointer can be used to change the character.
  // If such a pointer cannot be produced, as is frequently the
  // case, 0 is returned instead.
  // (Returns nonzero only if all nodes in the path have a refcount
  // of 1.)
  static _CharT* _S_fetch_ptr(_RopeRep* __r, size_type __pos);
#       endif

  static bool _S_apply_to_pieces(
                                // should be template parameter
                                 _Rope_char_consumer<_CharT>& __c,
                                 const _RopeRep* __r,
                                 size_t __begin, size_t __end);
                                // begin and end are assumed to be in range.

#       ifndef __GC
  static void _S_unref(_RopeRep* __t)
  {
    _RopeRep::_S_unref(__t);
  }
  static void _S_ref(_RopeRep* __t)
  {
    _RopeRep::_S_ref(__t);
  }
#       else /* __GC */
  static void _S_unref(_RopeRep*) {}
  static void _S_ref(_RopeRep*) {}
#       endif


#       ifdef __GC
  typedef _Rope_RopeRep<_CharT,_Alloc>* _Self_destruct_ptr;
#       else
  typedef _Rope_self_destruct_ptr<_CharT,_Alloc> _Self_destruct_ptr;
#       endif

  // _Result is counted in refcount.
  static _RopeRep* _S_substring(_RopeRep* __base,
                                size_t __start, size_t __endp1);

  static _RopeRep* _S_concat_char_iter(_RopeRep* __r,
                                       const _CharT* __iter, size_t __slen);
  // Concatenate rope and char ptr, copying __s.
  // Should really take an arbitrary iterator.
  // Result is counted in refcount.
  static _RopeRep* _S_destr_concat_char_iter(_RopeRep* __r,
                                             const _CharT* __iter, size_t __slen)
    // As above, but one reference to __r is about to be
    // destroyed.  Thus the pieces may be recycled if all
    // relevent reference counts are 1.
#           ifdef __GC
    // We can't really do anything since refcounts are unavailable.
  { return _S_concat_char_iter(__r, __iter, __slen); }
#           else
  ;
#           endif

  static _RopeRep* _S_concat_rep(_RopeRep* __left, _RopeRep* __right);
  // General concatenation on _RopeRep.  _Result
  // has refcount of 1.  Adjusts argument refcounts.

public:
  void apply_to_pieces( size_t __begin, size_t __end,
                        _Rope_char_consumer<_CharT>& __c) const {
    _S_apply_to_pieces(__c, _M_tree_ptr._M_data, __begin, __end);
  }


protected:

  static size_t _S_rounded_up_size(size_t __n) {
    return _RopeRep::_S_rounded_up_size(__n);
  }

  static size_t _S_allocated_capacity(size_t __n) {
    if (_S_is_basic_char_type((_CharT*)0)) {
      return _S_rounded_up_size(__n) - 1;
    } else {
      return _S_rounded_up_size(__n);
    }
  }
                
  // Allocate and construct a RopeLeaf using the supplied allocator
  // Takes ownership of s instead of copying.
  static _RopeLeaf* _S_new_RopeLeaf(__GC_CONST _CharT *__s,
                                    size_t _p_size, allocator_type __a)
  {
   _RopeLeaf* __space = _STLP_CREATE_ALLOCATOR(allocator_type,__a, _RopeLeaf).allocate(1,(const void*)0);
    _STLP_TRY {
      _STLP_PLACEMENT_NEW(__space) _RopeLeaf(__s, _p_size, __a);
    }
   _STLP_UNWIND(_STLP_CREATE_ALLOCATOR(allocator_type,__a, 
                                   _RopeLeaf).deallocate(__space, 1))
	  return __space;
  }

  static _RopeConcatenation* _S_new_RopeConcatenation(
                                                      _RopeRep* __left, _RopeRep* __right,
                                                      allocator_type __a)
  {
   _RopeConcatenation* __space = _STLP_CREATE_ALLOCATOR(allocator_type,__a,
                                                    _RopeConcatenation).allocate(1,(const void*)0);
    return _STLP_PLACEMENT_NEW(__space) _RopeConcatenation(__left, __right, __a);
  }

  static _RopeFunction* _S_new_RopeFunction(char_producer<_CharT>* __f,
                                            size_t _p_size, bool __d, allocator_type __a)
  {
   _RopeFunction* __space = _STLP_CREATE_ALLOCATOR(allocator_type,__a, 
                                               _RopeFunction).allocate(1,(const void*)0);
    return _STLP_PLACEMENT_NEW(__space) _RopeFunction(__f, _p_size, __d, __a);
  }

  static _RopeSubstring* _S_new_RopeSubstring(
                                              _Rope_RopeRep<_CharT,_Alloc>* __b, size_t __s,
                                              size_t __l, allocator_type __a)
  {
   _RopeSubstring* __space = _STLP_CREATE_ALLOCATOR(allocator_type,__a, 
                                                _RopeSubstring).allocate(1,(const void*)0);
    return _STLP_PLACEMENT_NEW(__space) _RopeSubstring(__b, __s, __l, __a);
  }

#         define _STLP_ROPE_FROM_UNOWNED_CHAR_PTR(__s, _p_size, __a) \
                _S_RopeLeaf_from_unowned_char_ptr(__s, _p_size, __a)     

  static
  _RopeLeaf* _S_RopeLeaf_from_unowned_char_ptr(const _CharT *__s,
                                               size_t _p_size, allocator_type __a)
  {
    if (0 == _p_size) return 0;

   _CharT* __buf = _STLP_CREATE_ALLOCATOR(allocator_type,__a, _CharT).allocate(_S_rounded_up_size(_p_size));

    uninitialized_copy_n(__s, _p_size, __buf);
    _S_cond_store_eos(__buf[_p_size]);

    _STLP_TRY {
      return _S_new_RopeLeaf(__buf, _p_size, __a);
    }
    _STLP_UNWIND(_RopeRep::_S_free_string(__buf, _p_size, __a))
            
# if defined (_STLP_THROW_RETURN_BUG)
      return 0;
# endif
  }
            

  // Concatenation of nonempty strings.
  // Always builds a concatenation node.
  // Rebalances if the result is too deep.
  // Result has refcount 1.
  // Does not increment left and right ref counts even though
  // they are referenced.
  static _RopeRep*
  _S_tree_concat(_RopeRep* __left, _RopeRep* __right);

  // Concatenation helper functions
  static _RopeLeaf*
  _S_leaf_concat_char_iter(_RopeLeaf* __r,
                           const _CharT* __iter, size_t __slen);
  // Concatenate by copying leaf.
  // should take an arbitrary iterator
  // result has refcount 1.
#       ifndef __GC
  static _RopeLeaf* _S_destr_leaf_concat_char_iter
  (_RopeLeaf* __r, const _CharT* __iter, size_t __slen);
  // A version that potentially clobbers __r if __r->_M_ref_count == 1.
#       endif


  // A helper function for exponentiating strings.
  // This uses a nonstandard refcount convention.
  // The result has refcount 0.
  friend struct _Rope_Concat_fn<_CharT,_Alloc>;
  typedef _Rope_Concat_fn<_CharT,_Alloc> _Concat_fn;

public:
  static size_t _S_char_ptr_len(const _CharT* __s) {
    const _CharT* __p = __s;
	  
    while (!_S_is0(*__p)) { ++__p; }
    return (__p - __s);
  }

public: /* for operators */
  rope(_RopeRep* __t, const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, __t) { }
private:
  // Copy __r to the _CharT buffer.
  // Returns __buffer + __r->_M_size._M_data.
  // Assumes that buffer is uninitialized.
  static _CharT* _S_flatten(_RopeRep* __r, _CharT* __buffer);

  // Again, with explicit starting position and length.
  // Assumes that buffer is uninitialized.
  static _CharT* _S_flatten(_RopeRep* __r,
                            size_t __start, size_t __len,
                            _CharT* __buffer);

  // fbp : HP aCC prohibits access to protected min_len from within static methods ( ?? )
public:
  static const unsigned long _S_min_len[46];
protected:
  static bool _S_is_balanced(_RopeRep* __r)
  { return (__r->_M_size._M_data >= _S_min_len[__r->_M_depth]); }

  static bool _S_is_almost_balanced(_RopeRep* __r)
  { return (__r->_M_depth == 0 ||
            __r->_M_size._M_data >= _S_min_len[__r->_M_depth - 1]); }

  static bool _S_is_roughly_balanced(_RopeRep* __r)
  { return (__r->_M_depth <= 1 ||
            __r->_M_size._M_data >= _S_min_len[__r->_M_depth - 2]); }

  // Assumes the result is not empty.
  static _RopeRep* _S_concat_and_set_balanced(_RopeRep* __left,
                                              _RopeRep* __right)
  {
    _RopeRep* __result = _S_concat_rep(__left, __right);
    if (_S_is_balanced(__result)) __result->_M_is_balanced = true;
    return __result;
  }

  // The basic rebalancing operation.  Logically copies the
  // rope.  The result has refcount of 1.  The client will
  // usually decrement the reference count of __r.
  // The result is within height 2 of balanced by the above
  // definition.
  static _RopeRep* _S_balance(_RopeRep* __r);

  // Add all unbalanced subtrees to the forest of balanceed trees.
  // Used only by balance.
  static void _S_add_to_forest(_RopeRep*__r, _RopeRep** __forest);
        
  // Add __r to forest, assuming __r is already balanced.
  static void _S_add_leaf_to_forest(_RopeRep* __r, _RopeRep** __forest);

  // Print to stdout, exposing structure
  static void _S_dump(_RopeRep* __r, int __indent = 0);

  // Return -1, 0, or 1 if __x < __y, __x == __y, or __x > __y resp.
  static int _S_compare(const _RopeRep* __x, const _RopeRep* __y);

public:
  bool empty() const { return 0 == _M_tree_ptr._M_data; }

  // Comparison member function.  This is public only for those
  // clients that need a ternary comparison.  Others
  // should use the comparison operators below.
  int compare(const _Self& __y) const {
    return _S_compare(_M_tree_ptr._M_data, __y._M_tree_ptr._M_data);
  }

  rope(const _CharT* __s, const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, _STLP_ROPE_FROM_UNOWNED_CHAR_PTR(__s, _S_char_ptr_len(__s),__a))
  { }

  rope(const _CharT* __s, size_t __len,
       const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, (_STLP_ROPE_FROM_UNOWNED_CHAR_PTR(__s, __len, __a)))
  { }

  // Should perhaps be templatized with respect to the iterator type
  // and use Sequence_buffer.  (It should perhaps use sequence_buffer
  // even now.)
  rope(const _CharT *__s, const _CharT *__e,
       const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, _STLP_ROPE_FROM_UNOWNED_CHAR_PTR(__s, __e - __s, __a))
  { }

  rope(const const_iterator& __s, const const_iterator& __e,
       const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, _S_substring(__s._M_root, __s._M_current_pos,
                                    __e._M_current_pos))
  { }

  rope(const iterator& __s, const iterator& __e,
       const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, _S_substring(__s._M_root, __s._M_current_pos,
                                    __e._M_current_pos))
  { }

  rope(_CharT __c, const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, (_RopeRep*)0)
  {
    _CharT* __buf = _M_tree_ptr.allocate(_S_rounded_up_size(1));

    _Construct(__buf, __c);
    _STLP_TRY {
      _M_tree_ptr._M_data = _S_new_RopeLeaf(__buf, 1, __a);
    }
    _STLP_UNWIND(_RopeRep::_S_free_string(__buf, 1, __a))
      }

  rope(size_t __n, _CharT __c,     
       const allocator_type& __a = allocator_type()):
    _M_tree_ptr(__a, (_RopeRep*)0) {
    rope<_CharT,_Alloc> __result;
# define  __exponentiate_threshold size_t(32)
    _RopeRep* __remainder;
    rope<_CharT,_Alloc> __remainder_rope;
	    
    // gcc-2.7.2 bugs
    typedef _Rope_Concat_fn<_CharT,_Alloc> _Concat_fn;
	    
    if (0 == __n)
      return;
	    
    size_t __exponent = __n / __exponentiate_threshold;
    size_t __rest = __n % __exponentiate_threshold;
    if (0 == __rest) {
      __remainder = 0;
    } else {
      _CharT* __rest_buffer = _M_tree_ptr.allocate(_S_rounded_up_size(__rest));
      uninitialized_fill_n(__rest_buffer, __rest, __c);
      _S_cond_store_eos(__rest_buffer[__rest]);
      _STLP_TRY {
		__remainder = _S_new_RopeLeaf(__rest_buffer, __rest, __a);
      }
      _STLP_UNWIND(_RopeRep::_S_free_string(__rest_buffer, __rest, __a))
		}
    __remainder_rope._M_tree_ptr._M_data = __remainder;
    if (__exponent != 0) {
      _CharT* __base_buffer =
		_M_tree_ptr.allocate(_S_rounded_up_size(__exponentiate_threshold));
      _RopeLeaf* __base_leaf;
      rope<_CharT,_Alloc> __base_rope;
      uninitialized_fill_n(__base_buffer, __exponentiate_threshold, __c);
      _S_cond_store_eos(__base_buffer[__exponentiate_threshold]);
      _STLP_TRY {
		__base_leaf = _S_new_RopeLeaf(__base_buffer,
                                      __exponentiate_threshold, __a);
      }
      _STLP_UNWIND(_RopeRep::_S_free_string(__base_buffer, 
                                            __exponentiate_threshold, __a))
		__base_rope._M_tree_ptr._M_data = __base_leaf;
      if (1 == __exponent) {
		__result = __base_rope;
#         ifndef __GC
		_STLP_ASSERT(2 == __result._M_tree_ptr._M_data->_M_ref_count)
		// One each for base_rope and __result
#         endif
      } else {
		__result = power(__base_rope, __exponent, _Concat_fn());
      }
      if (0 != __remainder) {
		__result += __remainder_rope;
      }
    } else {
      __result = __remainder_rope;
    }
    _M_tree_ptr._M_data = __result._M_tree_ptr._M_data;
    _M_tree_ptr._M_data->_M_ref_nonnil();
# undef __exponentiate_threshold
  }

  rope(const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, (_RopeRep*)0) {}

  // Construct a rope from a function that can compute its members
  rope(char_producer<_CharT> *__fn, size_t __len, bool __delete_fn,
       const allocator_type& __a = allocator_type())
    : _M_tree_ptr(__a, (_RopeRep*)0)
  {
    _M_tree_ptr._M_data = (0 == __len) ?
      0 : _S_new_RopeFunction(__fn, __len, __delete_fn, __a);
  }

  rope(const _Self& __x)
    : _M_tree_ptr(__x.get_allocator(), __x._M_tree_ptr._M_data)
  {
    _S_ref(_M_tree_ptr._M_data);
  }

  ~rope()
  {
    _S_unref(_M_tree_ptr._M_data);
  }

  _Self& operator=(const _Self& __x)
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _STLP_ASSERT(get_allocator() == __x.get_allocator())
    _M_tree_ptr._M_data = __x._M_tree_ptr._M_data;
    _S_ref(_M_tree_ptr._M_data);
    _S_unref(__old);
    return(*this);
  }
  void clear()
  {
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = 0;
  }
  void push_back(_CharT __x)
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _M_tree_ptr._M_data = _S_destr_concat_char_iter(_M_tree_ptr._M_data, &__x, 1);
    _S_unref(__old);
  }

  void pop_back()
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _M_tree_ptr._M_data = 
      _S_substring(_M_tree_ptr._M_data, 0, _M_tree_ptr._M_data->_M_size._M_data - 1);
    _S_unref(__old);
  }

  _CharT back() const
  {
    return _S_fetch(_M_tree_ptr._M_data, _M_tree_ptr._M_data->_M_size._M_data - 1);
  }

  void push_front(_CharT __x)
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _RopeRep* __left =
      _STLP_ROPE_FROM_UNOWNED_CHAR_PTR(&__x, 1, get_allocator());
    _STLP_TRY {
      _M_tree_ptr._M_data = _S_concat_rep(__left, _M_tree_ptr._M_data);
      _S_unref(__old);
      _S_unref(__left);
    }
    _STLP_UNWIND(_S_unref(__left))
      }

  void pop_front()
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _M_tree_ptr._M_data = _S_substring(_M_tree_ptr._M_data, 1, _M_tree_ptr._M_data->_M_size._M_data);
    _S_unref(__old);
  }

  _CharT front() const
  {
    return _S_fetch(_M_tree_ptr._M_data, 0);
  }

  void balance()
  {
    _RopeRep* __old = _M_tree_ptr._M_data;
    _M_tree_ptr._M_data = _S_balance(_M_tree_ptr._M_data);
    _S_unref(__old);
  }

  void copy(_CharT* __buffer) const {
    _Destroy(__buffer, __buffer + size());
    _S_flatten(_M_tree_ptr._M_data, __buffer);
  }

  // This is the copy function from the standard, but
  // with the arguments reordered to make it consistent with the
  // rest of the interface.
  // Note that this guaranteed not to compile if the draft standard
  // order is assumed.
  size_type copy(size_type __pos, size_type __n, _CharT* __buffer) const 
  {
    size_t _p_size = size();
    size_t __len = (__pos + __n > _p_size? _p_size - __pos : __n);

    _Destroy(__buffer, __buffer + __len);
    _S_flatten(_M_tree_ptr._M_data, __pos, __len, __buffer);
    return __len;
  }

  // Print to stdout, exposing structure.  May be useful for
  // performance debugging.
  void dump() {
    _S_dump(_M_tree_ptr._M_data);
  }

  // Convert to 0 terminated string in new allocated memory.
  // Embedded 0s in the input do not terminate the copy.
  const _CharT* c_str() const;

  // As above, but lso use the flattened representation as the
  // the new rope representation.
  const _CharT* replace_with_c_str();

  // Reclaim memory for the c_str generated flattened string.
  // Intentionally undocumented, since it's hard to say when this
  // is safe for multiple threads.
  void delete_c_str () {
    if (0 == _M_tree_ptr._M_data) return;
    if (_RopeRep::_S_leaf == _M_tree_ptr._M_data->_M_tag && 
        ((_RopeLeaf*)_M_tree_ptr._M_data)->_M_data == 
        _M_tree_ptr._M_data->_M_c_string) {
      // Representation shared
      return;
    }
#           ifndef __GC
    _M_tree_ptr._M_data->_M_free_c_string();
#           endif
    _M_tree_ptr._M_data->_M_c_string = 0;
  }

  _CharT operator[] (size_type __pos) const {
    return _S_fetch(_M_tree_ptr._M_data, __pos);
  }

  _CharT at(size_type __pos) const {
    // if (__pos >= size()) throw out_of_range;  // XXX
    return (*this)[__pos];
  }

  const_iterator begin() const {
    return(const_iterator(_M_tree_ptr._M_data, 0));
  }

  // An easy way to get a const iterator from a non-const container.
  const_iterator const_begin() const {
    return(const_iterator(_M_tree_ptr._M_data, 0));
  }

  const_iterator end() const {
    return(const_iterator(_M_tree_ptr._M_data, size()));
  }

  const_iterator const_end() const {
    return(const_iterator(_M_tree_ptr._M_data, size()));
  }

  size_type size() const { 
    return(0 == _M_tree_ptr._M_data? 0 : _M_tree_ptr._M_data->_M_size._M_data);
  }

  size_type length() const {
    return size();
  }

  size_type max_size() const {
    return _S_min_len[__ROPE_MAX_DEPTH-1] - 1;
    //  Guarantees that the result can be sufficirntly
    //  balanced.  Longer ropes will probably still work,
    //  but it's harder to make guarantees.
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator const_rbegin() const {
    return const_reverse_iterator(end());
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  const_reverse_iterator const_rend() const {
    return const_reverse_iterator(begin());
  }
  // The symmetric cases are intentionally omitted, since they're presumed
  // to be less common, and we don't handle them as well.

  // The following should really be templatized.
  // The first argument should be an input iterator or
  // forward iterator with value_type _CharT.
  _Self& append(const _CharT* __iter, size_t __n) {
    _RopeRep* __result = 
      _S_destr_concat_char_iter(_M_tree_ptr._M_data, __iter, __n);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
    return *this;
  }

  _Self& append(const _CharT* __c_string) {
    size_t __len = _S_char_ptr_len(__c_string);
    append(__c_string, __len);
    return(*this);
  }

  _Self& append(const _CharT* __s, const _CharT* __e) {
    _RopeRep* __result =
      _S_destr_concat_char_iter(_M_tree_ptr._M_data, __s, __e - __s);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
    return *this;
  }

  _Self& append(const_iterator __s, const_iterator __e) {
    _STLP_ASSERT(__s._M_root == __e._M_root)
    _STLP_ASSERT(get_allocator() == __s._M_root->get_allocator())
    _Self_destruct_ptr __appendee(_S_substring(
                                               __s._M_root, __s._M_current_pos, __e._M_current_pos));
    _RopeRep* __result = 
      _S_concat_rep(_M_tree_ptr._M_data, (_RopeRep*)__appendee);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
    return *this;
  }

  _Self& append(_CharT __c) {
    _RopeRep* __result = 
      _S_destr_concat_char_iter(_M_tree_ptr._M_data, &__c, 1);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
    return *this;
  }

  _Self& append() { return append(_CharT()); }  // XXX why?

  _Self& append(const _Self& __y) {
    _STLP_ASSERT(__y.get_allocator() == get_allocator())
    _RopeRep* __result = _S_concat_rep(_M_tree_ptr._M_data, __y._M_tree_ptr._M_data);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
    return *this;
  }

  _Self& append(size_t __n, _CharT __c) {
    rope<_CharT,_Alloc> __last(__n, __c);
    return append(__last);
  }

  void swap(_Self& __b) {
    _STLP_ASSERT(get_allocator() == __b.get_allocator())
    _RopeRep* __tmp = _M_tree_ptr._M_data;
    _M_tree_ptr._M_data = __b._M_tree_ptr._M_data;
    __b._M_tree_ptr._M_data = __tmp;
  }


protected:
  // Result is included in refcount.
  static _RopeRep* replace(_RopeRep* __old, size_t __pos1,
                           size_t __pos2, _RopeRep* __r) {
    if (0 == __old) { _S_ref(__r); return __r; }
    _Self_destruct_ptr __left(
                              _S_substring(__old, 0, __pos1));
    _Self_destruct_ptr __right(
                               _S_substring(__old, __pos2, __old->_M_size._M_data));
	_STLP_MPWFIX_TRY	//*TY 06/01/2000 - 
    _RopeRep* __result;

    if (0 == __r) {
      __result = _S_concat_rep(__left, __right);
    } else {
      _STLP_ASSERT(__old->get_allocator() == __r->get_allocator())
      _Self_destruct_ptr __left_result(_S_concat_rep(__left, __r));
      __result = _S_concat_rep(__left_result, __right);
    }
    return __result;
	_STLP_MPWFIX_CATCH	//*TY 06/01/2000 - 
  }

public:
  void insert(size_t __p, const _Self& __r) {
    _RopeRep* __result = 
      replace(_M_tree_ptr._M_data, __p, __p, __r._M_tree_ptr._M_data);
    _STLP_ASSERT(get_allocator() == __r.get_allocator())
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
  }

  void insert(size_t __p, size_t __n, _CharT __c) {
    rope<_CharT,_Alloc> __r(__n,__c);
    insert(__p, __r);
  }

  void insert(size_t __p, const _CharT* __i, size_t __n) {
    _Self_destruct_ptr __left(_S_substring(_M_tree_ptr._M_data, 0, __p));
    _Self_destruct_ptr __right(_S_substring(_M_tree_ptr._M_data, __p, size()));
    _Self_destruct_ptr __left_result(
                                     _S_concat_char_iter(__left, __i, __n));
    // _S_ destr_concat_char_iter should be safe here.
    // But as it stands it's probably not a win, since __left
    // is likely to have additional references.
    _RopeRep* __result = _S_concat_rep(__left_result, __right);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
  }

  void insert(size_t __p, const _CharT* __c_string) {
    insert(__p, __c_string, _S_char_ptr_len(__c_string));
  }

  void insert(size_t __p, _CharT __c) {
    insert(__p, &__c, 1);
  }

  void insert(size_t __p) {
    _CharT __c = _CharT();
    insert(__p, &__c, 1);
  }

  void insert(size_t __p, const _CharT* __i, const _CharT* __j) {
    _Self __r(__i, __j);
    insert(__p, __r);
  }

  void insert(size_t __p, const const_iterator& __i,
              const const_iterator& __j) {
    _Self __r(__i, __j);
    insert(__p, __r);
  }

  void insert(size_t __p, const iterator& __i,
              const iterator& __j) {
    _Self __r(__i, __j);
    insert(__p, __r);
  }

  // (position, length) versions of replace operations:

  void replace(size_t __p, size_t __n, const _Self& __r) {
    _RopeRep* __result = 
      replace(_M_tree_ptr._M_data, __p, __p + __n, __r._M_tree_ptr._M_data);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
  }

  void replace(size_t __p, size_t __n, 
               const _CharT* __i, size_t __i_len) {
    _Self __r(__i, __i_len);
    replace(__p, __n, __r);
  }

  void replace(size_t __p, size_t __n, _CharT __c) {
    _Self __r(__c);
    replace(__p, __n, __r);
  }

  void replace(size_t __p, size_t __n, const _CharT* __c_string) {
    _Self __r(__c_string);
    replace(__p, __n, __r);
  }

  void replace(size_t __p, size_t __n, 
               const _CharT* __i, const _CharT* __j) {
    _Self __r(__i, __j);
    replace(__p, __n, __r);
  }

  void replace(size_t __p, size_t __n,
               const const_iterator& __i, const const_iterator& __j) {
    _Self __r(__i, __j);
    replace(__p, __n, __r);
  }

  void replace(size_t __p, size_t __n,
               const iterator& __i, const iterator& __j) {
    _Self __r(__i, __j);
    replace(__p, __n, __r);
  }

  // Single character variants:
  void replace(size_t __p, _CharT __c) {
    iterator __i(this, __p);
    *__i = __c;
  }

  void replace(size_t __p, const _Self& __r) {
    replace(__p, 1, __r);
  }

  void replace(size_t __p, const _CharT* __i, size_t __i_len) {
    replace(__p, 1, __i, __i_len);
  }

  void replace(size_t __p, const _CharT* __c_string) {
    replace(__p, 1, __c_string);
  }

  void replace(size_t __p, const _CharT* __i, const _CharT* __j) {
    replace(__p, 1, __i, __j);
  }

  void replace(size_t __p, const const_iterator& __i,
               const const_iterator& __j) {
    replace(__p, 1, __i, __j);
  }

  void replace(size_t __p, const iterator& __i,
               const iterator& __j) {
    replace(__p, 1, __i, __j);
  }

  // Erase, (position, size) variant.
  void erase(size_t __p, size_t __n) {
    _RopeRep* __result = replace(_M_tree_ptr._M_data, __p, __p + __n, 0);
    _S_unref(_M_tree_ptr._M_data);
    _M_tree_ptr._M_data = __result;
  }

  // Erase, single character
  void erase(size_t __p) {
    erase(__p, __p + 1);
  }

  // Insert, iterator variants.  
  iterator insert(const iterator& __p, const _Self& __r)
  { insert(__p.index(), __r); return __p; }
  iterator insert(const iterator& __p, size_t __n, _CharT __c)
  { insert(__p.index(), __n, __c); return __p; }
  iterator insert(const iterator& __p, _CharT __c) 
  { insert(__p.index(), __c); return __p; }
  iterator insert(const iterator& __p ) 
  { insert(__p.index()); return __p; }
  iterator insert(const iterator& __p, const _CharT* c_string) 
  { insert(__p.index(), c_string); return __p; }
  iterator insert(const iterator& __p, const _CharT* __i, size_t __n)
  { insert(__p.index(), __i, __n); return __p; }
  iterator insert(const iterator& __p, const _CharT* __i, 
                  const _CharT* __j)
  { insert(__p.index(), __i, __j);  return __p; }
  iterator insert(const iterator& __p,
                  const const_iterator& __i, const const_iterator& __j)
  { insert(__p.index(), __i, __j); return __p; }
  iterator insert(const iterator& __p,
                  const iterator& __i, const iterator& __j)
  { insert(__p.index(), __i, __j); return __p; }

  // Replace, range variants.
  void replace(const iterator& __p, const iterator& __q,
               const _Self& __r)
  { replace(__p.index(), __q.index() - __p.index(), __r); }
  void replace(const iterator& __p, const iterator& __q, _CharT __c)
  { replace(__p.index(), __q.index() - __p.index(), __c); }
  void replace(const iterator& __p, const iterator& __q,
               const _CharT* __c_string)
  { replace(__p.index(), __q.index() - __p.index(), __c_string); }
  void replace(const iterator& __p, const iterator& __q,
               const _CharT* __i, size_t __n)
  { replace(__p.index(), __q.index() - __p.index(), __i, __n); }
  void replace(const iterator& __p, const iterator& __q,
               const _CharT* __i, const _CharT* __j)
  { replace(__p.index(), __q.index() - __p.index(), __i, __j); }
  void replace(const iterator& __p, const iterator& __q,
               const const_iterator& __i, const const_iterator& __j)
  { replace(__p.index(), __q.index() - __p.index(), __i, __j); }
  void replace(const iterator& __p, const iterator& __q,
               const iterator& __i, const iterator& __j)
  { replace(__p.index(), __q.index() - __p.index(), __i, __j); }

  // Replace, iterator variants.
  void replace(const iterator& __p, const _Self& __r)
  { replace(__p.index(), __r); }
  void replace(const iterator& __p, _CharT __c)
  { replace(__p.index(), __c); }
  void replace(const iterator& __p, const _CharT* __c_string)
  { replace(__p.index(), __c_string); }
  void replace(const iterator& __p, const _CharT* __i, size_t __n)
  { replace(__p.index(), __i, __n); }
  void replace(const iterator& __p, const _CharT* __i, const _CharT* __j)
  { replace(__p.index(), __i, __j); }
  void replace(const iterator& __p, const_iterator __i, 
               const_iterator __j)
  { replace(__p.index(), __i, __j); }
  void replace(const iterator& __p, iterator __i, iterator __j)
  { replace(__p.index(), __i, __j); }

  // Iterator and range variants of erase
  iterator erase(const iterator& __p, const iterator& __q) {
    size_t __p_index = __p.index();
    erase(__p_index, __q.index() - __p_index);
    return iterator(this, __p_index);
  }
  iterator erase(const iterator& __p) {
    size_t __p_index = __p.index();
    erase(__p_index, 1);
    return iterator(this, __p_index);
  }

  _Self substr(size_t __start, size_t __len = 1) const {
    return rope<_CharT,_Alloc>(
                               _S_substring(_M_tree_ptr._M_data, __start, __start + __len));
  }

  _Self substr(iterator __start, iterator __end) const {
    return rope<_CharT,_Alloc>(
                               _S_substring(_M_tree_ptr._M_data, __start.index(), __end.index()));
  }
        
  _Self substr(iterator __start) const {
    size_t __pos = __start.index();
    return rope<_CharT,_Alloc>(
                               _S_substring(_M_tree_ptr._M_data, __pos, __pos + 1));
  }
        
  _Self substr(const_iterator __start, const_iterator __end) const {
    // This might eventually take advantage of the cache in the
    // iterator.
    return rope<_CharT,_Alloc>(
                               _S_substring(_M_tree_ptr._M_data, __start.index(), __end.index()));
  }

  rope<_CharT,_Alloc> substr(const_iterator __start) {
    size_t __pos = __start.index();
    return rope<_CharT,_Alloc>(
                               _S_substring(_M_tree_ptr._M_data, __pos, __pos + 1));
  }

  enum { npos = -1 };

  //         static const size_type npos;

  size_type find(_CharT __c, size_type __pos = 0) const;
  size_type find(const _CharT* __s, size_type __pos = 0) const {
    size_type __result_pos;
    const_iterator __result = search(const_begin() + (ptrdiff_t)__pos, const_end(),
                                     __s, __s + _S_char_ptr_len(__s));
    __result_pos = __result.index();
#           ifndef _STLP_OLD_ROPE_SEMANTICS
    if (__result_pos == size()) __result_pos = npos;
#           endif
    return __result_pos;
  }

  iterator mutable_begin() {
    return(iterator(this, 0));
  }

  iterator mutable_end() {
    return(iterator(this, size()));
  }

  reverse_iterator mutable_rbegin() {
    return reverse_iterator(mutable_end());
  }

  reverse_iterator mutable_rend() {
    return reverse_iterator(mutable_begin());
  }

  reference mutable_reference_at(size_type __pos) {
    return reference(this, __pos);
  }

#       ifdef __STD_STUFF
  reference operator[] (size_type __pos) {
    return reference(this, __pos);
  }

  reference at(size_type __pos) {
    // if (__pos >= size()) throw out_of_range;  // XXX
    return (*this)[__pos];
  }

  void resize(size_type, _CharT) {}
  void resize(size_type) {}
  void reserve(size_type = 0) {}
  size_type capacity() const {
    return max_size();
  }

  // Stuff below this line is dangerous because it's error prone.
  // I would really like to get rid of it.
  // copy function with funny arg ordering.
  size_type copy(_CharT* __buffer, size_type __n, 
                 size_type __pos = 0) const {
    return copy(__pos, __n, __buffer);
  }

  iterator end() { return mutable_end(); }

  iterator begin() { return mutable_begin(); }

  reverse_iterator rend() { return mutable_rend(); }

  reverse_iterator rbegin() { return mutable_rbegin(); }

#       else

  const_iterator end() { return const_end(); }

  const_iterator begin() { return const_begin(); }

  const_reverse_iterator rend() { return const_rend(); }
  
  const_reverse_iterator rbegin() { return const_rbegin(); }

#	endif

  __ROPE_DEFINE_ALLOCS(_Alloc, _M_tree_ptr)
    };

# undef __ROPE_DEFINE_ALLOC
# undef __ROPE_DEFINE_ALLOCS

template <class _CharT, class _Alloc>
inline _CharT 
_Rope_const_iterator< _CharT, _Alloc>::operator[](size_t __n)
{
  return rope<_CharT,_Alloc>::_S_fetch(this->_M_root, this->_M_current_pos + __n);
}

template <class _CharT, class _Alloc>
inline bool operator== (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                        const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return (__x._M_current_pos == __y._M_current_pos && 
          __x._M_root == __y._M_root);
}

template <class _CharT, class _Alloc>
inline bool operator< (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                       const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return (__x._M_current_pos < __y._M_current_pos);
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _CharT, class _Alloc>
inline bool operator!= (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                        const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _CharT, class _Alloc>
inline bool operator> (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                       const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return __y < __x;
}

template <class _CharT, class _Alloc>
inline bool operator<= (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                        const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _CharT, class _Alloc>
inline bool operator>= (const _Rope_const_iterator<_CharT,_Alloc>& __x,
                        const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return !(__x < __y);
}

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

template <class _CharT, class _Alloc>
inline ptrdiff_t operator-(const _Rope_const_iterator<_CharT,_Alloc>& __x,
                           const _Rope_const_iterator<_CharT,_Alloc>& __y) {
  return (ptrdiff_t)__x._M_current_pos - (ptrdiff_t)__y._M_current_pos;
}

#if !defined( __MWERKS__ ) || __MWERKS__ >= 0x2000		// dwa 8/21/97  - "ambiguous access to overloaded function" bug.
template <class _CharT, class _Alloc>
inline _Rope_const_iterator<_CharT,_Alloc>
operator-(const _Rope_const_iterator<_CharT,_Alloc>& __x, ptrdiff_t __n) {
  return _Rope_const_iterator<_CharT,_Alloc>(
                                             __x._M_root, __x._M_current_pos - __n);
}
# endif

template <class _CharT, class _Alloc>
inline _Rope_const_iterator<_CharT,_Alloc>
operator+(const _Rope_const_iterator<_CharT,_Alloc>& __x, ptrdiff_t __n) {
  return _Rope_const_iterator<_CharT,_Alloc>(
                                             __x._M_root, __x._M_current_pos + __n);
}

template <class _CharT, class _Alloc>
inline _Rope_const_iterator<_CharT,_Alloc>
operator+(ptrdiff_t __n, const _Rope_const_iterator<_CharT,_Alloc>& __x) {
  return _Rope_const_iterator<_CharT,_Alloc>(
                                             __x._M_root, __x._M_current_pos + __n);
}

template <class _CharT, class _Alloc>
inline bool operator== (const _Rope_iterator<_CharT,_Alloc>& __x,
                        const _Rope_iterator<_CharT,_Alloc>& __y) {
  return (__x._M_current_pos == __y._M_current_pos && 
          __x._M_root_rope == __y._M_root_rope);
}

template <class _CharT, class _Alloc>
inline bool operator< (const _Rope_iterator<_CharT,_Alloc>& __x,
                       const _Rope_iterator<_CharT,_Alloc>& __y) {
  return (__x._M_current_pos < __y._M_current_pos);
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _CharT, class _Alloc>
inline bool operator!= (const _Rope_iterator<_CharT,_Alloc>& __x,
                        const _Rope_iterator<_CharT,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _CharT, class _Alloc>
inline bool operator> (const _Rope_iterator<_CharT,_Alloc>& __x,
                       const _Rope_iterator<_CharT,_Alloc>& __y) {
  return __y < __x;
}

template <class _CharT, class _Alloc>
inline bool operator<= (const _Rope_iterator<_CharT,_Alloc>& __x,
                        const _Rope_iterator<_CharT,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _CharT, class _Alloc>
inline bool operator>= (const _Rope_iterator<_CharT,_Alloc>& __x,
                        const _Rope_iterator<_CharT,_Alloc>& __y) {
  return !(__x < __y);
}

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

template <class _CharT, class _Alloc>
inline ptrdiff_t operator-(const _Rope_iterator<_CharT,_Alloc>& __x,
                           const _Rope_iterator<_CharT,_Alloc>& __y) {
  return (ptrdiff_t)__x._M_current_pos - (ptrdiff_t)__y._M_current_pos;
}

#if !defined( __MWERKS__ ) || __MWERKS__ >= 0x2000		// dwa 8/21/97  - "ambiguous access to overloaded function" bug.
template <class _CharT, class _Alloc>
inline _Rope_iterator<_CharT,_Alloc>
operator-(const _Rope_iterator<_CharT,_Alloc>& __x,
          ptrdiff_t __n) {
  return _Rope_iterator<_CharT,_Alloc>(
                                       __x._M_root_rope, __x._M_current_pos - __n);
}
# endif

template <class _CharT, class _Alloc>
inline _Rope_iterator<_CharT,_Alloc>
operator+(const _Rope_iterator<_CharT,_Alloc>& __x,
          ptrdiff_t __n) {
  return _Rope_iterator<_CharT,_Alloc>(
                                       __x._M_root_rope, __x._M_current_pos + __n);
}

template <class _CharT, class _Alloc>
inline _Rope_iterator<_CharT,_Alloc>
operator+(ptrdiff_t __n, const _Rope_iterator<_CharT,_Alloc>& __x) {
  return _Rope_iterator<_CharT,_Alloc>(
                                       __x._M_root_rope, __x._M_current_pos + __n);
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>
operator+ (const rope<_CharT,_Alloc>& __left,
           const rope<_CharT,_Alloc>& __right)
{
  _STLP_ASSERT(__left.get_allocator() == __right.get_allocator())
  return rope<_CharT,_Alloc>(rope<_CharT,_Alloc>::_S_concat_rep(__left._M_tree_ptr._M_data, __right._M_tree_ptr._M_data));
  // Inlining this should make it possible to keep __left and
  // __right in registers.
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>&
operator+= (rope<_CharT,_Alloc>& __left, 
            const rope<_CharT,_Alloc>& __right)
{
  __left.append(__right);
  return __left;
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>
operator+ (const rope<_CharT,_Alloc>& __left,
           const _CharT* __right) {
  size_t __rlen = rope<_CharT,_Alloc>::_S_char_ptr_len(__right);
  return rope<_CharT,_Alloc>(
                             rope<_CharT,_Alloc>::_S_concat_char_iter(
                                                                      __left._M_tree_ptr._M_data, __right, __rlen)); 
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>&
operator+= (rope<_CharT,_Alloc>& __left,
            const _CharT* __right) {
  __left.append(__right);
  return __left;
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>
operator+ (const rope<_CharT,_Alloc>& __left, _STLP_SIMPLE_TYPE(_CharT) __right) {
  return rope<_CharT,_Alloc>(
                             rope<_CharT,_Alloc>::_S_concat_char_iter(
                                                                      __left._M_tree_ptr._M_data, &__right, 1));
}

template <class _CharT, class _Alloc>
inline
rope<_CharT,_Alloc>&
operator+= (rope<_CharT,_Alloc>& __left, _STLP_SIMPLE_TYPE(_CharT) __right) {
  __left.append(__right);
  return __left;
}

template <class _CharT, class _Alloc>
inline bool
operator< (const rope<_CharT,_Alloc>& __left, 
           const rope<_CharT,_Alloc>& __right) {
  return __left.compare(__right) < 0;
}
        
template <class _CharT, class _Alloc>
inline bool
operator== (const rope<_CharT,_Alloc>& __left, 
            const rope<_CharT,_Alloc>& __right) {
  return __left.compare(__right) == 0;
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _CharT, class _Alloc>
inline bool
operator!= (const rope<_CharT,_Alloc>& __x, const rope<_CharT,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _CharT, class _Alloc>
inline bool
operator> (const rope<_CharT,_Alloc>& __x, const rope<_CharT,_Alloc>& __y) {
  return __y < __x;
}

template <class _CharT, class _Alloc>
inline bool
operator<= (const rope<_CharT,_Alloc>& __x, const rope<_CharT,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _CharT, class _Alloc>
inline bool
operator>= (const rope<_CharT,_Alloc>& __x, const rope<_CharT,_Alloc>& __y) {
  return !(__x < __y);
}

template <class _CharT, class _Alloc>
inline bool operator!= (const _Rope_char_ptr_proxy<_CharT,_Alloc>& __x,
                        const _Rope_char_ptr_proxy<_CharT,_Alloc>& __y) {
  return !(__x == __y);
}

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

template <class _CharT, class _Alloc>
inline bool operator== (const _Rope_char_ptr_proxy<_CharT,_Alloc>& __x,
                        const _Rope_char_ptr_proxy<_CharT,_Alloc>& __y) {
  return (__x._M_pos == __y._M_pos && __x._M_root == __y._M_root);
}

#ifdef _STLP_USE_NEW_IOSTREAMS
template<class _CharT, class _Traits, class _Alloc>
basic_ostream<_CharT, _Traits>& operator<< (
                                            basic_ostream<_CharT, _Traits>& __o,
                                            const rope<_CharT, _Alloc>& __r);
#elif ! defined (_STLP_USE_NO_IOSTREAMS)
template<class _CharT, class _Alloc>
ostream& operator<< (ostream& __o, const rope<_CharT,_Alloc>& __r);        
#endif
        
typedef rope<char, _STLP_DEFAULT_ALLOCATOR(char) > crope;
# ifdef _STLP_HAS_WCHAR_T
typedef rope<wchar_t, _STLP_DEFAULT_ALLOCATOR(wchar_t) > wrope;
# endif

inline crope::reference __mutable_reference_at(crope& __c, size_t __i)
{
  return __c.mutable_reference_at(__i);
}

# ifdef _STLP_HAS_WCHAR_T
inline wrope::reference __mutable_reference_at(wrope& __c, size_t __i)
{
  return __c.mutable_reference_at(__i);
}
# endif

#ifdef _STLP_FUNCTION_TMPL_PARTIAL_ORDER

template <class _CharT, class _Alloc>
inline void swap(rope<_CharT,_Alloc>& __x, rope<_CharT,_Alloc>& __y) {
  __x.swap(__y);
}
#else

inline void swap(crope& __x, crope& __y) { __x.swap(__y); }
# ifdef _STLP_HAS_WCHAR_T	// dwa 8/21/97
inline void swap(wrope& __x, wrope& __y) { __x.swap(__y); }
# endif

#endif /* _STLP_FUNCTION_TMPL_PARTIAL_ORDER */


// Hash functions should probably be revisited later:
_STLP_TEMPLATE_NULL struct hash<crope>
{
  size_t operator()(const crope& __str) const
  {
    size_t _p_size = __str.size();

    if (0 == _p_size) return 0;
    return 13*__str[0] + 5*__str[_p_size - 1] + _p_size;
  }
};

# ifdef _STLP_HAS_WCHAR_T	// dwa 8/21/97
_STLP_TEMPLATE_NULL struct hash<wrope>
{
  size_t operator()(const wrope& __str) const
  {
    size_t _p_size = __str.size();

    if (0 == _p_size) return 0;
    return 13*__str[0] + 5*__str[_p_size - 1] + _p_size;
  }
};
#endif

#ifndef _STLP_MSVC
// I couldn't get this to work with VC++
template<class _CharT,class _Alloc>
void
_Rope_rotate(_Rope_iterator<_CharT,_Alloc> __first,
             _Rope_iterator<_CharT,_Alloc> __middle,
             _Rope_iterator<_CharT,_Alloc> __last);

#if !defined(__GNUC__)
// Appears to confuse g++
inline void rotate(_Rope_iterator<char,_STLP_DEFAULT_ALLOCATOR(char) > __first,
                   _Rope_iterator<char,_STLP_DEFAULT_ALLOCATOR(char) > __middle,
                   _Rope_iterator<char,_STLP_DEFAULT_ALLOCATOR(char) > __last) {
  _Rope_rotate(__first, __middle, __last);
}
#endif

#endif

template <class _CharT, class _Alloc>
inline _Rope_char_ref_proxy<_CharT, _Alloc>::operator _CharT () const
{
  if (_M_current_valid) {
	return _M_current;
  } else {
    return _My_rope::_S_fetch(_M_root->_M_tree_ptr._M_data, _M_pos);
  }
}
_STLP_END_NAMESPACE

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_rope.c>
# endif

# endif /* _STLP_INTERNAL_ROPE_H */

// Local Variables:
// mode:C++
// End:
