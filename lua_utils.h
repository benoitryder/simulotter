#ifndef LUA_UTILS_H
#define LUA_UTILS_H

extern "C"
{
#include <lualib.h>
#include <lauxlib.h>
}
#include <vector>

#include "colors.h"
#include "log.h"


/// Prefix for registry keys
#define LUA_REGISTRY_PREFIX  "simulotter_"


class LuaClassBase;


class LuaManager
{
public:
  LuaManager();
  ~LuaManager();

  /// Run a Lua script
  void do_file(const char *filename);

  /** @brief Checks whether a given argument is a color and get it
   *
   * Valid colors are tables of 3 or 4 elements, ordered.
   */
  static void checkcolor(lua_State *L, int narg, Color4 c);

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

  /** @brief ODE bindings
   *
   * Geometries are stored as lightuserdata.
   *
   * @note Unused geoms are not collected. It is not a big problem since they
   * are not in the common space. But it is good practice to destroy them.
   */
  //@{

  static int ode_sphere(lua_State *L);
  static int ode_box(lua_State *L);
  static int ode_plane(lua_State *L);
  static int ode_capsule(lua_State *L);
  static int ode_cylinder(lua_State *L);
  static int ode_ray(lua_State *L);
  //static int ode_convex(lua_State *L);
  //static int ode_trimesh(lua_State *L);

  static int ode_destroy(lua_State *L);

  //@}
};


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
   *
   * @TODO Pas à sa place
   */
  //@{
  static void push(lua_State *L) { lua_pushnil(L); }
  static void push(lua_State *L, int    n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, long   n) { lua_pushinteger(L, n); }
  static void push(lua_State *L, float  n) { lua_pushnumber (L, n); }
  static void push(lua_State *L, double n) { lua_pushnumber (L, n); }
  static void push(lua_State *L, bool   b) { lua_pushboolean(L, b); }
  static void push(lua_State *L, const char *s) { lua_pushstring(L, s); }
  static void push(lua_State *L, Color4 c)
  {
    lua_createtable(L, 0, 4);
    LuaClassBase::push(L, c[0]);
    lua_setfield(L, -2, "r");
    LuaClassBase::push(L, c[1]);
    lua_setfield(L, -2, "g");
    LuaClassBase::push(L, c[2]);
    lua_setfield(L, -2, "b");
    LuaClassBase::push(L, c[3]);
    lua_setfield(L, -2, "a");
  }
  //@}

protected:
  /// Method list, with their name
  std::vector<LuaRegFunc> functions;

private:

  /// Array of registered class.
  static std::vector<LuaClassBase*> classes;

  /** @name Class stuff
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
  //@{

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

  /// Class object metatable name in registry
  static const char *registry_class_mt_name;
};


/** @brief Class inherited by classes which provide a Lua class.
 * @TODO destructors
 * @todo User Lunar?
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
   * @todo Cannot check using checkudata since derived classes does not hava
   * the correct type. Find an other way to check (metatable+inheritance, ...).
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
    //TODO add the prefix
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
    //TODO add the prefix
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
  static T **get_userdata(lua_State *L)
  {
    push(L, "_ud");
    T **ud = (T **)lua_newuserdata(L, sizeof(T *));
    lua_getfield(L, LUA_REGISTRYINDEX, name);//XXX add the prefix
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

//@}

/// Bind getters (no argument, 1 return value)
#define LUA_DEFINE_GET(n) \
  static int n(lua_State *L) { push(L, get_ptr(L)->n()); return 1; }

/** @name Bind setters (arguments, no return value)
 */
//@{

#define LUA_DEFINE_SET0(n) \
  static int n(lua_State *L) { get_ptr(L)->n( ); return 0; }
#define LUA_DEFINE_SET1(n,m1) \
  static int n(lua_State *L) { get_ptr(L)->n( m1(2) ); return 0; }
#define LUA_DEFINE_SET2(n,m1,m2) \
  static int n(lua_State *L) { get_ptr(L)->n( m1(2), m2(3) ); return 0; }
#define LUA_DEFINE_SET3(n,m1,m2,m3) \
  static int n(lua_State *L) { get_ptr(L)->n( m1(2), m2(3), m3(4) ); return 0; }
#define LUA_DEFINE_SET4(n,m1,m2,m3,m4) \
  static int n(lua_State *L) { get_ptr(L)->n( m1(2), m2(3), m3(4), m4(5) ); return 0; }

//@}

class LuaError: public Error
{
public:
  LuaError() { this->err=0; }
  LuaError(const char *msg): Error("LUA: %s", msg) { this->err=0; }
  LuaError(lua_State *L, int err=0):
    Error("LUA: %s", lua_tostring(L,-1)) { lua_pop(L,-1); this->err=err; }
  LuaError(lua_State *L, const char *msg, int err=0):
    Error("LUA: %s: %s", msg, lua_tostring(L,-1)) { lua_pop(L,-1); this->err=err; }
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
