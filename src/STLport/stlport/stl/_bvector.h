/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
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

#ifndef _STLP_INTERNAL_BVECTOR_H
#define _STLP_INTERNAL_BVECTOR_H

#ifndef _STLP_INTERNAL_VECTOR_H
# include <stl/_vector.h>
# endif

#define __WORD_BIT (int(CHAR_BIT*sizeof(unsigned int)))

_STLP_BEGIN_NAMESPACE 

struct _Bit_reference {
  unsigned int* _M_p;
  unsigned int _M_mask;
  _Bit_reference(unsigned int* __x, unsigned int __y) 
    : _M_p(__x), _M_mask(__y) {}

public:
  _Bit_reference() : _M_p(0), _M_mask(0) {}
  operator bool() const { return !(!(*_M_p & _M_mask)); }
  _Bit_reference& operator=(bool __x)
  {
    if (__x)  *_M_p |= _M_mask;
    else      *_M_p &= ~_M_mask;
    return *this;
  }
  _Bit_reference& operator=(const _Bit_reference& __x)
    { return *this = bool(__x); }
  bool operator==(const _Bit_reference& __x) const
    { return bool(*this) == bool(__x); }
  bool operator<(const _Bit_reference& __x) const {
    return !bool(*this) && bool(__x);
  }
  void flip() { *_M_p ^= _M_mask; }
};

inline void swap(_Bit_reference __x, _Bit_reference& __y)
{
  bool __tmp = (bool)__x;
  __x = __y;
  __y = __tmp;
}

struct _Bit_iterator_base;

struct _Bit_iterator_base
{
  typedef ptrdiff_t difference_type;

  unsigned int* _M_p;
  unsigned int  _M_offset;

  void _M_bump_up() {
    if (_M_offset++ == __WORD_BIT - 1) {
      _M_offset = 0;
      ++_M_p;
    }
  }

  void _M_bump_down() {
    if (_M_offset-- == 0) {
      _M_offset = __WORD_BIT - 1;
      --_M_p;
    }
  }

  _Bit_iterator_base() : _M_p(0), _M_offset(0) {}
  _Bit_iterator_base(unsigned int* __x, unsigned int __y) : _M_p(__x), _M_offset(__y) {}
  //  _Bit_iterator_base( const _Bit_iterator_base& __x) : _M_p(__x._M_p), _M_offset(__x._M_offset) {}
  //  _Bit_iterator_base& operator = ( const _Bit_iterator_base& __x) { _M_p = __x._M_p ; _M_offset = __x._M_offset ; return *this; }

  void _M_advance (difference_type __i) {
    difference_type __n = __i + _M_offset;
    _M_p += __n / __WORD_BIT;
    __n = __n % __WORD_BIT;
    if (__n < 0) {
      _M_offset = (unsigned int) __n + __WORD_BIT;
      --_M_p;
    } else
      _M_offset = (unsigned int) __n;
  }

  difference_type _M_subtract(const _Bit_iterator_base& __x) const {
    return __WORD_BIT * (_M_p - __x._M_p) + _M_offset - __x._M_offset;
  }
};

inline bool  _STLP_CALL operator==(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y) {
  return __y._M_p == __x._M_p && __y._M_offset == __x._M_offset;
}
inline bool  _STLP_CALL operator!=(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y) {
  return __y._M_p != __x._M_p || __y._M_offset != __x._M_offset;
}

inline bool _STLP_CALL operator<(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y) {
  return __x._M_p < __y._M_p || (__x._M_p == __y._M_p && __x._M_offset < __y._M_offset);
}

inline bool _STLP_CALL operator>(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y)  { 
  return operator <(__y , __x); 
}
inline bool _STLP_CALL operator<=(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y) { 
  return !(__y < __x); 
}
inline bool _STLP_CALL operator>=(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y) { 
  return !(__x < __y); 
}

template <class _Ref, class _Ptr>
struct _Bit_iter : public _Bit_iterator_base
{
  typedef _Ref  reference;
  typedef _Ptr  pointer;
  typedef _Bit_iter<_Ref, _Ptr> _Self;
  typedef random_access_iterator_tag iterator_category;
  typedef bool value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;

  _Bit_iter(unsigned int* __x, unsigned int __y) : _Bit_iterator_base(__x, __y) {}
  _Bit_iter() {}

  _Bit_iter(const _Bit_iter<_Bit_reference, _Bit_reference*>& __x): 
    _Bit_iterator_base((const _Bit_iterator_base&)__x) {}

  //  _Self& operator = (const _Bit_iter<_Bit_reference, _Bit_reference*>& __x)
  //   { (_Bit_iterator_base&)*this = (const _Bit_iterator_base&)__x; return *this; }

  reference operator*() const { 
    return _Bit_reference(_M_p, 1UL << _M_offset); 
  }
  _Self& operator++() {
    _M_bump_up();
    return *this;
  }
  _Self operator++(int) {
    _Self __tmp = *this;
    _M_bump_up();
    return __tmp;
  }
  _Self& operator--() {
    _M_bump_down();
    return *this;
  }
  _Self operator--(int) {
    _Self __tmp = *this;
    _M_bump_down();
    return __tmp;
  }
  _Self& operator+=(difference_type __i) {
    _M_advance(__i);
    return *this;
  }
  _Self& operator-=(difference_type __i) {
    *this += -__i;
    return *this;
  }
  _Self operator+(difference_type __i) const {
    _Self __tmp = *this;
    return __tmp += __i;
  }
  _Self operator-(difference_type __i) const {
    _Self __tmp = *this;
    return __tmp -= __i;
  }
  difference_type operator-(const _Self& __x) const {
    return _M_subtract(__x);
  }
  reference operator[](difference_type __i) { return *(*this + __i); }
};

template <class _Ref, class _Ptr>
inline _Bit_iter<_Ref,_Ptr>  _STLP_CALL
operator+(ptrdiff_t __n, const _Bit_iter<_Ref, _Ptr>& __x) {
   return __x + __n;
}

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
inline random_access_iterator_tag  iterator_category(const _Bit_iterator_base&) {return random_access_iterator_tag();}
inline ptrdiff_t* distance_type(const _Bit_iterator_base&) {return (ptrdiff_t*)0;}
inline bool* value_type(const _Bit_iter<_Bit_reference, _Bit_reference*>&) {return (bool*)0;}
inline bool* value_type(const _Bit_iter<bool, const bool*>&) {return (bool*)0;}
# endif

typedef _Bit_iter<bool, const bool*> _Bit_const_iterator;
typedef _Bit_iter<_Bit_reference, _Bit_reference*> _Bit_iterator;

// Bit-vector base class, which encapsulates the difference between
//  old SGI-style allocators and standard-conforming allocators.


template <class _Alloc>
class _Bvector_base
{
public:
  _STLP_FORCE_ALLOCATORS(bool, _Alloc)
  typedef typename _Alloc_traits<bool, _Alloc>::allocator_type allocator_type;
  typedef unsigned int __chunk_type;
  typedef typename _Alloc_traits<__chunk_type, 
          _Alloc>::allocator_type __chunk_allocator_type;
  allocator_type get_allocator() const { 
    return _STLP_CONVERT_ALLOCATOR((const __chunk_allocator_type&)_M_end_of_storage, bool); 
  }
  static allocator_type __get_dfl_allocator() { return allocator_type(); }
   
  _Bvector_base(const allocator_type& __a)
    : _M_start(), _M_finish(), _M_end_of_storage(_STLP_CONVERT_ALLOCATOR(__a, __chunk_type),
						 (__chunk_type*)0) {
  }
  ~_Bvector_base() { _M_deallocate();
  }

protected:

  unsigned int* _M_bit_alloc(size_t __n) 
    { return _M_end_of_storage.allocate((__n + __WORD_BIT - 1)/__WORD_BIT); }
  void _M_deallocate() {
    if (_M_start._M_p)
      _M_end_of_storage.deallocate(_M_start._M_p,
				   _M_end_of_storage._M_data - _M_start._M_p);
  }

  _Bit_iterator _M_start;
  _Bit_iterator _M_finish;
  _STLP_alloc_proxy<__chunk_type*, __chunk_type, __chunk_allocator_type> _M_end_of_storage;  
};


// The next few lines are confusing.  What we're doing is declaring a
//  partial specialization of vector<T, Alloc> if we have the necessary
//  compiler support.  Otherwise, we define a class bit_vector which uses
//  the default allocator. 

#if defined(_STLP_CLASS_PARTIAL_SPECIALIZATION) && ! defined(_STLP_NO_BOOL) && ! defined (__SUNPRO_CC)
# define _STLP_VECBOOL_TEMPLATE
# define __BVEC_TMPL_HEADER template <class _Alloc>
#else
# undef _STLP_VECBOOL_TEMPLATE
# ifdef _STLP_NO_BOOL
#  define __BVEC_TMPL_HEADER
# else
#  define __BVEC_TMPL_HEADER _STLP_TEMPLATE_NULL
# endif
# if !(defined(__MRC__)||defined(__SC__))			//*TY 12/17/2000 - 
#  define _Alloc _STLP_DEFAULT_ALLOCATOR(bool)
# else
#  define _Alloc allocator<bool>
# endif
#endif

#ifdef _STLP_NO_BOOL
# define __BVECTOR_QUALIFIED bit_vector
# define __BVECTOR           bit_vector
#else
# ifdef _STLP_VECBOOL_TEMPLATE
#  define __BVECTOR_QUALIFIED __WORKAROUND_DBG_RENAME(vector) <bool, _Alloc>
# else
#  define __BVECTOR_QUALIFIED __WORKAROUND_DBG_RENAME(vector) <bool, allocator<bool> >
# endif
#if defined (_STLP_PARTIAL_SPEC_NEEDS_TEMPLATE_ARGS)
# define __BVECTOR __BVECTOR_QUALIFIED
#else
# define __BVECTOR __WORKAROUND_DBG_RENAME(vector)
#endif
#endif


__BVEC_TMPL_HEADER
class __BVECTOR_QUALIFIED : public _Bvector_base<_Alloc >
{
  typedef _Bvector_base<_Alloc > _Base;
  typedef __BVECTOR_QUALIFIED _Self;
public:
  typedef bool value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type; 
  typedef _Bit_reference reference;
  typedef bool const_reference;
  typedef _Bit_reference* pointer;
  typedef const bool* const_pointer;
  typedef random_access_iterator_tag _Iterator_category;

  typedef _Bit_iterator                iterator;
  typedef _Bit_const_iterator          const_iterator;

#if defined ( _STLP_CLASS_PARTIAL_SPECIALIZATION )
  typedef _STLP_STD::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef _STLP_STD::reverse_iterator<iterator> reverse_iterator;
#else /* _STLP_CLASS_PARTIAL_SPECIALIZATION */
# if defined (_STLP_MSVC50_COMPATIBILITY)
  typedef _STLP_STD::reverse_iterator<const_iterator, value_type, const_reference, 
  const_pointer, difference_type> const_reverse_iterator;
  typedef _STLP_STD::reverse_iterator<iterator, value_type, reference, reference*, 
  difference_type> reverse_iterator;
# else
  typedef _STLP_STD::reverse_iterator<const_iterator, value_type, const_reference, 
                                  difference_type> const_reverse_iterator;
  typedef _STLP_STD::reverse_iterator<iterator, value_type, reference, difference_type>
          reverse_iterator;
# endif
#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

# ifdef _STLP_VECBOOL_TEMPLATE
  typedef typename _Bvector_base<_Alloc >::allocator_type allocator_type;
  typedef typename _Bvector_base<_Alloc >::__chunk_type __chunk_type ;
# else
  typedef _Bvector_base<_Alloc >::allocator_type allocator_type;
  typedef _Bvector_base<_Alloc >::__chunk_type __chunk_type ;
# endif

protected:

  void _M_initialize(size_type __n) {
    unsigned int* __q = this->_M_bit_alloc(__n);
    this->_M_end_of_storage._M_data = __q + (__n + __WORD_BIT - 1)/__WORD_BIT;
    this->_M_start = iterator(__q, 0);
    this->_M_finish = this->_M_start + difference_type(__n);
  }
  void _M_insert_aux(iterator __position, bool __x) {
    if (this->_M_finish._M_p != this->_M_end_of_storage._M_data) {
      __copy_backward(__position, this->_M_finish, this->_M_finish + 1, random_access_iterator_tag(), (difference_type*)0 );
      *__position = __x;
      ++this->_M_finish;
    }
    else {
      size_type __len = size() ? 2 * size() : __WORD_BIT;
      unsigned int* __q = this->_M_bit_alloc(__len);
      iterator __i = copy(begin(), __position, iterator(__q, 0));
      *__i++ = __x;
      this->_M_finish = copy(__position, end(), __i);
      this->_M_deallocate();
      this->_M_end_of_storage._M_data = __q + (__len + __WORD_BIT - 1)/__WORD_BIT;
      this->_M_start = iterator(__q, 0);
    }
  }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void _M_initialize_range(_InputIterator __first, _InputIterator __last,
			const input_iterator_tag &) {
    this->_M_start = iterator();
    this->_M_finish = iterator();
    this->_M_end_of_storage._M_data = 0;
    for ( ; __first != __last; ++__first) 
      push_back(*__first);
  }

  template <class _ForwardIterator>
  void _M_initialize_range(_ForwardIterator __first, _ForwardIterator __last,
                           const forward_iterator_tag &) {
    size_type __n = distance(__first, __last);
    _M_initialize(__n);
    //    copy(__first, __last, _M_start);
    copy(__first, __last, this->_M_start); // dwa 12/22/99 -- resolving ambiguous reference.
  }

  template <class _InputIterator>
  void _M_insert_range(iterator __pos,
                       _InputIterator __first, _InputIterator __last,
                       const input_iterator_tag &) {
    for ( ; __first != __last; ++__first) {
      __pos = insert(__pos, *__first);
      ++__pos;
    }
  }

  template <class _ForwardIterator>
  void _M_insert_range(iterator __position,
                       _ForwardIterator __first, _ForwardIterator __last,
                       const forward_iterator_tag &) {
    if (__first != __last) {
      size_type __n = distance(__first, __last);
      if (capacity() - size() >= __n) {
        __copy_backward(__position, end(), this->_M_finish + difference_type(__n), random_access_iterator_tag(), (difference_type*)0 );
        copy(__first, __last, __position);
        this->_M_finish += difference_type(__n);
      }
      else {
        size_type __len = size() + (max)(size(), __n);
        unsigned int* __q = this->_M_bit_alloc(__len);
        iterator __i = copy(begin(), __position, iterator(__q, 0));
        __i = copy(__first, __last, __i);
        this->_M_finish = copy(__position, end(), __i);
        this->_M_deallocate();
        this->_M_end_of_storage._M_data = __q + (__len + __WORD_BIT - 1)/__WORD_BIT;
        this->_M_start = iterator(__q, 0);
      }
    }
  }      

#endif /* _STLP_MEMBER_TEMPLATES */

public:
  iterator begin() { return this->_M_start; }
  const_iterator begin() const { return this->_M_start; }
  iterator end() { return this->_M_finish; }
  const_iterator end() const { return this->_M_finish; }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { 
    return const_reverse_iterator(end()); 
  }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { 
    return const_reverse_iterator(begin()); 
  }

  size_type size() const { return size_type(end() - begin()); }
  size_type max_size() const { return size_type(-1); }
  size_type capacity() const {
    return size_type(const_iterator(this->_M_end_of_storage._M_data, 0) - begin());
  }
  bool empty() const { return begin() == end(); }
  reference operator[](size_type __n) 
  { return *(begin() + difference_type(__n)); }
  const_reference operator[](size_type __n) const 
  { return *(begin() + difference_type(__n)); }

  void _M_range_check(size_type __n) const {
    if (__n >= this->size())
      __stl_throw_range_error("vector<bool>");
  }

  reference at(size_type __n)
    { _M_range_check(__n); return (*this)[__n]; }
  const_reference at(size_type __n) const
    { _M_range_check(__n); return (*this)[__n]; }

  explicit __BVECTOR(const allocator_type& __a = allocator_type())
    : _Bvector_base<_Alloc >(__a) {}

  __BVECTOR(size_type __n, bool __val,
            const allocator_type& __a = 
	    allocator_type())
    : _Bvector_base<_Alloc >(__a)
  {
    _M_initialize(__n);
    fill(this->_M_start._M_p, (__chunk_type*)(this->_M_end_of_storage._M_data), __val ? ~0 : 0);
  }

  explicit __BVECTOR(size_type __n)
    : _Bvector_base<_Alloc >(allocator_type())
  {
    _M_initialize(__n);
    fill(this->_M_start._M_p, (__chunk_type*)(this->_M_end_of_storage._M_data), 0);
  }

  __BVECTOR(const _Self& __x) : _Bvector_base<_Alloc >(__x.get_allocator()) {
    _M_initialize(__x.size());
    copy(__x.begin(), __x.end(), this->_M_start);
  }

#if defined (_STLP_MEMBER_TEMPLATES)
  template <class _Integer>
  void _M_initialize_dispatch(_Integer __n, _Integer __x, const __true_type&) {
    _M_initialize(__n);
    fill(this->_M_start._M_p, this->_M_end_of_storage._M_data, __x ? ~0 : 0);
  }
    
  template <class _InputIterator>
  void _M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
                              const __false_type&) {
    _M_initialize_range(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  __BVECTOR(_InputIterator __first, _InputIterator __last)
    : _Base(allocator_type())
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__first, __last, _Integral());
  }
# endif
  template <class _InputIterator>
  __BVECTOR(_InputIterator __first, _InputIterator __last,
            const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : _Base(__a)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__first, __last, _Integral());
  }
#else /* _STLP_MEMBER_TEMPLATES */
  __BVECTOR(const_iterator __first, const_iterator __last,
            const allocator_type& __a = allocator_type())
    : _Bvector_base<_Alloc >(__a)
  {
    size_type __n = distance(__first, __last);
    _M_initialize(__n);
    copy(__first, __last, this->_M_start);
  }
  __BVECTOR(const bool* __first, const bool* __last,
            const allocator_type& __a = allocator_type())
    : _Bvector_base<_Alloc >(__a)
  {
    size_type __n = distance(__first, __last);
    _M_initialize(__n);
    copy(__first, __last, this->_M_start);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

  ~__BVECTOR() { }

  __BVECTOR_QUALIFIED& operator=(const __BVECTOR_QUALIFIED& __x) {
    if (&__x == this) return *this;
    if (__x.size() > capacity()) {
      this->_M_deallocate();
      _M_initialize(__x.size());
    }
    copy(__x.begin(), __x.end(), begin());
    this->_M_finish = begin() + difference_type(__x.size());
    return *this;
  }

  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void _M_fill_assign(size_t __n, bool __x) {
    if (__n > size()) {
      fill(this->_M_start._M_p, (__chunk_type*)(this->_M_end_of_storage._M_data), __x ? ~0 : 0);
      insert(end(), __n - size(), __x);
    }
    else {
      erase(begin() + __n, end());
      fill(this->_M_start._M_p, (__chunk_type*)(this->_M_end_of_storage._M_data), __x ? ~0 : 0);
    }
  }
  void assign(size_t __n, bool __x) { _M_fill_assign(__n, __x); }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, const __true_type&)
    { _M_fill_assign((size_t) __n, (bool) __val); }

  template <class _InputIter>
  void _M_assign_dispatch(_InputIter __first, _InputIter __last, const __false_type&)
    { _M_assign_aux(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIter)); }

  template <class _InputIterator>
  void _M_assign_aux(_InputIterator __first, _InputIterator __last,
                     const input_iterator_tag &) {
    iterator __cur = begin();
    for ( ; __first != __last && __cur != end(); ++__cur, ++__first)
      *__cur = *__first;
    if (__first == __last)
      erase(__cur, end());
    else
      insert(end(), __first, __last);
  }

  template <class _ForwardIterator>
  void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
                     const forward_iterator_tag &) {
    size_type __len = distance(__first, __last);
    if (__len < size())
      erase(copy(__first, __last, begin()), end());
    else {
      _ForwardIterator __mid = __first;
      advance(__mid, size());
      copy(__first, __mid, begin());
      insert(end(), __mid, __last);
    }
  }    

#endif /* _STLP_MEMBER_TEMPLATES */

  void reserve(size_type __n) {
    if (capacity() < __n) {
      unsigned int* __q = this->_M_bit_alloc(__n);
      _Bit_iterator __z(__q, 0);
      this->_M_finish = copy(begin(), end(), __z);
      this->_M_deallocate();
      this->_M_start = iterator(__q, 0);
      this->_M_end_of_storage._M_data = __q + (__n + __WORD_BIT - 1)/__WORD_BIT;
    }
  }

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }
  void push_back(bool __x) {
    if (this->_M_finish._M_p != this->_M_end_of_storage._M_data) {
      *(this->_M_finish) = __x;
      ++this->_M_finish;
    }
    else
      _M_insert_aux(end(), __x);
  }
  void swap(__BVECTOR_QUALIFIED& __x) {
    _STLP_STD::swap(this->_M_start, __x._M_start);
    _STLP_STD::swap(this->_M_finish, __x._M_finish);
    _STLP_STD::swap(this->_M_end_of_storage, __x._M_end_of_storage);
  }
  iterator insert(iterator __position, bool __x = bool()) {
    difference_type __n = __position - begin();
    if (this->_M_finish._M_p != this->_M_end_of_storage._M_data && __position == end()) {
      *(this->_M_finish) = __x;
      ++this->_M_finish;
    }
    else
      _M_insert_aux(__position, __x);
    return begin() + __n;
  }

#if defined ( _STLP_MEMBER_TEMPLATES )

  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
                          const __true_type&) {
    _M_fill_insert(__pos, (size_type) __n, (bool) __x);
  }

  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          const __false_type&) {
    _M_insert_range(__pos, __first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  void insert(iterator __position,
              _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Is_Integral;
    _M_insert_dispatch(__position, __first, __last, _Is_Integral());
  }
#else /* _STLP_MEMBER_TEMPLATES */
  void insert(iterator __position,
              const_iterator __first, const_iterator __last) {
    if (__first == __last) return;
    size_type __n = distance(__first, __last);
    if (capacity() - size() >= __n) {
      __copy_backward(__position, end(), this->_M_finish + __n,
                      random_access_iterator_tag(), (difference_type*)0 );
      copy(__first, __last, __position);
      this->_M_finish += __n;
    }
    else {
      size_type __len = size() + (max)(size(), __n);
      unsigned int* __q = this->_M_bit_alloc(__len);
      iterator __i = copy(begin(), __position, iterator(__q, 0));
      __i = copy(__first, __last, __i);
      this->_M_finish = copy(__position, end(), __i);
      this->_M_deallocate();
      this->_M_end_of_storage._M_data = __q + (__len + __WORD_BIT - 1)/__WORD_BIT;
      this->_M_start = iterator(__q, 0);
    }
  }

  void insert(iterator __position, const bool* __first, const bool* __last) {
    if (__first == __last) return;
    size_type __n = distance(__first, __last);
    if (capacity() - size() >= __n) {
      __copy_backward(__position, end(), this->_M_finish + __n, 
                      random_access_iterator_tag(), (difference_type*)0 );
      copy(__first, __last, __position);
      this->_M_finish += __n;
    }
    else {
      size_type __len = size() + (max)(size(), __n);
      unsigned int* __q = this->_M_bit_alloc(__len);
      iterator __i = copy(begin(), __position, iterator(__q, 0));
      __i = copy(__first, __last, __i);
      this->_M_finish = copy(__position, end(), __i);
      this->_M_deallocate();
      this->_M_end_of_storage._M_data = __q + (__len + __WORD_BIT - 1)/__WORD_BIT;
      this->_M_start = iterator(__q, 0);
    }
  }
#endif /* _STLP_MEMBER_TEMPLATES */
  
  void _M_fill_insert(iterator __position, size_type __n, bool __x) {
    if (__n == 0) return;
    if (capacity() - size() >= __n) {
      __copy_backward(__position, end(), this->_M_finish + difference_type(__n), random_access_iterator_tag(), (difference_type*)0 );
      fill(__position, __position + difference_type(__n), __x);
      this->_M_finish += difference_type(__n);
    }
    else {
      size_type __len = size() + (max)(size(), __n);
      unsigned int* __q = this->_M_bit_alloc(__len);
      iterator __i = copy(begin(), __position, iterator(__q, 0));
      fill_n(__i, __n, __x);
      this->_M_finish = copy(__position, end(), __i + difference_type(__n));
      this->_M_deallocate();
      this->_M_end_of_storage._M_data = __q + (__len + __WORD_BIT - 1)/__WORD_BIT;
      this->_M_start = iterator(__q, 0);
    }
  }

  void insert(iterator __position, size_type __n, bool __x) {
    _M_fill_insert(__position, __n, __x);
  }

  void pop_back() {
      --this->_M_finish; 
  }
  iterator erase(iterator __position) {
    if (__position + 1 != end())
      copy(__position + 1, end(), __position);
      --this->_M_finish;
    return __position;
  }
  iterator erase(iterator __first, iterator __last) {
    this->_M_finish = copy(__last, end(), __first);
    return __first;
  }
  void resize(size_type __new_size, bool __x = bool()) {
    if (__new_size < size()) 
      erase(begin() + difference_type(__new_size), end());
    else
      insert(end(), __new_size - size(), __x);
  }
  void flip() {
    for (unsigned int* __p = this->_M_start._M_p; __p != this->_M_end_of_storage._M_data; ++__p)
      *__p = ~*__p;
  }

  void clear() { erase(begin(), end()); }
};

# if defined  ( _STLP_NO_BOOL ) || defined (__HP_aCC) // fixed soon (03/17/2000)
 
__BVEC_TMPL_HEADER
inline void swap(__BVECTOR_QUALIFIED& __x, __BVECTOR_QUALIFIED& __y) {
  __x.swap(__y);
}

__BVEC_TMPL_HEADER
inline bool _STLP_CALL 
operator==(const __BVECTOR_QUALIFIED& __x, const __BVECTOR_QUALIFIED& __y)
{
  return (__x.size() == __y.size() && 
          equal(__x.begin(), __x.end(), __y.begin()));
}


__BVEC_TMPL_HEADER
inline bool _STLP_CALL 
operator<(const __BVECTOR_QUALIFIED& __x, const __BVECTOR_QUALIFIED& __y)
{
  return lexicographical_compare(__x.begin(), __x.end(), 
                                 __y.begin(), __y.end());
}

_STLP_RELOPS_OPERATORS( __BVEC_TMPL_HEADER, __BVECTOR_QUALIFIED )
  
# endif /* NO_BOOL */
  
#if !defined (_STLP_NO_BOOL)
// This typedef is non-standard.  It is provided for backward compatibility.
  typedef __WORKAROUND_DBG_RENAME(vector) <bool, allocator<bool> > bit_vector;
#endif

_STLP_END_NAMESPACE

#undef _Alloc
#undef _STLP_VECBOOL_TEMPLATE
#undef __BVECTOR
#undef __BVECTOR_QUALIFIED
#undef __BVEC_TMPL_HEADER

# undef __WORD_BIT

#endif /* _STLP_INTERNAL_BVECTOR_H */

// Local Variables:
// mode:C++
// End:

