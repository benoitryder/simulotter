#include <GL/freeglut.h>

#include "galipeur.h"

const btScalar Galipeur::z_mass = scale(0.05);
const btScalar Galipeur::ground_clearance = scale(0.008);

const btScalar Galipeur::height  = scale(0.200);
const btScalar Galipeur::side    = scale(0.110);
const btScalar Galipeur::w_block = scale(0.030);
const btScalar Galipeur::r_wheel = scale(0.0246);
const btScalar Galipeur::h_wheel = scale(0.0127);

const btScalar Galipeur::d_side  = ( side + 2*w_block ) / btSqrt(3);
const btScalar Galipeur::d_wheel = ( w_block + 2*side ) / btSqrt(3);
const btScalar Galipeur::a_side  = 2*btAtan2( side,    d_side  );
const btScalar Galipeur::a_wheel = 2*btAtan2( w_block, d_wheel );
const btScalar Galipeur::radius  = btSqrt(side*side+d_side*d_side);

SmartPtr<btCompoundShape> Galipeur::shape;
btConvexHullShape Galipeur::body_shape;
btBoxShape Galipeur::wheel_shape( btVector3(h_wheel/2,r_wheel,r_wheel) );
GLuint Galipeur::dl_id_static = 0;


Galipeur::Galipeur(btScalar m)
{
  // First instance: initialize shape
  if( shape == NULL )
  {
    shape = new btCompoundShape();
    // Triangular body
    if( body_shape.getNumPoints() == 0 )
    {
      btVector2 p = btVector2(radius,0).rotated(-a_wheel/2);
      for( int i=0; i<3; i++ )
      {
        body_shape.addPoint( btVector3(p.x,p.y,+height/2) );
        body_shape.addPoint( btVector3(p.x,p.y,-height/2) );
        p.rotate(a_wheel);
        body_shape.addPoint( btVector3(p.x,p.y,+height/2) );
        body_shape.addPoint( btVector3(p.x,p.y,-height/2) );
        p.rotate(a_side);
      }
    }
    shape->addChildShape( btTransform(
          btMatrix3x3::getIdentity(), btVector3(0,0,ground_clearance-(z_mass-height/2))),
        &body_shape);

    // Wheels (use boxes instead of cylinders)
    btTransform tr = btTransform::getIdentity();
    btVector2 vw( d_wheel+h_wheel/2, 0 );
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    shape->addChildShape(tr, &wheel_shape);

    vw.rotate(2*M_PI/3);
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    tr.setRotation( btQuaternion(btVector3(0,0,1), 2*M_PI/3) );
    shape->addChildShape(tr, &wheel_shape);

    vw.rotate(-4*M_PI/3);
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    tr.setRotation( btQuaternion(btVector3(0,0,1), -2*M_PI/3) );
    shape->addChildShape(tr, &wheel_shape);
  }

  btVector3 inertia;
  shape->calculateLocalInertia(m, inertia);
  this->body = new btRigidBody(
      btRigidBody::btRigidBodyConstructionInfo(m,NULL,shape,inertia)
      );
  SmartPtr_add_ref(shape);

  // Init order
  this->order = ORDER_STOP;

  color = Color4(0.3);
}

Galipeur::~Galipeur()
{
}

void Galipeur::addToWorld(Physics *physics)
{
  physics->getWorld()->addRigidBody(body);
  Robot::addToWorld(physics);
}

void Galipeur::removeFromWorld()
{
  Physics *ph_bak = this->physics;
  Robot::removeFromWorld();
  ph_bak->getWorld()->removeRigidBody(body);
}

void Galipeur::draw()
{
  glColor4fv(color);

  glPushMatrix();
  drawTransform(body->getCenterOfMassTransform());

  if( dl_id_static != 0 )
    glCallList(dl_id_static);
  else
  {
    // new display list
    dl_id_static = glGenLists(1);
    glNewList(dl_id_static, GL_COMPILE_AND_EXECUTE);

    glPushMatrix();

    btglTranslate(0, 0, -z_mass);

    // Faces

    glPushMatrix();

    btglTranslate(0, 0, ground_clearance);

    btglScale(radius, radius, height);
    btVector2 v;


    glBegin(GL_QUADS);

    v = btVector2(1,0).rotated(-a_wheel/2);
    btVector2 n(1,0); // normal vector
    for( int i=0; i<3; i++ )
    {
      // wheel side
      btglNormal3(n.x, n.y, 0.0);
      n.rotate(M_PI/3);

      btglVertex3(v.x, v.y, 0.0);
      btglVertex3(v.x, v.y, 1.0);
      v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 1.0);
      btglVertex3(v.x, v.y, 0.0);

      // triangle side
      btglNormal3(n.x, n.y, 0.0);
      n.rotate(M_PI/3);

      btglVertex3(v.x, v.y, 0.0);
      btglVertex3(v.x, v.y, 1.0);
      v.rotate(a_side);
      btglVertex3(v.x, v.y, 1.0);
      btglVertex3(v.x, v.y, 0.0);
    }

    glEnd();

    // Bottom
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, -1.0);
    v = btVector2(1,0).rotated(-a_wheel/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x, v.y, 0.0);
      v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 0.0);
      v.rotate(a_side);
    }
    glEnd();

    // Top
    glBegin(GL_POLYGON);
    btglNormal3(0.0, 0.0, 1.0);
    v = btVector2(1,0).rotated(-a_wheel/2);
    for( int i=0; i<3; i++ )
    {
      btglVertex3(v.x, v.y, 1.0);
      v.rotate(a_wheel);
      btglVertex3(v.x, v.y, 1.0);
      v.rotate(a_side);
    }
    glEnd();

    glPopMatrix();

    // Wheels (box shapes, but drawn using cylinders)
    btglTranslate(0, 0, r_wheel);
    btglRotate(90.0f, 0.0f, 1.0f, 0.0f);
    btVector2 vw( d_wheel, 0 );

    glPushMatrix();
    btglTranslate(0, vw.y, vw.x);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(2*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(-120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(-4*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg->draw_div, cfg->draw_div);
    glPopMatrix();

    glPopMatrix();

    glEndList();
  }

  // Sharps
  std::vector<btTransform>::iterator it;
  for( it=sharps_trans.begin(); it!=sharps_trans.end(); ++it )
  {
    glPushMatrix();
    drawTransform( *it );
    glutSolidCube(0.1); //XXX
    SRay::gp2d12.draw();
    glPopMatrix();
  }

  glPopMatrix();
}

void Galipeur::asserv()
{
  if( physics == NULL )
    throw(Error("Galipeur is not in a world"));

  if( order == ORDER_STOP )
    return;

  const btScalar dt = physics->getTime() - ramp_last_t;
  if( dt == 0 )
    return; // ramp start, wait for the next step

  bool in_position_xy = false;
  bool in_position_a  = false;

  // Position
  const btVector2 dxy = target_xy - get_xy();
  const btScalar vxy = ramp_xy.step(dt, dxy.length());
  if( vxy == 0 )
  {
    set_v(btVector2(0,0));
    in_position_xy = true;
  }
  else
  {
    set_v( vxy * dxy.normalized() );
  }

  // Position
  const btScalar da = btNormalizeAngle( target_a-get_a() );
  const btScalar va = ramp_a.step(dt, da);
  if( va == 0 )
  {
    set_av(0);
    in_position_a = true;
  }
  else
  {
    set_av( va );
  }

  if( order & ORDER_GO && in_position_xy && in_position_a )
    order = ORDER_NONE;
}

void Galipeur::set_v(btVector2 vxy)
{
  body->activate();
  body->setLinearVelocity( btVector3(vxy.x, vxy.y,
        body->getLinearVelocity().z()) );
}

inline void Galipeur::set_av(btScalar v)
{
  body->activate();
  btVector3 v3 = body->getAngularVelocity();
  v3.setZ(v);
  body->setAngularVelocity(v3);
}


void Galipeur::order_xy(btVector2 xy, bool rel)
{
  order_xya( xy, rel ? 0 : get_a(), rel );
}

void Galipeur::order_a(btScalar a, bool rel)
{
  order_xya( rel ? btVector2(0,0) : get_xy(), a, rel );
}

void Galipeur::order_xya(btVector2 xy, btScalar a, bool rel)
{
  if( physics == NULL )
    throw(Error("Galipeur is not in a world"));

  target_xy = xy;
  target_a = a;
  if( rel )
  {
    target_xy += this->get_xy();
    target_a += this->get_a();
  }
  target_a = btNormalizeAngle(target_a);

  order |= ORDER_GO;
  // reset ramps
  ramp_last_t = physics->getTime();
  ramp_xy.reset(get_v().length());
  ramp_a.reset(get_av());
}

btScalar Galipeur::test_sensor(unsigned int i)
{
  if( i >= sharps_trans.size() )
    throw(Error("invalid sensor index: %u"));
  return SRay::gp2d12.hitTest( getTrans() * sharps_trans[i] );
}


btScalar Galipeur::Quadramp::step(btScalar dt, btScalar d)
{
  if( btFabs(d) < threshold && btFabs(cur_v) < var_a )
  {
    // target reached
    cur_v = 0;
  }
  else
  {
    const btScalar d_dec = 0.5 * cur_v*cur_v / var_a;
    if( d < 0 )
    {
      if( -d < d_dec )
        cur_v = MIN(0, cur_v + dt*var_a );
      else if( cur_v > -var_v )
        cur_v = MAX( -var_v, cur_v - dt*var_a );
    }
    else
    {
      if( d < d_dec )
        cur_v = MAX(0, cur_v - dt*var_a );
      else if( cur_v < var_v )
        cur_v = MIN( var_v, cur_v + dt*var_a );
    }
  }

  return cur_v;
}




class LuaGalipeur: public LuaClass<Galipeur>
{
  static int _ctor(lua_State *L)
  {
    store_ptr(L, new Galipeur(LARG_f(2)));
    return 0;
  }

  static int set_color(lua_State *L)
  {
    Color4 color;
    LuaManager::checkcolor(L, 2, color);
    get_ptr(L,1)->setColor( color );
    return 0;
  }

  LUA_DEFINE_GETN_SCALED(2, get_xy, get_xy)
  LUA_DEFINE_GETN_SCALED(2, get_v, get_v)
  LUA_DEFINE_GET(get_a , get_a)
  LUA_DEFINE_GET(get_av, get_av)

  LUA_DEFINE_SET2(set_ramp_xy,      set_ramp_xy,      LARG_scaled, LARG_scaled)
  LUA_DEFINE_SET2(set_ramp_a,       set_ramp_a,       LARG_f, LARG_f)
  LUA_DEFINE_SET1(set_threshold_xy, set_threshold_xy, LARG_scaled)
  LUA_DEFINE_SET1(set_threshold_a,  set_threshold_a,  LARG_f)

  static int order_xy(lua_State *L)
  {
    get_ptr(L,1)->order_xy( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_bn(4) );
    return 0;
  }
  static int order_xya(lua_State *L)
  {
    get_ptr(L,1)->order_xya( btVector2(LARG_scaled(2), LARG_scaled(3)), LARG_f(4), LARG_bn(5) );
    return 0;
  }
  LUA_DEFINE_SET2(order_a, order_a, LARG_f, LARG_bn)
  LUA_DEFINE_SET0(order_stop, order_stop)

  LUA_DEFINE_SET0(asserv, asserv)
  LUA_DEFINE_GET(is_waiting, is_waiting)

  static int set_sharps(lua_State *L)
  {
    SmartPtr<Galipeur> galipeur = get_ptr(L,1);
    luaL_checktype(L, 2, LUA_TTABLE);
    galipeur->sharps_trans.clear();

    lua_pushnil(L);
    while( lua_next(L, 2) != 0 )
    {
      btTransform tr;
      LuaManager::checktransform(L, -1, tr);
      galipeur->sharps_trans.push_back( tr );
      lua_pop(L, 1);
    }
    return 0;
  }

  static int test_sharp(lua_State *L)
  {
    SmartPtr<Galipeur> galipeur = get_ptr(L,1);
    unsigned int i = luaL_checkinteger(L, 2) - 1; // LUA indexes start at 1
    if( i < galipeur->sharps_trans.size() )
    {
      btScalar x = galipeur->test_sensor(i);
      if( x < 0 )
        LuaManager::push(L, false);
      else
        LuaManager::push(L, unscale(x));
    }
    else
      lua_pushnil(L);
    return 1;
  }

  static int get_sharp_count(lua_State *L)
  {
    LuaManager::push(L, get_ptr(L, 1)->sharps_trans.size());
    return 1;
  }


  virtual void init_members(lua_State *L)
  {
    LUA_CLASS_MEMBER(_ctor);
    LUA_CLASS_MEMBER(set_color);

    LUA_CLASS_MEMBER(get_xy);
    LUA_CLASS_MEMBER(get_v);
    LUA_CLASS_MEMBER(get_a);
    LUA_CLASS_MEMBER(get_av);

    LUA_CLASS_MEMBER(set_ramp_xy);
    LUA_CLASS_MEMBER(set_ramp_a);
    LUA_CLASS_MEMBER(set_threshold_xy);
    LUA_CLASS_MEMBER(set_threshold_a);

    LUA_CLASS_MEMBER(order_xy);
    LUA_CLASS_MEMBER(order_xya);
    LUA_CLASS_MEMBER(order_a);
    LUA_CLASS_MEMBER(order_stop);

    LUA_CLASS_MEMBER(asserv);
    LUA_CLASS_MEMBER(is_waiting);

    LUA_CLASS_MEMBER(set_sharps);
    LUA_CLASS_MEMBER(test_sharp);
    LUA_CLASS_MEMBER(get_sharp_count);
  }
};

LUA_REGISTER_SUB_CLASS(Galipeur,Robot);

