#ifndef LUA_UTILS_H
#define LUA_UTILS_H

///@file

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}
#include <vector>

#include "colors.h"
#include "log.h"


class LuaClassBase;


/** @brief Lua state and various functions.
 */
class LuaManager
{
public:
  LuaManager();
  ~LuaManager();

  /// Run a Lua script
  void do_file(const char *filename);

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

  /** @brief Check whether a given argument is a table of userdata
   *
   * @param  L       Lua state
   * @param  narg    checked argument
   * @param  type    type of table elements
   * @param  len_ptr if not null, pointed value is set to the actual table size
   * @param  len     size of table (0: any size, negative: minimum size)
   *
   * @return An array of objects which should be deleted after use.
   *
   * @note If the function fails, the value pointed by \e len_ptr is not *
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

  /** @brief Lua call in protected Lua with error forwarding
   *
   * C++ exceptions are not compatible with Lua error handling
   * (<em>longjmp/setjump</em>). Call is made in protected mode and errors are
   * transmitted (if any).
   */
  static void pcall(lua_State *L, int nargs, int nresults)
  {
    int err = lua_pcall(L, nargs, nresults, 0);
    if( err != 0 )
      lua_error(L);
  }

private:

  lua_State *L;

  /// Write a string line on stdout
  static int lua_trace(lua_State *L);
};


/** @brief Base class for Lua classes
 *
 * Classes are built using the global \e class function.
 *
 * Instances are tables.
 * Instances of (or which inherit from) predefined classes store their
 * userdata in the \e _ud field. If it is modified, results are unexpected.
 *
 * Predefined classes may store Lua objects (e.g. instances) using references
 * in the registry. Thus, they can be associated to C++ instances.
 * 
 * As a convention, field names starting with an underscore are reserved for
 * intern mechanics.
 */
class LuaClassBase
{
public:
  LuaClassBase()
  {
    register_class(this);
  }
  virtual ~LuaClassBase() {}

  /// Init classes
  static void init(lua_State *L);

  /// Create the class
  virtual void create(lua_State *L) = 0;

  typedef struct { const char *name; lua_CFunction f; } LuaRegFunc;

  /// Register a class
  static void register_class(LuaClassBase *c)
  {
    classes.push_back(c);
  }

  virtual const char *get_name() = 0;
  virtual const char *get_base_name() = 0;

  /** @name Push functions
   * @todo Should not be here
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
      LuaClassBase::push(L, c[i]);
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
  //@}

protected:
  /// Method list, with their name
  std::vector<LuaRegFunc> functions;

private:

  /// Array of registered class.
  static std::vector<LuaClassBase*> classes;

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
};


/** @brief Class inherited by classes which provide a Lua class.
 */
template<class T>
class LuaClass: public LuaClassBase
{
public:
  const char *get_name() { return this->name; }
  const char *get_base_name() { return this->base_name; }

protected:
  /** @brief Retrieve a pointer from userdata
   *
   * Get the C++ instance pointer from an instance table.
   * Replace the instance table by the userdata.
   *
   * @todo Cannot check using checkudata since derived classes does not have
   * the correct type. Find another way to check (metatable+inheritance, ...).
   */
  static T *get_ptr(lua_State *L)
  {
    lua_getfield(L, 1, "_ud");
    lua_replace(L, 1);

    return *(T**)lua_touserdata(L, 1);
  }

  /// Create the class
  void create(lua_State *L)
  {
    // Create the class object
    LuaClass<T> **ud = (LuaClass<T> **)lua_newuserdata(L, sizeof(this));
    *ud = this;

    lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
    lua_setmetatable(L, -2);

    // Add global symbol
    lua_setfield(L, LUA_GLOBALSINDEX, get_name());

    // Create the metatable
    luaL_newmetatable(L, get_name());

    lua_getfield(L, LUA_REGISTRYINDEX, registry_class_mt_name);
    lua_setmetatable(L, -2);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Add methods
    std::vector<LuaRegFunc>::iterator it;
    for( it=functions.begin(); it!=functions.end(); it++ )
    {
      lua_pushcfunction(L, (*it).f);
      lua_setfield(L, -2, (*it).name);
    }

    // Add class in registry
    lua_setfield(L, LUA_REGISTRYINDEX, get_name());
  }

  /** @brief Get a new userdata object
   *
   * Create a new userdata, set its metatable, set the \e _ud field of the
   * first argument.
   *
   * @return A pointer to the userdata memory block.
   *
   * @note This function should be called by any constructor.
   */
  static T **new_userdata(lua_State *L)
  {
    push(L, "_ud");
    T **ud = (T **)lua_newuserdata(L, sizeof(T *));
    lua_getfield(L, LUA_REGISTRYINDEX, name);
    lua_setmetatable(L, -2);
    lua_rawset(L, 1);
    return ud;
  }

  /// Class name
  static const char *name;

  /// Base class name
  static const char *base_name;
};

#define LUA_REGISTER_BASE_CLASS(T) \
  LUA_REGISTER_CLASS(T) \
  template<> const char *LuaClass<T>::base_name = NULL;\

#define LUA_REGISTER_SUB_CLASS(T,B) \
  LUA_REGISTER_CLASS(T) \
  template<> const char *LuaClass<T>::base_name = #B;\

#define LUA_REGISTER_CLASS(T) \
  template<> const char *LuaClass<T>::name = #T; \
  static Lua##T register_Lua##T;


/// Define a LuaRegFunc
#define LUA_REGFUNC(n) \
  functions.push_back((LuaRegFunc){ #n, n })

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
  static int n(lua_State *L) { push(L, get_ptr(L)->f()); return i; }
#define LUA_DEFINE_GETN_SCALED(i,n,f) \
  static int n(lua_State *L) { push(L, unscale(get_ptr(L)->f())); return i; }
#define LUA_DEFINE_GET(n,f)  LUA_DEFINE_GETN(1,n,f)
#define LUA_DEFINE_GET_SCALED(n,f) LUA_DEFINE_GETN_SCALED(1,n,f)


/** @name Bind setters (arguments, no return value)
 */
//@{

#define LUA_DEFINE_SET0(n,f) \
  static int n(lua_State *L) { get_ptr(L)->f( ); return 0; }
#define LUA_DEFINE_SET1(n,f,m1) \
  static int n(lua_State *L) { get_ptr(L)->f( m1(2) ); return 0; }
#define LUA_DEFINE_SET2(n,f,m1,m2) \
  static int n(lua_State *L) { get_ptr(L)->f( m1(2), m2(3) ); return 0; }
#define LUA_DEFINE_SET3(n,f,m1,m2,m3) \
  static int n(lua_State *L) { get_ptr(L)->f( m1(2), m2(3), m3(4) ); return 0; }
#define LUA_DEFINE_SET4(n,f,m1,m2,m3,m4) \
  static int n(lua_State *L) { get_ptr(L)->f( m1(2), m2(3), m3(4), m4(5) ); return 0; }

//@}

class LuaError: public Error
{
public:
  LuaError(): err(0) {}
  LuaError(const char *msg): Error("LUA: %s", msg), err(0) {}
  LuaError(lua_State *L, int err=0):
    Error("LUA: %s", lua_tostring(L,-1)), err(err) { lua_pop(L,-1); }
  LuaError(lua_State *L, const char *msg, int err=0):
    Error("LUA: %s: %s", msg, lua_tostring(L,-1)), err(err) { lua_pop(L,-1); }
  ~LuaError() throw() {}
  LuaError(const LuaError &e): Error(e) {}

  LuaError& operator=(const LuaError &e)
  {
    Error::operator=(e);
    this->err = e.err;
    return *this;
  }

  int get_err() { return err; }

private:
  /// Lua error code (0 if none)
  int err;

};



#endif
