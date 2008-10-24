#include <string.h>
#include <ode/ode.h>
#include "global.h"
#include "robot.h"
#include "maths.h"


Physics::Physics()
{
  dInitODE();
  pause_state = false;

  world = dWorldCreate();
  space = dSimpleSpaceCreate(0);

  dWorldSetGravity(world, 0, 0, cfg->gravity_z);

  contacts = dJointGroupCreate(0);
  dJointGroupEmpty(contacts);
}

Physics::~Physics()
{
  std::vector<Object*>::iterator it;
  for( it = objs.begin(); it != objs.end(); it++ )
    delete *it;
  objs.clear();

  dSpaceDestroy(space);
  dWorldDestroy(world);

  dCloseODE();
}


void Physics::step()
{
  if( this->pause_state )
    return;

  std::vector<dGeomID>::iterator it2;
  for( it2=hack_boxes.begin(); it2!=hack_boxes.end(); it2++ )
    dGeomDestroy(*it2);
  hack_boxes.clear();
  hack_cylinders.clear();

  dSpaceCollide(space, this, &Physics::collide_callback);

  // Cylinder hack
  // Convert the top cylinder into a box
  // Fine with small cylinders (h < r)
  // XXX si les base ne touchent pas, utiliser deux capsules?
  std::vector<GeomPair>::iterator it;
  for( it=hack_cylinders.begin(); it!=hack_cylinders.end(); it++ )
  {
    dGeomID o1 = (*it).o1;
    dGeomID o2 = (*it).o2;
    const dReal *v1, *v2;

    v1 = dGeomGetPosition(o1);
    v2 = dGeomGetPosition(o2);
    // The box should be the top element, swap if needed
    //XXX Use bounding box ?
    //XXX Really better to transform the top element?
    if( v2[2] > v1[2] )
    {
      o1 = (*it).o2;
      o2 = (*it).o1;
      v1 = dGeomGetPosition(o1);
      v2 = dGeomGetPosition(o2);
    }

    dReal r, l;
    dGeomCylinderGetParams(o1, &r, &l);
    dGeomID b1 = dCreateBox(0, 2*r, 2*r, l);
    dGeomSetBody(b1, dGeomGetBody(o1));

    dGeomSetCategoryBits(b1, dGeomGetCategoryBits(o1));

    const dReal *R1 = dGeomGetRotation(o1);
    const dReal *R2 = dGeomGetRotation(o2);

    // vc: vecteur joignant les centres
    dVector3 vc, v, vv;
    vc[0]=v2[0]-v1[0]; vc[1]=v2[1]-v1[1]; vc[2]=v2[2]-v1[2];
    // On garde la composante orthogonale à l'axe de o2
    // On passe dans le repère de o2, on met z à 0 et on rechange de repère
    v[0] = R2[0]*vc[0]+R2[4]*vc[1]+R2[8]*vc[2];
    v[1] = R2[1]*vc[0]+R2[5]*vc[1]+R2[9]*vc[2];
    vv[0] = R2[0]*v[0]+R2[1]*v[1];
    vv[1] = R2[4]*v[0]+R2[5]*v[1];
    vv[2] = R2[8]*v[0]+R2[9]*v[1];

    // Passage dans le repère de o1, on veut v colinéaire à x1
    v[0] = R1[0]*vv[0]+R1[4]*vv[1]+R1[8]*vv[2];
    v[1] = R1[1]*vv[0]+R1[5]*vv[1]+R1[9]*vv[2];

    if( !isnan(v[1]/v[0]) )
    {
      dQuaternion q;
      dQFromAxisAndAngle(q, 0, 0, 1, atan(v[1]/v[0]));
      if( dGeomGetBody(b1) != NULL )
        dGeomSetOffsetQuaternion(b1, q);
      else
      {
        dQuaternion q1, qq;
        dGeomGetQuaternion(b1, q1);
        dQMultiply1(qq, q1, q);
        dGeomSetQuaternion(b1, q);
      }
    }

    collide_callback(this, b1, o2);
    //hack_boxes.push_back(b1);
  }

  dWorldStep(world, cfg->step_dt);
  dJointGroupEmpty(contacts);

  // Update robot values, and asserv them
  std::vector<Robot*> &robots = Robot::get_robots();
  std::vector<Robot*>::iterator itr;
  for( itr=robots.begin(); itr!=robots.end(); itr++ )
  {
    (*itr)->update();
    (*itr)->asserv();
  }
}


void Physics::collide_callback(void *data, dGeomID o1, dGeomID o2)
{
  int i, n;
  unsigned long cat1, cat2;
  dSurfaceParameters sp;

  cat1 = dGeomGetCategoryBits(o1);
  cat2 = dGeomGetCategoryBits(o2);

  // Ignore elements in dispensers
  //TODO only ignore if completely inside
  if( (cat1==CAT_DISPENSER) && (cat2==CAT_ELEMENT) ||
      (cat2==CAT_DISPENSER) && (cat1==CAT_ELEMENT) )
  {
    dJointID c = dJointCreateSlider(physics->get_world(), physics->get_contacts());
    dJointAttach(c, dGeomGetBody(o1), dGeomGetBody(o2));
    dJointSetSliderAxis(c, 0.0, 0.0, 1.0);
    return;
  }

  // Default surface parameters
  sp.mode = dContactSlip1 | dContactSlip2 | dContactApprox1 | dContactBounce;
  sp.mu = dInfinity;
  sp.slip1 = 0.01;
  sp.slip2 = 0.01;
  sp.bounce = 0.05;
  //sp.bounce_vel = 0.010;

  dContact contacts[cfg->contacts_nb];

  // Store them to hack them later
  if( dGeomGetClass(o1)==dCylinderClass && dGeomGetClass(o2)==dCylinderClass )
  {
    ((Physics*)data)->hack_cylinders.push_back((GeomPair){o1,o2});
    return;
  }

  n = dCollide(o1, o2, cfg->contacts_nb, &contacts[0].geom, sizeof(*contacts));

  for( i=0; i<n; i++ )
  {
    dBodyID b1, b2;

    // Robots are not affected (XXX change?)
    if( (cat1&CAT_ROBOT)!=CAT_ROBOT || cat2==CAT_GROUND )
      b1 = dGeomGetBody(contacts[i].geom.g1);
    else
      b1 = 0;
    if( (cat2&CAT_ROBOT)!=CAT_ROBOT || cat1==CAT_GROUND )
      b2 = dGeomGetBody(contacts[i].geom.g2);
    else
      b2 = 0;

    memcpy(&contacts[i].surface, &sp, sizeof(sp));;
    dJointID c = dJointCreateContact(physics->get_world(), physics->get_contacts(), &contacts[i]);
    dJointAttach(c, b1, b2);
  }
}
