/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================
#ifndef X_ODE_H
#define X_ODE_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <ode/ode.h>

// shared -------------------------------------------------------------------
#include "x_shared.h"

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------




//
// Open Dynamics C++ compatibilty classes redesigned for QRazor FX / XreaL
//
class d_world_c
{
private:
	// intentionally undefined, don't use these
	d_world_c (const d_world_c &);
	void operator = (const d_world_c &);

public:
	d_world_c()					{_id = dWorldCreate();}
	~d_world_c()					{dWorldDestroy(_id);}

	dWorldID	getId()				{return _id;}
	operator dWorldID() const			{return _id;}
	
	void		setGravity(const vec3_c &g)	{dWorldSetGravity(_id, g[0], g[1], g[2]);}
	void		getGravity(vec3_c &g)		{dWorldGetGravity(_id, g);}
	
	void		setERP(vec_t erp)		{dWorldSetERP(_id, erp);}
	vec_t		getERP()			{return dWorldGetERP(_id);}
	
	void		setCFM(vec_t cfm)		{dWorldSetCFM(_id, cfm);}
	vec_t		getCFM()			{return dWorldGetCFM(_id);}
	
	void		step(vec_t step_size)		{dWorldStep(_id, step_size);}
	void		stepFast(vec_t step_size, int max_iterations)	{dWorldStepFast1(_id, step_size, max_iterations);}
	void		stepQuick(vec_t step_size)	{dWorldQuickStep(_id, step_size);}
	
	void		impulseToForce(vec_t stepsize, vec_t ix, vec_t iy, vec_t iz, dVector3 force)
	{
		dWorldImpulseToForce(_id, stepsize, ix, iy, iz, force);
	}
	
	void		setContactMaxCorrectingVel(vec_t vel)	{dWorldSetContactMaxCorrectingVel(_id, vel);}
	vec_t		getContactMaxCorrectingVel() const	{return dWorldGetContactMaxCorrectingVel(_id);}
	void		setContactSurfaceLayer(vec_t depth)	{dWorldSetContactSurfaceLayer(_id, depth);}
	vec_t		getContactSurfaceLayer() const		{return dWorldGetContactSurfaceLayer(_id);}
	
	void		setAutoEnableDepthSF1(int depth)	{dWorldSetAutoEnableDepthSF1(_id, depth);}
	int		getAutoEnableDepthSF1() const	{return dWorldGetAutoEnableDepthSF1(_id);}
	
	void		setQuickStepNumIterations(int num)	{dWorldSetQuickStepNumIterations(_id, num);}
	int		getQuickStepNumIterations() const	{return dWorldGetQuickStepNumIterations(_id);}
	void		setQuickStepW(vec_t param)		{dWorldSetQuickStepW(_id, param);}
	vec_t		getQuickStepW() const			{return dWorldGetQuickStepW(_id);}
	
	
	void		setAutoDisableLinearThreshold(vec_t threshold)	{dWorldSetAutoDisableLinearThreshold(_id, threshold);}
	vec_t		getAutoDisableLinearThreshold() const 		{return dWorldGetAutoDisableLinearThreshold(_id);}
	
	void		setAutoDisableAngularThreshold(vec_t threshold)	{dWorldSetAutoDisableAngularThreshold(_id, threshold);}
	vec_t		getAutoDisableAngularThreshold() const		{return dWorldGetAutoDisableAngularThreshold(_id);}
	
	void		setAutoDisableSteps(int steps)	{dWorldSetAutoDisableSteps(_id, steps);}
	int		getAutoDisableSteps() const	{return dWorldGetAutoDisableSteps(_id);}
	
	void		setAutoDisableTime(vec_t time)	{dWorldSetAutoDisableTime(_id, time);}
	vec_t		getAutoDisableTime() const	{return dWorldGetAutoDisableTime(_id);}
	
	void		setAutoDisableFlag(int do_auto_disable)		{dWorldSetAutoDisableFlag(_id, do_auto_disable);}
	int		getAutoDisableFlag() const			{return dWorldGetAutoDisableFlag(_id);}

private:
	dWorldID	_id;
};



class d_body_c
{
private:
	// intentionally undefined, don't use these
	d_body_c(const d_body_c &);
	void operator = (const d_body_c &);

public:
	
	d_body_c()				{_id = 0;}
	d_body_c(dWorldID world)		{_id = dBodyCreate(world);}
	~d_body_c()				{if(_id) dBodyDestroy(_id);}

	void create(dWorldID world)
	{
		if(_id)
			dBodyDestroy(_id);
		
		_id = dBodyCreate(world);
	}
	
	dBodyID		getId() const				{return _id;}
	operator dBodyID() const				{return _id;}

	void		setData(void *data)			{dBodySetData(_id,data);}
	void*		getData() const				{return dBodyGetData(_id);}

	void		setPosition(const vec3_c &pos)		{dBodySetPosition(_id, pos[0], pos[1], pos[2]);}
	void		setRotation(const vec3_c &angles)	
	{
		dMatrix3 R; 
		dRFromEulerAngles(R, angles[0], angles[1], angles[2]);
		dBodySetRotation(_id, R);
	}
	
	void		setQuaternion(const quaternion_c &q)		{dBodySetQuaternion(_id, q);}
	void		setLinearVel(const vec3_c &v)			{dBodySetLinearVel(_id, v[0], v[1], v[2]);}
	void		setLinearVel(vec_t v0, vec_t v1, vec_t v2)	{dBodySetLinearVel(_id, v0, v1, v2);}
	void		setAngularVel(const vec3_c &v)			{dBodySetAngularVel(_id, v[0], v[1], v[2]);}
	void		setAngularVel(vec_t v0, vec_t v1, vec_t v2)	{dBodySetAngularVel(_id, v0, v1, v2);}
	
	const vec_t*	getPosition() const		{return	(vec_t*)dBodyGetPosition(_id);}
	const vec_t*	getRotation() const		{return (vec_t*)dBodyGetRotation(_id);}
	const vec_t*	getQuaternion() const		{return (vec_t*)dBodyGetQuaternion(_id);}
	const vec_t*	getLinearVel() const		{return (vec_t*)dBodyGetLinearVel(_id);}
	const vec_t*	getAngularVel() const		{return (vec_t*)dBodyGetAngularVel(_id);}
	
	void		setMass(const dMass *mass)	{dBodySetMass(_id,mass);}
	void		getMass(dMass *mass) const	{dBodyGetMass(_id,mass);}
	
	void		addForce(const vec3_c &f)	{dBodyAddForce(_id, f[0], f[1], f[2]);}
	void		addTorque(const vec3_c &f)	{dBodyAddTorque(_id, f[0], f[1], f[2]);}
	void		addRelForce(const vec3_c &f)	{dBodyAddRelForce(_id, f[0], f[1], f[2]);}
	void		addRelTorque(const vec3_c &f)	{dBodyAddRelTorque(_id, f[0], f[1], f[2]);}
	void		addForceAtPos(const vec3_c &f, const vec3_c &p)		{dBodyAddForceAtPos(_id, f[0], f[1], f[2], p[0], p[1], p[2]);}
	void		addForceAtRelPos(const vec3_c &f, const vec3_c &p)	{dBodyAddForceAtRelPos(_id, f[0], f[1], f[2], p[0], p[1], p[2]);}
	void		addRelForceAtPos(const vec3_c &f, const vec3_c &p)	{dBodyAddRelForceAtPos(_id, f[0], f[1], f[2], p[0], p[1], p[2]);}
	void		addRelForceAtRelPos(const vec3_c &f, const vec3_c &p)	{dBodyAddRelForceAtRelPos(_id, f[0], f[1], f[2], p[0], p[1], p[2]);}
	
	const vec_t*	getForce() const		{return (vec_t*)dBodyGetForce(_id);}
	const vec_t*	getTorque() const		{return (vec_t*)dBodyGetTorque(_id);}
	
	void		setForce(const vec3_c &f)	{dBodySetForce(_id, f[0], f[1], f[2]);}
	void		setTorque(const vec3_c &f)	{dBodySetTorque(_id, f[0], f[1], f[2]);}
	
	void		enable()			{dBodyEnable(_id);}
	void		disable()			{dBodyDisable(_id);}
	
	int		isEnabled() const		{return dBodyIsEnabled(_id);}
	
	
	void	getRelPointPos(const vec3_c &p,	vec3_c &out) const 	{dBodyGetRelPointPos(_id, p[0], p[1], p[2], out);}
	void	getRelPointVel(const vec3_c &p, vec3_c &out) const	{dBodyGetRelPointVel(_id, p[0], p[1], p[2], out);}
	void	getPointVel(const vec3_c &p, vec3_c &out) const		{dBodyGetPointVel(_id, p[0], p[1], p[2], out);}
	void	getPosRelPoint(const vec3_c &p, vec3_c &out) const	{dBodyGetPosRelPoint(_id, p[0], p[1], p[2], out);}
	void	vectorToWorld(const vec3_c &p, vec3_c &out) const	{dBodyVectorToWorld(_id, p[0], p[1], p[2], out);}
	void	vectorFromWorld(const vec3_c &p, vec3_c &out) const	{dBodyVectorFromWorld(_id, p[0], p[1], p[2], out);}
	
	void	setFiniteRotationMode(int mode)			{dBodySetFiniteRotationMode(_id, mode);}
	void	setFiniteRotationAxis(const vec3_c &axis)	{dBodySetFiniteRotationAxis(_id, axis[0], axis[1], axis[2]);}

	int	getFiniteRotationMode() const			{return dBodyGetFiniteRotationMode(_id);}
	void	getFiniteRotationAxis(vec3_c &axis) const	{dBodyGetFiniteRotationAxis(_id, axis);}
	
	int		getNumJoints() const			{return dBodyGetNumJoints(_id);}
	dJointID	getJoint (int index) const		{return dBodyGetJoint(_id, index);}
	
	void		setGravityMode(int mode)		{dBodySetGravityMode(_id,mode);}
	int		getGravityMode() const			{return dBodyGetGravityMode(_id);}
	
	int		isConnectedTo(dBodyID body) const	{return dAreConnected (_id, body);}
	
	void	setAutoDisableLinearThreshold(vec_t threshold)	{dBodySetAutoDisableLinearThreshold(_id, threshold);}
	vec_t	getAutoDisableLinearThreshold() const		{return dBodyGetAutoDisableLinearThreshold(_id);}
	
	void	setAutoDisableAngularThreshold(dReal threshold)	{dBodySetAutoDisableAngularThreshold(_id, threshold);}
	vec_t	getAutoDisableAngularThreshold() const		{return dBodyGetAutoDisableAngularThreshold(_id);}
	
	void	setAutoDisableSteps(int steps)			{dBodySetAutoDisableSteps(_id, steps);}
	int	getAutoDisableSteps() const			{return dBodyGetAutoDisableSteps(_id);}
	
	void	setAutoDisableTime(vec_t time)			{dBodySetAutoDisableTime(_id, time);}
	vec_t	getAutoDisableTime() const			{return dBodyGetAutoDisableTime(_id);}
	
	void	setAutoDisableFlag(int do_auto_disable)		{dBodySetAutoDisableFlag(_id, do_auto_disable);}
	int	getAutoDisableFlag() const			{return dBodyGetAutoDisableFlag(_id);}
	
private:
	dBodyID _id;
};


class	d_joint_group_c
{
	 dJointGroupID _id;
	 
	// intentionally undefined, don't use these
	d_joint_group_c(const d_joint_group_c &);
	void operator = (const d_joint_group_c &);

public:
	d_joint_group_c(int dummy_arg = 0)		{_id = dJointGroupCreate(0);}
	~d_joint_group_c()				{dJointGroupDestroy(_id);}
	
	void		create(int dummy_arg=0)
	{
		if(_id)
			dJointGroupDestroy(_id);
			
		_id = dJointGroupCreate(0);
	}
	
	dJointGroupID	getId() const			{return _id;}
	
	operator dJointGroupID() const			{return _id;}
	
	void	empty()					{dJointGroupEmpty(_id);}
};


class d_joint_c
{
private:
	// intentionally undefined, don't use these
	d_joint_c(const d_joint_c &);
	void operator = (const d_joint_c &);

protected:
	dJointID _id;

public:
	d_joint_c()					{_id = 0;}
	~d_joint_c()					{if (_id) dJointDestroy(_id); }
	
	dJointID	getId() const			{return _id;}
	
	operator dJointID() const			{return _id;}

	void	attach(dBodyID body1, dBodyID body2)	{dJointAttach(_id, body1, body2);}

	void	setData(void *data)			{dJointSetData(_id, data);}
	void*	getData (void *data) const		{return dJointGetData(_id);}
	
	int	getType() const				{return dJointGetType (_id);}

	dBodyID getBody (int index) const		{return dJointGetBody (_id, index);}
};


class d_ball_joint_c : public d_joint_c
{
private:
 	// intentionally undefined, don't use these
	d_ball_joint_c(const d_ball_joint_c &);
	void operator = (const d_ball_joint_c &);

public:
 	d_ball_joint_c() 					{}
	d_ball_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateBall(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)	
	{
		if(_id)
			dJointDestroy(_id);
			
		_id = dJointCreateBall (world, group);
	}
	
	void	setAnchor(const vec3_c &v)		{dJointSetBallAnchor(_id, v[0], v[1], v[2]);}
	void	getAnchor(vec3_c &result) const		{dJointGetBallAnchor(_id, result); }
	
	void	getAnchor2(vec3_c &result) const	{dJointGetBallAnchor2(_id, result);}
};


class d_hinge_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_hinge_joint_c (const d_hinge_joint_c &);
	void operator = (const d_hinge_joint_c &);

public:
	d_hinge_joint_c()					{}
	d_hinge_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateHinge (world, group);}

	void	create(dWorldID world, dJointGroupID group=0)	
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateHinge (world, group);
	}
	
	void	setAnchor(const vec3_c &v)		{dJointSetHingeAnchor(_id, v[0], v[1], v[2]);}
	void	getAnchor(vec3_c &result) const		{dJointGetHingeAnchor(_id, result);}
	
	void	getAnchor2(vec3_c &result) const	{dJointGetHingeAnchor2(_id, result);}

	void	setAxis(const vec3_c &v)		{dJointSetHingeAxis(_id, v[0], v[1], v[2]);}
	void	getAxis(vec3_c &result) const		{dJointGetHingeAxis(_id, result);}

	vec_t	getAngle() const			{return dJointGetHingeAngle(_id);}
	vec_t	getAngleRate() const			{return dJointGetHingeAngleRate(_id);}
	
	void	setParam(int parameter, vec_t value)	{dJointSetHingeParam(_id, parameter, value);}
	vec_t	getParam(int parameter) const		{return dJointGetHingeParam(_id, parameter);}
	
	void	addTorque(vec_t torque)			{dJointAddHingeTorque(_id, torque);}
};


class d_slider_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_slider_joint_c (const d_slider_joint_c &);
	void operator = (const d_slider_joint_c &);

public:
	d_slider_joint_c()					{}
	d_slider_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateSlider(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateSlider(world, group);
	}
	
	void	setAxis(const vec3_c &v)		{dJointSetSliderAxis(_id, v[0], v[1], v[2]);}
	void	getAxis(vec3_c &result) const		{dJointGetSliderAxis(_id, result);}

	vec_t	getPosition() const			{return dJointGetSliderPosition(_id);}
	vec_t	getPositionRate() const			{return dJointGetSliderPositionRate(_id);}

	void	setParam(int parameter, vec_t value)	{dJointSetSliderParam(_id, parameter, value);}
	vec_t	getParam(int parameter) const		{return dJointGetSliderParam(_id, parameter);}
	
	void	addForce(vec_t force)			{dJointAddSliderForce(_id, force);}
};


class d_universal_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_universal_joint_c (const d_universal_joint_c &);
	void operator = (const d_universal_joint_c &);

public:
	d_universal_joint_c()					{}
	d_universal_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateUniversal(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)	
	{
		if(_id)
			dJointDestroy(_id);
		
		 _id = dJointCreateUniversal(world, group);
	}
	
	void	setAnchor(const vec3_c &v)		{dJointSetUniversalAnchor(_id, v[0], v[1], v[2]);}
	void	setAxis1(const vec3_c &v)		{dJointSetUniversalAxis1(_id, v[0], v[1], v[2]);}
	void	setAxis2(const vec3_c &v)		{dJointSetUniversalAxis2(_id, v[0], v[1], v[2]);}
	void	setParam(int parameter, vec_t value)	{dJointSetUniversalParam(_id, parameter, value);}
	
	void	getAnchor(vec3_c& result) const		{dJointGetUniversalAnchor(_id, result);}
	void	getAnchor2(vec3_c &result) const	{dJointGetUniversalAnchor2(_id, result);}
	void	getAxis1(vec3_c &result) const		{dJointGetUniversalAxis1(_id, result);}
	void	getAxis2(vec3_c &result) const		{dJointGetUniversalAxis2(_id, result);}
	vec_t	getParam(int parameter) const		{return dJointGetUniversalParam(_id, parameter);}
	vec_t	getAngle1() const			{return dJointGetUniversalAngle1(_id);}
	vec_t	getAngle1Rate() const			{return dJointGetUniversalAngle1Rate(_id);}
	vec_t	getAngle2() const			{return dJointGetUniversalAngle2(_id);}
	vec_t	getAngle2Rate() const			{return dJointGetUniversalAngle2Rate(_id);}
	
	void	addTorques(vec_t torque1, vec_t torque2){dJointAddUniversalTorques(_id, torque1, torque2);}
};


class d_hinge2_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_hinge2_joint_c (const d_hinge2_joint_c &);
	void operator = (const d_hinge2_joint_c &);

public:
	d_hinge2_joint_c()					{}
	d_hinge2_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateHinge2(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateHinge2(world, group);
	}
	
	void	setAnchor(const vec3_c &v)		{dJointSetHinge2Anchor(_id, v[0], v[1], v[2]);}
	void	setAxis1(const vec3_c &v)		{dJointSetHinge2Axis1(_id, v[0], v[1], v[2]);}
	void	setAxis2(const vec3_c &v)		{dJointSetHinge2Axis2(_id, v[0], v[1], v[2]);}

	void	getAnchor(vec3_c &result) const		{dJointGetHinge2Anchor(_id, result);}
	void	getAnchor2(vec3_c &result) const	{dJointGetHinge2Anchor2(_id, result);}
	void	getAxis1(vec3_c &result) const		{dJointGetHinge2Axis1(_id, result);}
	void	getAxis2(vec3_c &result) const		{dJointGetHinge2Axis2(_id, result);}
	vec_t	getAngle1() const			{return dJointGetHinge2Angle1(_id);}
	vec_t	getAngle1Rate() const			{return dJointGetHinge2Angle1Rate(_id);}
	vec_t	getAngle2Rate() const			{return dJointGetHinge2Angle2Rate(_id);}
	
	void	setParam(int parameter, vec_t value)	{dJointSetHinge2Param(_id, parameter, value);}
	vec_t	getParam(int parameter) const		{return dJointGetHinge2Param(_id, parameter);}
	
	void	addTorques(vec_t torque1, vec_t torque2){dJointAddHinge2Torques(_id, torque1, torque2);}
};


class d_fixed_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_fixed_joint_c (const d_fixed_joint_c &);
	void operator = (const d_fixed_joint_c &);

public:
	d_fixed_joint_c()					{}
	d_fixed_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateFixed(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateFixed(world, group);
	}
	
	void	set()					{dJointSetFixed(_id);}
};


class d_contact_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_contact_joint_c (const d_contact_joint_c &);
	void operator = (const d_contact_joint_c &);

public:
	d_contact_joint_c()					{}
	d_contact_joint_c(dWorldID world, dJointGroupID group, dContact *contact)	{_id = dJointCreateContact(world, group, contact);}

	void	create(dWorldID world, dJointGroupID group, dContact *contact)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateContact(world, group, contact);
	}
};


class d_null_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_null_joint_c (const d_null_joint_c &);
	void operator = (const d_null_joint_c &);

public:
	d_null_joint_c()					{}
	d_null_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateNull(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateNull(world, group);
	}
};


class d_amotor_joint_c : public d_joint_c
{
	// intentionally undefined, don't use these
	d_amotor_joint_c (const d_amotor_joint_c &);
	void operator = (const d_amotor_joint_c &);

public:
	d_amotor_joint_c()					{}
	d_amotor_joint_c(dWorldID world, dJointGroupID group=0)	{_id = dJointCreateAMotor(world, group);}

	void	create(dWorldID world, dJointGroupID group=0)
	{
		if(_id)
			dJointDestroy(_id);
		
		_id = dJointCreateAMotor(world, group);
	}
	
	void	setMode(int mode)			{dJointSetAMotorMode(_id, mode);}
	int	getMode() const				{return dJointGetAMotorMode(_id);}
	
	void	setNumAxes(int num)			{dJointSetAMotorNumAxes(_id, num);}
	int	getNumAxes() const			{return dJointGetAMotorNumAxes(_id);}
	
	void	setAxis(int anum, int rel, const vec3_c &v)	{dJointSetAMotorAxis(_id, anum, rel, v[0], v[1], v[2]);}
	void	getAxis(int anum, vec3_c &result) const		{dJointGetAMotorAxis(_id, anum, result);}
	
	int	getAxisRel(int anum) const		{return dJointGetAMotorAxisRel(_id, anum);}
	
	void	setAngle(int anum, vec_t angle)		{dJointSetAMotorAngle(_id, anum, angle);}
	vec_t	getAngle(int anum) const		{return dJointGetAMotorAngle(_id, anum);}
	
	vec_t	getAngleRate(int anum)			{return dJointGetAMotorAngleRate(_id,anum);}
	
	void	setParam(int parameter, vec_t value)	{dJointSetAMotorParam(_id, parameter, value);}
	vec_t	getParam(int parameter) const		{return dJointGetAMotorParam(_id, parameter);}
	
	void	addTorques(vec_t torque1, vec_t torque2, vec_t torque3)	{dJointAddAMotorTorques(_id, torque1, torque2, torque3);}
};


class d_geom_c
{
	// intentionally undefined, don't use these
	d_geom_c (d_geom_c &);
	void operator = (d_geom_c &);

protected:
	dGeomID _id;

public:
	d_geom_c()						{_id = 0;}
	~d_geom_c()						{if(_id) dGeomDestroy(_id);}
	
	dGeomID	getId() const					{return _id;}
	
	operator dGeomID() const				{return _id;}

	void	destroy()
	{
		if(_id)
			dGeomDestroy(_id);
			
		_id = 0;
	}

	int	getClass() const				{return dGeomGetClass(_id);}
	
	void	setData(void *data)				{dGeomSetData(_id,data);}
	void*	getData() const					{return dGeomGetData(_id);}
	
	void	setBody(dBodyID b)				{dGeomSetBody(_id,b);}
	dBodyID	getBody() const					{return dGeomGetBody(_id);}
	
	void	setPosition(const vec3_c &v)			{dGeomSetPosition(_id, v[0], v[1], v[2]);}
	const vec_t* getPosition() const			{return (vec_t*)dGeomGetPosition(_id);}
	
	void	setRotation(const dMatrix3 R)			{dGeomSetRotation(_id,R);}
	void	setRotation(const vec3_c &angles)
	{
		dMatrix3 R; 
		dRFromEulerAngles(R, angles[0], angles[1], angles[2]);
		dGeomSetRotation(_id, R);
	}
	const vec_t* getRotation() const			{return (vec_t*)dGeomGetRotation(_id);}
	void	setQuaternion(quaternion_c &quat)		{dGeomSetQuaternion(_id, quat);}
	void	getQuaternion(quaternion_c &quat) const		{dGeomGetQuaternion(_id, quat);}

//	void	getAABB(vec_t aabb[6]) const			{dGeomGetAABB(_id, aabb);}
	
	int	isSpace()					{return dGeomIsSpace(_id);}

	void	setCategoryBits(unsigned long bits)		{dGeomSetCategoryBits(_id, bits);}
	void	setCollideBits(unsigned long bits)		{dGeomSetCollideBits(_id, bits);}
	
	unsigned long	getCategoryBits()			{return dGeomGetCategoryBits(_id);}
	unsigned long	getCollideBits()			{return dGeomGetCollideBits(_id);}
	
	void	enable()					{dGeomEnable(_id);}
	void	disable()					{dGeomDisable(_id);}
	
	int	isEnabled()					{return dGeomIsEnabled(_id);}
	
	void	collide2(dGeomID g, void *data, dNearCallback *callback)	{dSpaceCollide2(_id, g, data, callback);}
};


class d_space_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_space_c (d_space_c &);
	void operator = (d_space_c &);

protected:
	// the default constructor is protected so that you
	// can't instance this class. you must instance one
	// of its subclasses instead.
	d_space_c ()						{_id = 0;}

public:
	dSpaceID getId() const					{return (dSpaceID) _id;}
	operator dSpaceID() const				{return (dSpaceID) _id;}

	void	setCleanup(int mode)				{dSpaceSetCleanup(getId(), mode);}
	int	getCleanup()					{return dSpaceGetCleanup(getId());}
	
	void	add(dGeomID x)					{dSpaceAdd(getId(), x);}
	void	remove(dGeomID x)				{dSpaceRemove(getId(), x);}
	int	query(dGeomID x)				{return dSpaceQuery(getId(),x);}

	int	getNumGeoms()					{return dSpaceGetNumGeoms(getId());}
	dGeomID getGeom(int i)					{return dSpaceGetGeom(getId(), i);}
	
	void	collide(void *data, dNearCallback *callback)	{dSpaceCollide(getId(), data, callback);}
};


class d_simple_space_c : public d_space_c 
{
	// intentionally undefined, don't use these
	d_simple_space_c (d_simple_space_c &);
	void operator = (d_simple_space_c &);

public:
	d_simple_space_c(dSpaceID space = 0)			{ _id = (dGeomID) dSimpleSpaceCreate(space);}
};


class d_hash_space_c : public d_space_c
{
	// intentionally undefined, don't use these
	d_hash_space_c (d_hash_space_c &);
	void operator = (d_hash_space_c &);

public:
	d_hash_space_c(dSpaceID space = 0)			{ _id = (dGeomID) dHashSpaceCreate(space);}
	void	setLevels(int minlevel, int maxlevel)		{ dHashSpaceSetLevels(getId(), minlevel, maxlevel);}
};


class d_quadtree_space_c : public d_space_c
{
	// intentionally undefined, don't use these
	d_quadtree_space_c(d_quadtree_space_c &);
	void operator = (d_quadtree_space_c &);

public:
	d_quadtree_space_c(dSpaceID space, const vec3_c &center, const vec3_c &extents, int depth)
	{
		_id = (dGeomID)dQuadTreeSpaceCreate(space, center, extents, depth);
	}
};

class d_sphere_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_sphere_c (d_sphere_c &);
	void operator= (d_sphere_c &);

public:
	d_sphere_c()						{}
	d_sphere_c(dSpaceID space, vec_t radius)		{ _id = dCreateSphere(space, radius);}
	
	void	create(dSpaceID space, vec_t radius)
	{
		if(_id)
			 dGeomDestroy(_id);
			 
		 _id = dCreateSphere(space, radius);
	}
	
	void	setRadius(vec_t radius)				{dGeomSphereSetRadius(_id, radius);}
	vec_t	getRadius() const				{return dGeomSphereGetRadius(_id);}
};


class d_box_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_box_c (d_box_c &);
	void operator = (d_box_c &);

public:
	d_box_c()						{}
	d_box_c(dSpaceID space, vec_t x, vec_t y, vec_t z)	{_id = dCreateBox(space, x, y, z);}
	d_box_c(dSpaceID space, const vec3_c &v)		{_id = dCreateBox(space, v[0], v[1], v[2]);}
	
	void	create(dSpaceID space, const vec3_c &v)
	{
		if(_id)
		 	dGeomDestroy(_id);
		
		_id = dCreateBox(space, v[0], v[1], v[2]);
	}
	
	void	setLengths(const vec3_c &v)			{dGeomBoxSetLengths(_id, v[0], v[1], v[2]);}
	void	getLengths(vec3_c &result) const		{dGeomBoxGetLengths(_id, result);}
};


class d_plane_c : public d_geom_c 
{
	// intentionally undefined, don't use these
	d_plane_c (d_plane_c &);
	void operator= (d_plane_c &);

public:
	d_plane_c() 						{}
	d_plane_c(dSpaceID space, const vec3_c &n, vec_t d)	{ _id = dCreatePlane(space, n[0], n[1], n[2], d);}

	void	create(dSpaceID space, const vec3_c &n, vec_t d)
	{
		if(_id)
			dGeomDestroy(_id);
		
		_id = dCreatePlane(space, n[0], n[1], n[2], d);
  }

	void	setParams(vec_t a, vec_t b, vec_t c, vec_t d)	{dGeomPlaneSetParams(_id, a, b, c, d);}
	void	getParams(vec4_c &result) const			{dGeomPlaneGetParams(_id, result);}
};


class d_ccylinder_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_ccylinder_c (d_ccylinder_c &);
	void operator = (d_ccylinder_c &);

public:
	d_ccylinder_c()						{}
	d_ccylinder_c(dSpaceID space, vec_t radius, vec_t length)	{_id = dCreateCCylinder(space, radius, length);}
	
	void	create(dSpaceID space, vec_t radius, vec_t length)
	{
		if(_id)
			dGeomDestroy(_id);
			
		_id = dCreateCCylinder(space, radius, length);
	}
	
	void	setParams(vec_t radius, vec_t length)		{dGeomCCylinderSetParams(_id, radius, length);}
	void	getParams(vec_t *radius, vec_t *length) const	{dGeomCCylinderGetParams(_id, (dReal*)radius, (dReal*)length);}
};


class d_ray_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_ray_c (d_ray_c &);
	void operator = (d_ray_c &);

public:
	//d_ray_c()						{}
	d_ray_c(dSpaceID space, const vec3_c &pos, const vec3_c &dir, vec_t length)
	{
		_id = dCreateRay(space, length);
		set(pos, dir);
	}
	
	void	create(dSpaceID space, vec_t radius, vec_t length)
	{
		if(_id)
			dGeomDestroy(_id);
			
		_id = dCreateRay(space, length);
	}
	
	void	set(const vec3_c &pos, const vec3_c &dir)	{dGeomRaySet(_id, pos[0], pos[1], pos[2], dir[0], dir[1], dir[2]);}
	void	get(vec3_c &pos, vec3_c &dir) const		{dGeomRayGet(_id, pos, dir);}
	
	void	setLength(vec_t length)				{dGeomRaySetLength(_id, length);}
	vec_t	getLength() const				{return dGeomRayGetLength(_id);}
	
	void	setParams(int first_contact, int backface_cull)		{dGeomRaySetParams(_id, first_contact, backface_cull);}
	void	getParams(int *first_contact, int *backface_cull) const	{dGeomRayGetParams(_id, first_contact, backface_cull);}
	
	void	setClosestHit(int closest_hit)			{dGeomRaySetClosestHit(_id, closest_hit);}
	int	getClosestHit()	const				{return dGeomRayGetClosestHit(_id);}
};


class d_geom_transform_c : public d_geom_c 
{
	// intentionally undefined, don't use these
	d_geom_transform_c(d_geom_transform_c &);
	void operator = (d_geom_transform_c &);

public:
	d_geom_transform_c()					{}
	d_geom_transform_c(dSpaceID space)			{ _id = dCreateGeomTransform(space);}
	
	void	create(dSpaceID space=0)
	{
		if(_id)
			dGeomDestroy(_id);
		
		_id = dCreateGeomTransform(space);
	}
	
	void	setGeom(dGeomID geom)				{dGeomTransformSetGeom(_id, geom);}
	dGeomID getGeom() const					{return dGeomTransformGetGeom(_id);}
	
	void	setCleanup(int mode)				{dGeomTransformSetCleanup(_id, mode);}
	int	getCleanup(dGeomID g)				{return dGeomTransformGetCleanup(_id);}
	
	void	setInfo(int mode)				{dGeomTransformSetInfo(_id, mode);}
	int	getInfo()					{return dGeomTransformGetInfo(_id);}
};


class d_trimesh_c : public d_geom_c
{
	// intentionally undefined, don't use these
	d_trimesh_c(d_trimesh_c &);
	void operator = (d_trimesh_c &);

public:
	d_trimesh_c()						{}
	d_trimesh_c(dSpaceID space, const std::vector<vec3_c> &vertexes, const std::vector<index_t> &indexes)
	{
		_data = dGeomTriMeshDataCreate();
		dGeomTriMeshDataBuildSingle(_data, &vertexes[0], sizeof(vec3_c), vertexes.size(), &indexes[0], indexes.size(), 3 * sizeof(index_t));
	
		_id = dCreateTriMesh(space, _data, NULL, NULL, NULL);
	}
	
	void	create(dSpaceID space, const std::vector<vec3_c> &vertexes, const std::vector<index_t> &indexes)
	{
		if(_id)
			dGeomDestroy(_id);
			
		if(_data)
			dGeomTriMeshDataDestroy(_data);
			
		_data = dGeomTriMeshDataCreate();	
		dGeomTriMeshDataBuildSingle(_data, &vertexes[0], sizeof(vec3_c), vertexes.size(), &indexes[0], indexes.size(), 3 * sizeof(index_t));
		
		_id = dCreateTriMesh(space, _data, NULL, NULL, NULL);
	}
	
	
	
private:
	dTriMeshDataID	_data;
};



#endif // X_ODE_H

