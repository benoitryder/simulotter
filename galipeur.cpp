#include <GL/freeglut.h>
#include "galipeur.h"
#include "physics.h"
#include "sensors.h"
#include "lua_utils.h"
#include "config.h"
#include "log.h"


const btScalar Galipeur::z_mass = btScale(0.08);
const btScalar Galipeur::ground_clearance = btScale(0.009);

const btScalar Galipeur::height  = btScale(0.300);
const btScalar Galipeur::side    = btScale(0.110);
const btScalar Galipeur::w_block = btScale(0.030);
const btScalar Galipeur::r_wheel = btScale(0.0246);
const btScalar Galipeur::h_wheel = btScale(0.0127);

const btScalar Galipeur::d_side  = ( side + 2*w_block ) / btSqrt(3);
const btScalar Galipeur::d_wheel = ( w_block + 2*side ) / btSqrt(3);
const btScalar Galipeur::a_side  = 2*btAtan2( side,    d_side  );
const btScalar Galipeur::a_wheel = 2*btAtan2( w_block, d_wheel );
const btScalar Galipeur::radius  = btSqrt(side*side+d_side*d_side);

SmartPtr<btCompoundShape> Galipeur::shape_;
btConvexHullShape Galipeur::body_shape_;
btBoxShape Galipeur::wheel_shape_( btVector3(h_wheel/2,r_wheel,r_wheel) );
GLuint Galipeur::dl_id_static_ = 0;


Galipeur::Galipeur(btScalar m):
    color_(Color4(0.3))
{
  // First instance: initialize shape
  if( shape_ == NULL )
  {
    shape_ = new btCompoundShape();
    // Triangular body
    if( body_shape_.getNumPoints() == 0 )
    {
      btVector2 p = btVector2(radius,0).rotated(-a_wheel/2);
      for( int i=0; i<3; i++ )
      {
        body_shape_.addPoint( btVector3(p.x,p.y,+height/2) );
        body_shape_.addPoint( btVector3(p.x,p.y,-height/2) );
        p.rotate(a_wheel);
        body_shape_.addPoint( btVector3(p.x,p.y,+height/2) );
        body_shape_.addPoint( btVector3(p.x,p.y,-height/2) );
        p.rotate(a_side);
      }
    }
    shape_->addChildShape( btTransform(
          btMatrix3x3::getIdentity(), btVector3(0,0,ground_clearance-(z_mass-height/2))),
        &body_shape_);

    // Wheels (use boxes instead of cylinders)
    btTransform tr = btTransform::getIdentity();
    btVector2 vw( d_wheel+h_wheel/2, 0 );
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    shape_->addChildShape(tr, &wheel_shape_);

    vw.rotate(2*M_PI/3);
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    tr.setRotation( btQuaternion(btVector3(0,0,1), 2*M_PI/3) );
    shape_->addChildShape(tr, &wheel_shape_);

    vw.rotate(-4*M_PI/3);
    tr.setOrigin( btVector3(vw.x, vw.y, -(z_mass - r_wheel)) );
    tr.setRotation( btQuaternion(btVector3(0,0,1), -2*M_PI/3) );
    shape_->addChildShape(tr, &wheel_shape_);
  }

  btVector3 inertia;
  shape_->calculateLocalInertia(m, inertia);
  body_ = new btRigidBody(
      btRigidBody::btRigidBodyConstructionInfo(m,NULL,shape_,inertia)
      );
  SmartPtr_add_ref(shape_);

  order_stop();
}

Galipeur::~Galipeur()
{
}

void Galipeur::addToWorld(Physics *physics)
{
  physics->getWorld()->addRigidBody(body_);
  Robot::addToWorld(physics);
}

void Galipeur::removeFromWorld()
{
  Physics *ph_bak = physics_;
  Robot::removeFromWorld();
  ph_bak->getWorld()->removeRigidBody(body_);
}

void Galipeur::draw()
{
  glColor4fv(color_);

  glPushMatrix();
  drawTransform(body_->getCenterOfMassTransform());

  if( dl_id_static_ != 0 )
    glCallList(dl_id_static_);
  else
  {
    // new display list
    dl_id_static_ = glGenLists(1);
    glNewList(dl_id_static_, GL_COMPILE_AND_EXECUTE);

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
    glutSolidCylinder(r_wheel, h_wheel, cfg.draw_div, cfg.draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(2*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(-120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg.draw_div, cfg.draw_div);
    glPopMatrix();

    glPushMatrix();
    vw.rotate(-4*M_PI/3);
    btglTranslate(0, vw.y, vw.x);
    btglRotate(120.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCylinder(r_wheel, h_wheel, cfg.draw_div, cfg.draw_div);
    glPopMatrix();

    glPopMatrix();

    glEndList();
  }

  // Sharps
  std::vector<btTransform>::iterator it;
  for( it=sharps_trans_.begin(); it!=sharps_trans_.end(); ++it )
  {
    glPushMatrix();
    drawTransform( *it );
    glColor4fv(Color4::white()); //XXX
    glutSolidCube(0.1); //XXX
    SRay::gp2d12.draw();
    glPopMatrix();
  }

  glPopMatrix();
}


void Galipeur::asserv()
{
  if( this->stopped() )
    return;

  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  const btScalar dt = physics_->getTime() - ramp_last_t_;
  if( dt == 0 )
    return; // ramp start, wait for the next step

  // position
  btVector2 dxy = (*ckpt_) - get_xy();
  if( !this->lastCheckpoint() && dxy.length() < threshold_steering_ ) {
    ++ckpt_; // checkpoint change
    dxy = (*ckpt_) - get_xy();
  }
  if( this->lastCheckpoint() ) {
    ramp_xy_.var_dec = va_stop_;
    ramp_xy_.var_v0 = v_stop_;
  } else {
    ramp_xy_.var_dec = va_steering_;
    ramp_xy_.var_v0 = v_steering_;
  }
  set_v( dxy.normalized() * ramp_xy_.step(dt, dxy.length()) );

  // angle
  const btScalar da = btNormalizeAngle( target_a_-get_a() );
  set_av( ramp_a_.step(dt, da) );
}


void Galipeur::set_v(btVector2 vxy)
{
  body_->activate();
  body_->setLinearVelocity( btVector3(vxy.x, vxy.y,
        body_->getLinearVelocity().z()) );
}

inline void Galipeur::set_av(btScalar v)
{
  body_->activate();
  btVector3 v3 = body_->getAngularVelocity();
  v3.setZ(v);
  body_->setAngularVelocity(v3);
}


void Galipeur::order_xy(btVector2 xy, bool rel)
{
  CheckPoints v(1);
  if( rel )
    xy += this->get_xy();
  v[0] = xy;
  this->order_trajectory(v);
}

void Galipeur::order_a(btScalar a, bool rel)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  if( rel )
    a += this->get_a();
  target_a_ = btNormalizeAngle(a);

  ramp_last_t_ = physics_->getTime();
  ramp_a_.reset(get_av());
}

void Galipeur::order_xya(btVector2 xy, btScalar a, bool rel)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));

  this->order_xy(xy, rel);
  this->order_a(a, rel);
}

void Galipeur::order_stop()
{
  checkpoints_.clear();
  ckpt_ = checkpoints_.end();
}


void Galipeur::order_trajectory(const std::vector<btVector2> &pts)
{
  if( physics_ == NULL )
    throw(Error("Galipeur is not in a world"));
  if( pts.empty() )
    throw(Error("empty checkpoint list"));
  checkpoints_ = pts;
  ckpt_ = checkpoints_.begin();

  ramp_last_t_ = physics_->getTime();
  ramp_xy_.reset(get_v().length());
}


btScalar Galipeur::test_sensor(unsigned int i) const
{
  if( i >= sharps_trans_.size() )
    throw(Error("invalid sensor index: %u"));
  return SRay::gp2d12.hitTest( getTrans() * sharps_trans_[i] );
}


btScalar Galipeur::Quadramp::step(btScalar dt, btScalar d)
{
  const btScalar d_dec = 0.5 * (cur_v_-var_v0)*(cur_v_-var_v0) / var_dec;
  btScalar target_v, a;
  if( btFabs(d) < d_dec ) {
    target_v = var_v0;
    a = var_dec;
  } else {
    target_v = var_v;
    a = var_acc;
  }
  if( d < 0 ) {
    target_v *= -1;
  }
  cur_v_ = cur_v_ + CLAMP(target_v-cur_v_, -a*dt, a*dt);
  return cur_v_;
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

  LUA_DEFINE_SET2(set_speed_xy,       set_speed_xy,       LARG_scaled, LARG_scaled)
  LUA_DEFINE_SET2(set_speed_steering, set_speed_steering, LARG_scaled, LARG_scaled)
  LUA_DEFINE_SET2(set_speed_stop,     set_speed_stop,     LARG_scaled, LARG_scaled)
  LUA_DEFINE_SET1(set_threshold_xy,   set_threshold_xy,   LARG_scaled)
  LUA_DEFINE_SET1(set_threshold_steering, set_threshold_steering, LARG_scaled)
  LUA_DEFINE_SET2(set_speed_a,        set_speed_a,        LARG_f, LARG_f)
  LUA_DEFINE_SET1(set_threshold_a,    set_threshold_a,    LARG_f)

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
  static int order_trajectory(lua_State *L)
  {
    SmartPtr<Galipeur> galipeur = get_ptr(L,1);
    luaL_checktype(L, 2, LUA_TTABLE);
    Galipeur::CheckPoints pts;

    lua_pushnil(L);
    while( lua_next(L, 2) != 0 )
    {
      btVector3 v;
      LuaManager::checkvector(L, -1, v);
      pts.push_back( v );
      lua_pop(L, 1);
    }
    galipeur->order_trajectory(pts);
    return 0;
  }

  LUA_DEFINE_SET0(asserv, asserv)
  LUA_DEFINE_GET(order_xy_done, order_xy_done)
  LUA_DEFINE_GET(order_a_done, order_a_done)
  LUA_DEFINE_GET(is_waiting, is_waiting)
  static int current_checkpoint(lua_State *L)
  {
    LuaManager::push(L, get_ptr(L,1)->current_checkpoint()+1);
    return 1;
  }

  static int set_sharps(lua_State *L)
  {
    SmartPtr<Galipeur> galipeur = get_ptr(L,1);
    luaL_checktype(L, 2, LUA_TTABLE);
    galipeur->sharps_trans_.clear();

    lua_pushnil(L);
    while( lua_next(L, 2) != 0 )
    {
      btTransform tr;
      LuaManager::checktransform(L, -1, tr);
      galipeur->sharps_trans_.push_back( tr );
      lua_pop(L, 1);
    }
    return 0;
  }

  static int test_sharp(lua_State *L)
  {
    SmartPtr<Galipeur> galipeur = get_ptr(L,1);
    unsigned int i = luaL_checkinteger(L, 2) - 1; // LUA indexes start at 1
    if( i < galipeur->sharps_trans_.size() )
    {
      btScalar x = galipeur->test_sensor(i);
      if( x < 0 )
        LuaManager::push(L, false);
      else
        LuaManager::push(L, btUnscale(x));
    }
    else
      lua_pushnil(L);
    return 1;
  }

  static int get_sharp_count(lua_State *L)
  {
    LuaManager::push(L, get_ptr(L, 1)->sharps_trans_.size());
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

    LUA_CLASS_MEMBER(set_speed_xy);
    LUA_CLASS_MEMBER(set_speed_steering);
    LUA_CLASS_MEMBER(set_speed_stop);
    LUA_CLASS_MEMBER(set_threshold_xy);
    LUA_CLASS_MEMBER(set_threshold_steering);
    LUA_CLASS_MEMBER(set_speed_a);
    LUA_CLASS_MEMBER(set_threshold_a);

    LUA_CLASS_MEMBER(order_xy);
    LUA_CLASS_MEMBER(order_xya);
    LUA_CLASS_MEMBER(order_a);
    LUA_CLASS_MEMBER(order_trajectory);
    LUA_CLASS_MEMBER(order_stop);

    LUA_CLASS_MEMBER(asserv);
    LUA_CLASS_MEMBER(order_xy_done);
    LUA_CLASS_MEMBER(order_a_done);
    LUA_CLASS_MEMBER(is_waiting);
    LUA_CLASS_MEMBER(current_checkpoint);

    LUA_CLASS_MEMBER(set_sharps);
    LUA_CLASS_MEMBER(test_sharp);
    LUA_CLASS_MEMBER(get_sharp_count);
  }
};

LUA_REGISTER_SUB_CLASS(Galipeur,Robot);

