/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef ODE_OBJECTS_H
#define ODE_OBJECTS_H

#include "common.h"
#include "mass.h"
#include "contact.h"


/* world */
dWorldID	dWorldCreate();
void		dWorldDestroy(dWorldID);

void		dWorldSetGravity (dWorldID, vec_t x, vec_t y, vec_t z);
void		dWorldGetGravity (dWorldID, dVector3 gravity);
void		dWorldSetERP (dWorldID, vec_t erp);
vec_t		dWorldGetERP (dWorldID);
void		dWorldSetCFM (dWorldID, vec_t cfm);
vec_t		dWorldGetCFM (dWorldID);
void		dWorldStep (dWorldID, vec_t stepsize);
void		dWorldImpulseToForce (dWorldID, vec_t stepsize, vec_t ix, vec_t iy, vec_t iz, dVector3 force);

/* World QuickStep functions */
void		dWorldQuickStep (dWorldID w, vec_t stepsize);
void		dWorldSetQuickStepNumIterations (dWorldID, int num);
int		dWorldGetQuickStepNumIterations (dWorldID);
void		dWorldSetQuickStepW (dWorldID, vec_t param);
vec_t		dWorldGetQuickStepW (dWorldID);

/* World contact parameter functions */
void		dWorldSetContactMaxCorrectingVel (dWorldID, vec_t vel);
vec_t		dWorldGetContactMaxCorrectingVel (dWorldID);
void		dWorldSetContactSurfaceLayer (dWorldID, vec_t depth);
vec_t		dWorldGetContactSurfaceLayer (dWorldID);

/* Auto-disable functions */
vec_t		dWorldGetAutoDisableLinearThreshold (dWorldID);
void		dWorldSetAutoDisableLinearThreshold (dWorldID, vec_t linear_threshold);
vec_t		dWorldGetAutoDisableAngularThreshold (dWorldID);
void		dWorldSetAutoDisableAngularThreshold (dWorldID, vec_t angular_threshold);
int		dWorldGetAutoDisableSteps (dWorldID);
void		dWorldSetAutoDisableSteps (dWorldID, int steps);
vec_t		dWorldGetAutoDisableTime (dWorldID);
void		dWorldSetAutoDisableTime (dWorldID, vec_t time);
int		dWorldGetAutoDisableFlag (dWorldID);
void		dWorldSetAutoDisableFlag (dWorldID, int do_auto_disable);

vec_t		dBodyGetAutoDisableLinearThreshold (dBodyID);
void		dBodySetAutoDisableLinearThreshold (dBodyID, vec_t linear_threshold);
vec_t		dBodyGetAutoDisableAngularThreshold (dBodyID);
void		dBodySetAutoDisableAngularThreshold (dBodyID, vec_t angular_threshold);
int		dBodyGetAutoDisableSteps (dBodyID);
void		dBodySetAutoDisableSteps (dBodyID, int steps);
vec_t		dBodyGetAutoDisableTime (dBodyID);
void		dBodySetAutoDisableTime (dBodyID, vec_t time);
int		dBodyGetAutoDisableFlag (dBodyID);
void		dBodySetAutoDisableFlag (dBodyID, int do_auto_disable);
void		dBodySetAutoDisableDefaults (dBodyID);

/* bodies */
dBodyID dBodyCreate (dWorldID);
void dBodyDestroy (dBodyID);

void  dBodySetData (dBodyID, void *data);
void *dBodyGetData (dBodyID);

void dBodySetPosition   (dBodyID, vec_t x, vec_t y, vec_t z);
void dBodySetRotation   (dBodyID, const matrix_c &R);
void	dBodySetQuaternion(dBodyID, const quaternion_c &q);
void dBodySetLinearVel  (dBodyID, vec_t x, vec_t y, vec_t z);
void dBodySetAngularVel (dBodyID, vec_t x, vec_t y, vec_t z);
const vec_t * dBodyGetPosition   (dBodyID);
const vec_t * dBodyGetRotation   (dBodyID);	/* ptr to 4x3 rot matrix */
const quaternion_c&	dBodyGetQuaternion(dBodyID);
const vec_t * dBodyGetLinearVel  (dBodyID);
const vec_t * dBodyGetAngularVel (dBodyID);

void dBodySetMass (dBodyID, const dMass *mass);
void dBodyGetMass (dBodyID, dMass *mass);

void dBodyAddForce            (dBodyID, vec_t fx, vec_t fy, vec_t fz);
void dBodyAddTorque           (dBodyID, vec_t fx, vec_t fy, vec_t fz);
void dBodyAddRelForce         (dBodyID, vec_t fx, vec_t fy, vec_t fz);
void dBodyAddRelTorque        (dBodyID, vec_t fx, vec_t fy, vec_t fz);
void dBodyAddForceAtPos       (dBodyID, vec_t fx, vec_t fy, vec_t fz,
			                vec_t px, vec_t py, vec_t pz);
void dBodyAddForceAtRelPos    (dBodyID, vec_t fx, vec_t fy, vec_t fz,
			                vec_t px, vec_t py, vec_t pz);
void dBodyAddRelForceAtPos    (dBodyID, vec_t fx, vec_t fy, vec_t fz,
			                vec_t px, vec_t py, vec_t pz);
void dBodyAddRelForceAtRelPos (dBodyID, vec_t fx, vec_t fy, vec_t fz,
			                vec_t px, vec_t py, vec_t pz);


const vec_t * dBodyGetForce   (dBodyID);
const vec_t * dBodyGetTorque  (dBodyID);
void dBodySetForce  (dBodyID b, vec_t x, vec_t y, vec_t z);
void dBodySetTorque (dBodyID b, vec_t x, vec_t y, vec_t z);

void dBodyGetRelPointPos    (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);
void dBodyGetRelPointVel    (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);
void dBodyGetPointVel       (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);
void dBodyGetPosRelPoint    (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);
void dBodyVectorToWorld     (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);
void dBodyVectorFromWorld   (dBodyID, vec_t px, vec_t py, vec_t pz,
			     dVector3 result);

void dBodySetFiniteRotationMode (dBodyID, int mode);
void dBodySetFiniteRotationAxis (dBodyID, vec_t x, vec_t y, vec_t z);

int dBodyGetFiniteRotationMode (dBodyID);
void dBodyGetFiniteRotationAxis (dBodyID, dVector3 result);

int dBodyGetNumJoints (dBodyID b);
dJointID dBodyGetJoint (dBodyID, int index);

void dBodyEnable (dBodyID);
void dBodyDisable (dBodyID);
int dBodyIsEnabled (dBodyID);

void dBodySetGravityMode (dBodyID b, int mode);
int dBodyGetGravityMode (dBodyID b);


/* joints */

dJointID dJointCreateBall (dWorldID, dJointGroupID);
dJointID dJointCreateHinge (dWorldID, dJointGroupID);
dJointID dJointCreateSlider (dWorldID, dJointGroupID);
dJointID dJointCreateContact (dWorldID, dJointGroupID, const dContact *);
dJointID dJointCreateHinge2 (dWorldID, dJointGroupID);
dJointID dJointCreateUniversal (dWorldID, dJointGroupID);
dJointID dJointCreateFixed (dWorldID, dJointGroupID);
dJointID dJointCreateNull (dWorldID, dJointGroupID);
dJointID dJointCreateAMotor (dWorldID, dJointGroupID);
dJointID dJointCreatePlane2D (dWorldID, dJointGroupID);

void dJointDestroy (dJointID);

dJointGroupID dJointGroupCreate (int max_size);
void dJointGroupDestroy (dJointGroupID);
void dJointGroupEmpty (dJointGroupID);

void dJointAttach (dJointID, dBodyID body1, dBodyID body2);
void dJointSetData (dJointID, void *data);
void *dJointGetData (dJointID);
int dJointGetType (dJointID);
dBodyID dJointGetBody (dJointID, int index);

void dJointSetFeedback (dJointID, dJointFeedback *);
dJointFeedback *dJointGetFeedback (dJointID);

void dJointSetBallAnchor (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetBallAnchor2 (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHingeAnchor (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHingeAnchorDelta (dJointID, vec_t x, vec_t y, vec_t z, vec_t ax, vec_t ay, vec_t az);
void dJointSetHingeAxis (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHingeParam (dJointID, int parameter, vec_t value);
void dJointAddHingeTorque(dJointID joint, vec_t torque);
void dJointSetSliderAxis (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetSliderAxisDelta (dJointID, vec_t x, vec_t y, vec_t z, vec_t ax, vec_t ay, vec_t az);
void dJointSetSliderParam (dJointID, int parameter, vec_t value);
void dJointAddSliderForce(dJointID joint, vec_t force);
void dJointSetHinge2Anchor (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHinge2Axis1 (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHinge2Axis2 (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetHinge2Param (dJointID, int parameter, vec_t value);
void dJointAddHinge2Torques(dJointID joint, vec_t torque1, vec_t torque2);
void dJointSetUniversalAnchor (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetUniversalAxis1 (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetUniversalAxis2 (dJointID, vec_t x, vec_t y, vec_t z);
void dJointSetUniversalParam (dJointID, int parameter, vec_t value);
void dJointAddUniversalTorques(dJointID joint, vec_t torque1, vec_t torque2);
void dJointSetFixed (dJointID);
void dJointSetAMotorNumAxes (dJointID, int num);
void dJointSetAMotorAxis (dJointID, int anum, int rel,
			  vec_t x, vec_t y, vec_t z);
void dJointSetAMotorAngle (dJointID, int anum, vec_t angle);
void dJointSetAMotorParam (dJointID, int parameter, vec_t value);
void dJointSetAMotorMode (dJointID, int mode);
void dJointAddAMotorTorques (dJointID, vec_t torque1, vec_t torque2, vec_t torque3);
void dJointSetPlane2DXParam (dJointID, int parameter, vec_t value);
void dJointSetPlane2DYParam (dJointID, int parameter, vec_t value);
void dJointSetPlane2DAngleParam (dJointID, int parameter, vec_t value);

void dJointGetBallAnchor (dJointID, dVector3 result);
void dJointGetBallAnchor2 (dJointID, dVector3 result);
void dJointGetHingeAnchor (dJointID, dVector3 result);
void dJointGetHingeAnchor2 (dJointID, dVector3 result);
void dJointGetHingeAxis (dJointID, dVector3 result);
vec_t dJointGetHingeParam (dJointID, int parameter);
vec_t dJointGetHingeAngle (dJointID);
vec_t dJointGetHingeAngleRate (dJointID);
vec_t dJointGetSliderPosition (dJointID);
vec_t dJointGetSliderPositionRate (dJointID);
void dJointGetSliderAxis (dJointID, dVector3 result);
vec_t dJointGetSliderParam (dJointID, int parameter);
void dJointGetHinge2Anchor (dJointID, dVector3 result);
void dJointGetHinge2Anchor2 (dJointID, dVector3 result);
void dJointGetHinge2Axis1 (dJointID, dVector3 result);
void dJointGetHinge2Axis2 (dJointID, dVector3 result);
vec_t dJointGetHinge2Param (dJointID, int parameter);
vec_t dJointGetHinge2Angle1 (dJointID);
vec_t dJointGetHinge2Angle1Rate (dJointID);
vec_t dJointGetHinge2Angle2Rate (dJointID);
void dJointGetUniversalAnchor (dJointID, dVector3 result);
void dJointGetUniversalAnchor2 (dJointID, dVector3 result);
void dJointGetUniversalAxis1 (dJointID, dVector3 result);
void dJointGetUniversalAxis2 (dJointID, dVector3 result);
vec_t dJointGetUniversalParam (dJointID, int parameter);
vec_t dJointGetUniversalAngle1 (dJointID);
vec_t dJointGetUniversalAngle2 (dJointID);
vec_t dJointGetUniversalAngle1Rate (dJointID);
vec_t dJointGetUniversalAngle2Rate (dJointID);
int dJointGetAMotorNumAxes (dJointID);
void dJointGetAMotorAxis (dJointID, int anum, dVector3 result);
int dJointGetAMotorAxisRel (dJointID, int anum);
vec_t dJointGetAMotorAngle (dJointID, int anum);
vec_t dJointGetAMotorAngleRate (dJointID, int anum);
vec_t dJointGetAMotorParam (dJointID, int parameter);
int dJointGetAMotorMode (dJointID);

dJointID dConnectingJoint (dBodyID, dBodyID);
int dConnectingJointList (dBodyID, dBodyID, dJointID*);
int dAreConnected (dBodyID, dBodyID);
int dAreConnectedExcluding (dBodyID, dBodyID, int joint_type);

#endif
