#ifndef LUA_UTILS_H
#define LUA_UTILS_H

///@file

#include <vector>

#include "global.h"


class LuaClassBase;
class LuaMainModule;
class LuaError;
template <class T> class LuaClass;


/** @brief Lua state and various functions.
 */
class LuaManager
{
public:
  LuaManager();
  ~LuaManager();

  /// Run a Lua script
  void do_file(const char *filename);

  /** @brief Lua call in protected Lua with error forwarding.
   *
   * C++ exceptions are not compatible with Lua error handling
   * (<em>longjmp/setjump</em>). Thus, throwing after a Lua error will crash.
   * To prevent this, we use an error handler which throw an exception (before
   * the <em>setjump</em>). Other exceptions are caught and rethrown with LUA
   * stack info.
   *
   * If the method fails (an exception is thrown) the LUA state may be in an
   * inconsistent state and the error should lead the programm to exit.
   */
  static void pcall(lua_State *L, int nargs, int nresults);
  /// Handler used by pcall().
  static int pcall_error_handler(lua_State *L);

  /** @brief Convert a given value to a color
   *
   * Valid colors are tables of 3 or 4 elements, indexed by integers.
   *
   * @retval  0  color is valid
   * @retval  1  invalid color, error message has been pushed on the stack
   */
  static int tocolor(lua_State *L, int index, Color4 &c);

  /** @brief Check whether a given argument is a color and get it
   * @sa tocolor
   */
  static void checkcolor(lua_State *L, int narg, Color4 &c);

  /** @brief Convert a given value to a transform
   *
   * Valid transforms are tables of 3 or 6 elements, indexed by integers.
   * First 3 elements give the position, last ones give the rotation.
   *
   * @retval  0  transform is valid
   * @retval  1  invalid transform, error message has been pushed on the stack
   */
  static int totransform(lua_State *L, int index, btTransform &tr);

  /** @brief Check whether a given argument is a transform and get it
   * @sa totransform
   */
  static void checktransform(lua_State *L, int narg, btTransform &tr);

  /** @brief Check whether a given argument is a table of userdata
   *
   * @param  L       Lua state
   * @param  narg    checked argument
   * @param  tname   type of table elements
   * @param  len_ptr if not null, pointed value is set to the actual table size
   * @param  len     size of table (0: any size, negative: minimum size)
   *
   * @return An array of objects which should be deleted after use.
   *
   * @note If the function fails, the value pointed by \e len_ptr is not
   * modified.
   */
  template<typename T> static T *checkudtable(lua_State *L, int narg, const char *tname, int *len_ptr=NULL, int len=0)
  {
    T *a;
    luaL_checktype(L, narg, LUA_TTABLE);

    int nb = lua_objlen(L, narg);
    if( len > 0 )
      luaL_argcheck(L, nb==len, narg, "invalid table size");
    else if( len < 0 )
      luaL_argcheck(L, nb>=len, narg, "invalid table size");

    luaL_getmetatable(L, tname);
    int mt = lua_gettop(L);
    a = new T[nb];

    lua_pushnil(L);
    for( int i=0; i<nb; i++ )
    {
      lua_next(L, narg);
      if( lua_type(L, -1) != LUA_TUSERDATA ||
          lua_getmetatable(L, -1) == 0 ||
          lua_equal(L, mt, -1) == 0 )
      {
        delete[] a;
        luaL_argerror(L, narg, "invalid type in table");
      }
      a[i] = *dynamic_cast<T*>(lua_touserdata(L, -2));
      lua_pop(L, 2);
    }
    lua_pop(L, 2);

    if( len_ptr != NULL )
      *len_ptr = nb;

    return a;
  }


  lua_State *get_L() { return this->L; }

  /** @name Push functions
   */
  //@{
  static void push(lua_State *L) { lua_pushnil(L); }
  static void push(lua_State *L, int    n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, long   n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, float  n) { lua_pushnumber (L, n); }
  static void push(lua_State *L, double n) { lua_pushnumber (L, n); }
  static void push(lua_State *L, bool   b) { lua_pushboolean(L, b); }
  static void push(lua_State *L, const char *s) { lua_pushstring(L, s); }
  static void push(lua_State *L, unsigned int    n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, unsigned long   n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, lua_CFunction f) { lua_pushcfunction(L, f); }
  static void push(lua_State *L, const btVector2 &v) { lua_pushnumber(L, unscale(v.x)); lua_pushnumber(L, unscale(v.y)); }
  static void push(lua_State *L, const btVector3 &v)
  {
    lua_pushnumber(L, unscale(v[0]));
    lua_pushnumber(L, unscale(v[1]));
    lua_pushnumber(L, unscale(v[2]));
  }
  static void push(lua_State *L, const Color4 &c)
  {
    lua_createtable(L, 0, 4);
    for( int i=0; i<4; i++ )
    {
      LuaManager::push(L, c[i]);
      lua_rawseti(L, -2, i+1);
    }
  }
  static void push(lua_State *L, const btMatrix3x3 &m)
  {
    // Push Euler angles, YPR/YXZ order
    btScalar y, p, r;
    m.getEulerYPR(y,p,r);
    push(L, y); push(L, p); push(L, r);
  }
  template<typename T, int n> static void push(lua_State *L, const T t[n]) { for( int i=0; i<n; i++ ) push(L, t[i]); }

  /** @brief Push an already created object, or nil
   *
   * @warning This will not work for LuaShape.
   */
  template<class T> static void push(lua_State *L, T *t)
  {
    if( t == NULL )
      lua_pushnil(L);
    else
    {
      lua_newtable(L);
      LuaClass<T>::store_ptr(L, t, lua_gettop(L));
      lua_getfield(L, LUA_REGISTRYINDEX, LuaClass<T>::name);
      lua_setmetatable(L, -2);
    }
  }

  /// Push and set a field
  template<typename T> static inline void setfield(lua_State *L, int index, const char *k, const T val)
  {
    push(L, val);
    if( index < 0 )
      index--;
    lua_setfield(L, index, k);
  }

  //@}

private:

  lua_State *L;

  /// Write a string line on stdout
  static int lua_trace(lua_State *L);

  static LuaMainModule main_module;
};


/** @brief Base class for Lua classes
 *
 * Classes are built using the global \e class function.
 *
 * Instances are tables.
 * Instances of (or which inherit from) predefined classes store their
 * userdata in the \e _ud field. It must not be modified.
 *
 * Each predefined class stores its metatable in the registry. The \e _cls
 * field of this metatable stores the class userdata. It must not be modified.
 * 
 * Predefined classes may store Lua objects (e.g. instances) using references
 * in the registry. Thus, they can be associated to C++ instances.
 *
 * As a convention, field names starting with an underscore are reserved for
 * internal mechanics.
 */
class LuaClassBase
{
public:
  LuaClassBase();
  virtual ~LuaClassBase() {}

  /// Initialize registered classes, define the \e class method
  static void init(lua_State *L);

protected:

  /** @brief Create class, add it to the registry
   *
   * @note Base class is not set in this method because class registration
   * order cannot be guaranteed.
   */
  void create(lua_State *L);

  /** @brief Initialize the base class
   */
  void init_base_class(lua_State *L);

  /** @brief Add class members.
   *
   * The class table to add members to is on the top of the stack.
   */
  virtual void init_class_members(lua_State *L) = 0;

  /// Create and push the class userdata onto the stack.
  virtual void push_class_ud(lua_State *L) = 0;

  virtual const char *get_name() = 0;
  virtual const char *get_base_name() = 0;

private:

  /** @brief Constructor for lua classes
   *
   * Two parameters: base class (or \e nil) and an optional constructor for the
   * class.
   */
  static int new_class(lua_State *L);

  /** @brief Instance constructor, binded to class call
   */
  static int new_instance(lua_State *L);

  /** @brief Class index metamethod
   *
   * Retrieve the required field from the base class (if any) or from the
   * method table (for class userdata objects, since they do not contain their
   * methods).
   * If the base class is a predefined class, get field from \e _ud.
   */
  static int index_class(lua_State *L);

  /// Class metatable key in registry
  static const char *registry_class_mt_name;

  /// Array of registered class.
  static std::vector<LuaClassBase*> classes;

};


/** @brief Class inherited by classes which provide a Lua class.
 *
 * Userdata are raw pointers.
 * Refcount is increased in store_ptr() and decreased in _gc() to count the
 * pointer held by Lua.
 */
template<class T>
class LuaClass: public LuaClassBase
{
  friend class LuaManager;
public:
  typedef T data_type;
  typedef T *data_ptr;

  const char *get_name() { return this->name; }
  const char *get_base_name() { return this->base_name; }

  /** @brief Retrieve userdata pointer
   *
   * Get the C++ pointer from an instance.
   *
   * @todo Cannot check using checkudata since derived classes does not have
   * the correct type. Find another way to check (metatable+inheritance, ...).
   */
  static data_ptr get_ptr(lua_State *L, int narg)
  {
    if( lua_istable(L, narg) )
      lua_getfield(L, narg, "_ud");
    else
      lua_pushvalue(L, narg);
    if( !lua_isuserdata(L, -1) )
      throw(LuaError(L, "invalid type, userdata expected"));
    data_ptr ptr = *(data_ptr *)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return ptr;
  }

protected:
  virtual void init_class_members(lua_State *L)
  {
    // Define garbage collector metamethod
    lua_pushcfunction(L, _gc);
    lua_setfield(L, -2, "__gc");
    // Add class specific members
    this->init_members(L);
  }

  /** @brief Add class specific members
   *
   * This method is intended to be defined by each subclasse.
   *
   * The class table to add members to is on the top of the stack.
   *
   * @note This name may seem strange since it could (should?) be more
   * explicit. It has been choosed to be convenient for subclass definition.
   */
  virtual void init_members(lua_State *L) = 0;

  virtual void push_class_ud(lua_State *L)
  {
    // Create the class object
    LuaClass<T> **ud = (LuaClass<T> **)lua_newuserdata(L, sizeof(this));
    *ud = this;
  }

  /// Garbage collector method
  static int _gc(lua_State *L)
  {
    data_ptr *ud = (data_ptr *)luaL_checkudata(L, 1, name);
    SmartPtr_release( *ud );
    return 0;
  }

  /** @brief Return pointer to a new userdata object
   *
   * Create a new userdata, set its metatable, set the \e _ud field of the
   * given argument and return a pointer to the userdata.
   */
  static data_ptr *new_ptr(lua_State *L, int narg=1)
  {
    // Create userdata
    LuaManager::push(L, "_ud");
    data_ptr *ud = (data_ptr *)lua_newuserdata(L, sizeof(data_ptr));
    lua_getfield(L, LUA_REGISTRYINDEX, name);
    lua_setmetatable(L, -2);
    lua_rawset(L, narg);
    return ud;
  }

  /** @brief Store a new userdata object
   *
   * @note This function should be called by any constructor.
   */
  static void store_ptr(lua_State *L, data_ptr p, int narg=1)
  {
    data_ptr *ud = new_ptr(L, narg);
    *ud = p;
    // Increase ref count and store the pointer
    SmartPtr_add_ref(p);
  }


  /// Class name
  static const char *name;

  /// Base class name
  static const char *base_name;
};


/// Prefix for all simulotter entries in the registry
#define LUA_REGISTRY_PREFIX  "simulotter_"
/// Namespace prefix for registry entries
#define LUA_NS_PREFIX(NS) #NS "_"


#define LUA_REGISTER_BASE_CLASS_NAME(C,T,N) \
  LUA_REGISTER_CLASS_NAME(C,T,N) \
  template<> const char *LuaClass<T>::base_name = NULL;\

#define LUA_REGISTER_SUB_CLASS_NAME(C,T,N,B) \
  LUA_REGISTER_CLASS_NAME(C,T,N) \
  template<> const char *LuaClass<T>::base_name = LUA_REGISTRY_PREFIX B;\

#define LUA_REGISTER_CLASS_NAME(C,T,N) \
  template<> const char *LuaClass<T>::name = LUA_REGISTRY_PREFIX N; \
  static C C##_register;

#define LUA_REGISTER_BASE_CLASS(T)   LUA_REGISTER_BASE_CLASS_NAME(Lua##T,T,#T)
#define LUA_REGISTER_SUB_CLASS(T,B)  LUA_REGISTER_SUB_CLASS_NAME(Lua##T,T,#T,#B)

#define LUA_REGISTER_BASE_CLASS_NS(NS,T)   LUA_REGISTER_BASE_CLASS_NAME(Lua##T,T,LUA_NS_PREFIX(NS) #T)
#define LUA_REGISTER_SUB_CLASS_NS(NS,T,B)  LUA_REGISTER_SUB_CLASS_NAME(Lua##T,T,LUA_NS_PREFIX(NS) #T,#B)


/** @brief Set a member to a class
 * This method is intended to be used in a \e init_members() method but can be
 * used to set a field on any table at the top of the stack.
 */
#define LUA_CLASS_MEMBER_VAL(N,V)  LuaManager::setfield(L, -1, N, V)
#define LUA_CLASS_MEMBER(N)  LUA_CLASS_MEMBER_VAL(#N,N)


/** @name Argument check macros
 */
//@{

#define LARG_i(n)   (luaL_checkint(L,(n)))
#define LARG_b(n)   (luaL_checktype(L,(n), LUA_TBOOLEAN),lua_toboolean(L,(n)))
#define LARG_f(n)   (luaL_checknumber(L,(n)))
#define LARG_s(n)   (luaL_checkstring(L,(n)))
#define LARG_bn(n)  (lua_toboolean(L,(n)))
#define LARG_lud(n) (luaL_checktype(L,(n), LUA_TLIGHTUSERDATA),lua_touserdata(L,(n)))
#define LARG_scaled(n) (scale(LARG_f(n)))

//@}

/** @name Bind getters (no argument, \e i return values)
 */
//@{

#define LUA_DEFINE_GETN(i,n,f) \
  static int n(lua_State *L) { LuaManager::push(L, get_ptr(L,1)->f()); return i; }
#define LUA_DEFINE_GETN_SCALED(i,n,f) \
  static int n(lua_State *L) { LuaManager::push(L, unscale(get_ptr(L,1)->f())); return i; }
#define LUA_DEFINE_GET(n,f)  LUA_DEFINE_GETN(1,n,f)
#define LUA_DEFINE_GET_SCALED(n,f) LUA_DEFINE_GETN_SCALED(1,n,f)


/** @name Bind setters (arguments, no return value)
 */
//@{

#define LUA_DEFINE_SET0(n,f) \
  static int n(lua_State *L) { get_ptr(L,1)->f( ); return 0; }
#define LUA_DEFINE_SET1(n,f,m1) \
  static int n(lua_State *L) { get_ptr(L,1)->f( m1(2) ); return 0; }
#define LUA_DEFINE_SET2(n,f,m1,m2) \
  static int n(lua_State *L) { get_ptr(L,1)->f( m1(2), m2(3) ); return 0; }
#define LUA_DEFINE_SET3(n,f,m1,m2,m3) \
  static int n(lua_State *L) { get_ptr(L,1)->f( m1(2), m2(3), m3(4) ); return 0; }
#define LUA_DEFINE_SET4(n,f,m1,m2,m3,m4) \
  static int n(lua_State *L) { get_ptr(L,1)->f( m1(2), m2(3), m3(4), m4(5) ); return 0; }

//@}


/** @brief Group of classes and functons
 */
class LuaModule
{
public:
  LuaModule() {}

  /** @brief Add module to LUA environment.
   *
   * Elements are put in the table with the given name (created if needed) or
   * in the global environment if name is \e NULL.
   */
  void import(lua_State *L, const char *name);

protected:

  /** @brief Add module elements.
   *
   * This method is intended to be defined by each module subclasse.
   *
   * Elements should be added to element at the top of the stack.
   */
  virtual void do_import(lua_State *L) = 0;

  /** @brief Import class \e cls with name \e name.
   */
  void import_class(lua_State *L, const char *cls, const char *name);
};

#define LUA_IMPORT_CLASS_NAME(C,N) \
  import_class(L, LUA_REGISTRY_PREFIX C, N)
#define LUA_IMPORT_CLASS(C)  LUA_IMPORT_CLASS_NAME(#C, #C)
#define LUA_IMPORT_CLASS_NS_NAME(NS,C,N)  LUA_IMPORT_CLASS_NAME(LUA_NS_PREFIX(NS) C, N)
#define LUA_IMPORT_CLASS_NS(NS,C)  LUA_IMPORT_CLASS_NS_NAME(NS,#C,#C)


/** @brief Module with main elements.
 */
class LuaMainModule: public LuaModule
{
public:
  virtual void do_import(lua_State *L);
};



class LuaError: public Error
{
public:
  /// Get error message from the stack (and pop it).
  LuaError(lua_State *L);
  /// LUA error with given message.
  LuaError(lua_State *L, const char *msg):
    Error() { setLuaMsg(L, msg); }
  // Add LUA error info to an existing error.
  LuaError(lua_State *L, const Error &e):
    Error() { setLuaMsg(L, e.what()); }
  ~LuaError() throw() {}
  LuaError(const LuaError &e): Error(e) {}

protected:
  void setLuaMsg(lua_State *L, const char *msg);
};


#endif
