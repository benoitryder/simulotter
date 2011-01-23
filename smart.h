#ifndef SMART_H_
#define SMART_H_

///@file

#include <cstring>

/** @brief Smart pointer class
 *
 * Implementation is very similar to boost::intrusive_ptr.
 * Implicit cast to raw pointers has been added.
 * add_ref and release methods have been added.
 */
template<class T>
class SmartPtr
{
public:
  typedef T element_type;

  SmartPtr(): px_(0) {}
  SmartPtr(T *p): px_(p) { if(px_!=0) SmartPtr_add_ref(px_); }
  SmartPtr(SmartPtr const &rhs): px_(rhs.px_) { if(px_!=0) SmartPtr_add_ref(px_); }
  ~SmartPtr() { if(px_!=0) SmartPtr_release(px_); }

  SmartPtr &operator=(SmartPtr const &rhs)
  {
    SmartPtr(rhs).swap(*this);
    return *this;
  }
  SmartPtr &operator=(T *rhs)
  {
    SmartPtr(rhs).swap(*this);
    return *this;
  }

  T *get() const { return px_; }
  T &operator*() const { return *px_; }
  T *operator->() const { return px_; }
  operator T *() const { return px_; }
  bool operator!() const { return px_ == 0; }

  void swap(SmartPtr &rhs)
  {
    T *tmp = px_;
    px_ = rhs.px_;
    rhs.px_ = tmp;
  }

private:
  T *px_;
};

template<class T, class U> inline bool operator==(SmartPtr<T> const &a, SmartPtr<U> const &b) { return a.get() == b.get(); }
template<class T, class U> inline bool operator!=(SmartPtr<T> const &a, SmartPtr<U> const &b) { return a.get() != b.get(); }
template<class T, class U> inline bool operator==(SmartPtr<T> const &a, U *b) { return a.get() == b; }
template<class T, class U> inline bool operator!=(SmartPtr<T> const &a, U *b) { return a.get() != b; }
template<class T, class U> inline bool operator==(T *a, SmartPtr<U> const &b) { return a == b.get(); }
template<class T, class U> inline bool operator!=(T *a, SmartPtr<U> const &b) { return a != b.get(); }
template<class T> inline bool operator<(SmartPtr<T> const &a, SmartPtr<T> const &b) { return a.get() < b.get(); }


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
  size_t ref = (size_t)p->getUserPointer();
  ++ref;
  p->setUserPointer( (void*)ref );
}
template<class T> void bullet_ptr_release(T *p)
{
  size_t ref = (size_t)p->getUserPointer();
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
