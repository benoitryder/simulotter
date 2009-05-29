#ifndef SMART_H
#define SMART_H

///@file

/** @brief Smart pointer class
 *
 * Implementation is the same as boost::intrusive_ptr.
 * Some features have been removed.
 * Implicit cast to raw pointers has been added.
 * add_ref and release methods have been added.
 */
template<class T>
class SmartPtr
{
private:
  typedef SmartPtr this_type;

public:

  SmartPtr(): px( 0 )
  {
  }

  SmartPtr( T * p, bool do_add_ref = true ): px( p )
  {
    if( px != 0 && do_add_ref ) SmartPtr_add_ref( px );
  }

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)
  template<class U> SmartPtr( SmartPtr<U> const & rhs ) : px( rhs.get() )
  {
    if( px != 0 ) SmartPtr_add_ref( px );
  }
#endif

  SmartPtr(SmartPtr const & rhs): px( rhs.px )
  {
    if( px != 0 ) SmartPtr_add_ref( px );
  }

  ~SmartPtr()
  {
    if( px != 0 ) SmartPtr_release( px );
  }

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)
  template<class U> SmartPtr & operator=(SmartPtr<U> const & rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }
#endif

  SmartPtr & operator=(SmartPtr const & rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  SmartPtr & operator=(T * rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  void reset() { this_type().swap( *this ); }
  void reset( T * rhs ) { this_type( rhs ).swap( *this ); }

  T * get() const { return px; }
  T & operator*() const { return *px; }
  T * operator->() const { return px; }
  operator T *() const { return px; }

  void swap(SmartPtr & rhs)
  {
    T * tmp = px;
    px = rhs.px;
    rhs.px = tmp;
  }

private:

  T * px;
};

template<class T, class U> inline bool operator==(SmartPtr<T> const & a, SmartPtr<U> const & b) { return a.get() == b.get(); }
template<class T, class U> inline bool operator!=(SmartPtr<T> const & a, SmartPtr<U> const & b) { return a.get() != b.get(); }
template<class T, class U> inline bool operator==(SmartPtr<T> const & a, U * b) { return a.get() == b; }
template<class T, class U> inline bool operator!=(SmartPtr<T> const & a, U * b) { return a.get() != b; }
template<class T, class U> inline bool operator==(T * a, SmartPtr<U> const & b) { return a == b.get(); }
template<class T, class U> inline bool operator!=(T * a, SmartPtr<U> const & b) { return a != b.get(); }
#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96
// Resolve the ambiguity between our op!= and the one in rel_ops
template<class T> inline bool operator!=(SmartPtr<T> const & a, SmartPtr<T> const & b) { return a.get() != b.get(); }
#endif
template<class T> inline bool operator<(SmartPtr<T> const & a, SmartPtr<T> const & b) { return a.get() < b.get(); }


/** @brief Base class for objects with ref count.
 */
class SmartObject
{
protected:
  SmartObject(): ref_(0) {}
  virtual ~SmartObject() {};
  unsigned int get_count() const { return ref_; }
private:
  unsigned int ref_;
  friend void SmartPtr_add_ref(SmartObject *);
  friend void SmartPtr_release(SmartObject *);
};


/** @name Functions to manage ref count.
 * Due to name resolution issues, SmartPtr functions should be defined for
 * each Bullet class using bullet_ptr_* aliases.
 */
//@{

inline void SmartPtr_add_ref(SmartObject *p)
{
  ++(p->ref_);
}
inline void SmartPtr_release(SmartObject *p)
{
  if( --(p->ref_) == 0 )
    delete p;
}

template<class T> void bullet_ptr_add_ref(T *p)
{
  unsigned int ref = (unsigned int)p->getUserPointer();
  ++ref;
  p->setUserPointer( (void*)ref );
}
template<class T> void bullet_ptr_release(T *p)
{
  unsigned int ref = (unsigned int)p->getUserPointer();
  if( --ref == 0 )
    delete p;
  else
    p->setUserPointer( (void*)ref );
}

#include "bullet.h"
inline void SmartPtr_add_ref(btCollisionShape *p) { bullet_ptr_add_ref(p); }
inline void SmartPtr_release(btCollisionShape *p) { bullet_ptr_release(p); }

//@}


#endif
