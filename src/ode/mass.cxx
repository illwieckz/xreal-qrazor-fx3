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

#include "config.h"
#include "mass.h"
#include "odemath.h"
#include "matrix.h"





void	dMass::setZero()
{
	_mass = REAL(0.0);
	
	dSetZero(_c, sizeof(_c) / sizeof(vec_t));
	
	_I.zero();
}


void	dMass::setParameters(vec_t themass,
			 	vec_t cgx, vec_t cgy, vec_t cgz,
			 	vec_t I11, vec_t I22, vec_t I33,
			 	vec_t I12, vec_t I13, vec_t I23)
{
	setZero();
	
	_mass = themass;
	
	_c[0] = cgx;
	_c[1] = cgy;
	_c[2] = cgz;
	
	_I[0][0] = I11;	_I[0][1] = I12;	_I[0][2] = I13;
	_I[1][0] = I12;	_I[1][1] = I22;	_I[1][2] = I23;
	_I[2][0] = I13;	_I[2][1] = I23;	_I[2][2] = I33;
	
//#if DEBUG
	check();
//#endif
}

void	dMass::setSphere(vec_t density, vec_t radius)
{
	setSphereTotal((REAL(4.0)/REAL(3.0)) * M_PI * radius*radius*radius * density, radius);
}

void	dMass::setSphereTotal(vec_t total_mass, vec_t radius)
{
	setZero();
	
	_mass = total_mass;
	
	vec_t II = REAL(0.4) * total_mass * radius*radius;
	
	_I[0][0] = II;
				_I[1][1] = II;
							_I[2][2] = II;
#if DEBUG
	check();
#endif
}

/*
void	dMass::setCappedCylinder(vec_t density, int direction, vec_t radius, vec_t length)
{
	dUASSERT(direction >= 1 && direction <= 3, "bad direction number");
	
	setZero();
	
	vec_t M1, M2, Ia, Ib;
	
	M1 = M_PI*radius*radius*length*density;				// cylinder mass
	M2 = (REAL(4.0)/REAL(3.0))*M_PI*radius*radius*radius*density;	// total cap mass
	
	_mass = M1+M2;
	Ia =	M1*(REAL(0.25)*radius*radius + (REAL(1.0)/REAL(12.0))*length*length) +
		M2*(REAL(0.4)*radius*radius + REAL(0.375)*radius*length + REAL(0.25)*length*length);
		
	Ib = (M1*REAL(0.5) + M2*REAL(0.4))*radius*radius;
	
	_I[0][0] = Ia;
				_I[1][1] = Ia;
							_I[2][2] = Ia;
	_I[direction-1][direction-1] = Ib;
	
#if DEBUG
	check();
#endif
}


void	dMass::setCappedCylinderTotal(vec_t total_mass, int direction, vec_t a, vec_t b)
{
	setCappedCylinder(1.0, direction, a, b);
	adjust(total_mass);
}


void	dMass::setCylinder(vec_t density, int direction, vec_t radius, vec_t length)
{
	setCylinderTotal(M_PI*radius*radius*length*density, direction, radius, length);
}

void	dMass::setCylinderTotal(vec_t total_mass, int direction, vec_t radius, vec_t length)
{
	setZero();
	
	vec_t r2, I;
	
	r2 = radius*radius;
	_mass = total_mass;
	I = total_mass*(REAL(0.25)*r2 + (REAL(1.0)/REAL(12.0))*length*length);
	
	_I[0][0] = I;
				_I[1][1] = I;
							_I[2][2] = I;
	_I[direction-1][direction-1] = total_mass*REAL(0.5)*r2;
	
#if DEBUG
	check();
#endif
}
*/

void	dMass::setBox(vec_t density, vec_t lx, vec_t ly, vec_t lz)
{
	setBoxTotal(lx*ly*lz*density, lx, ly, lz);
}

void	dMass::setBoxTotal(vec_t total_mass, vec_t lx, vec_t ly, vec_t lz)
{
	setZero();
	
	_mass = total_mass;
	
	_I[0][0] = total_mass/REAL(12.0) * (ly*ly + lz*lz);
	_I[1][1] = total_mass/REAL(12.0) * (lx*lx + lz*lz);
	_I[2][2] = total_mass/REAL(12.0) * (lx*lx + ly*ly);
	
#if DEBUG
	check();
#endif
}


void	dMass::adjust(vec_t newmass)
{
	vec_t scale = newmass / _mass;
	_mass = newmass;
	
	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		{
			_I[i][j] *= scale;
		}
	}

#if DEBUG
 	check();
#endif
}


void	dMass::translate(vec_t x, vec_t y, vec_t z)
{
	// if the body is translated by `a' relative to its point of reference,
	// the new inertia about the point of reference is:
	//
	//   I + mass*(crossmat(c)^2 - crossmat(c+a)^2)
	//
	// where c is the existing center of mass and I is the old inertia.
	
	int i,j;
	matrix_c ahat, chat, t1, t2;
	vec_t a[3];
	
	// adjust inertia matrix
	chat.zero();
	dCROSSMAT((vec_t*)chat, _c, 4, +, -);
	
	a[0] = x + _c[0];
	a[1] = y + _c[1];
	a[2] = z + _c[2];
	
	ahat.zero();
	dCROSSMAT((vec_t*)ahat, a, 4, +, -);
	dMULTIPLY0_333((vec_t*)t1, (vec_t*)ahat, (vec_t*)ahat);
	dMULTIPLY0_333((vec_t*)t2, (vec_t*)chat, (vec_t*)chat);
	
	for(i=0; i<3; i++)
	{
		for(j=0; j<3; j++)
		{
			_I[i][j] += _mass * (t2[i][j]-t1[i][j]);
		}
	}
	
	// ensure perfect symmetry
	_I[1][0] = _I[0][1];
	_I[2][0] = _I[0][2];
	_I[2][1] = _I[1][2];
	
	// adjust center of mass
	_c[0] += x;
	_c[1] += y;
	_c[2] += z;
	
#if DEBUG
	check();
#endif
}


void	dMass::rotate(const matrix_c &R)
{
	// if the body is rotated by `R' relative to its point of reference,
	// the new inertia about the point of reference is:
	//
	//   R * I * R'
	//
	// where I is the old inertia.
	
	matrix_c t1;
	vec_t t2[3];
	
	// rotate inertia matrix
	dMULTIPLY2_333((vec_t*)t1, (vec_t*)_I, (vec_t*)R);
	dMULTIPLY0_333((vec_t*)_I, (vec_t*)R, (vec_t*)t1);
	
	// ensure perfect symmetry
	_I[1][0] = _I[0][1];
	_I[2][0] = _I[0][2];
	_I[2][1] = _I[1][2];
	
	// rotate center of mass
	dMULTIPLY0_331((vec_t*)t2, (vec_t*)R, _c);
	_c[0] = t2[0];
	_c[1] = t2[1];
	_c[2] = t2[2];

#if DEBUG
	check();
#endif
}


void	dMass::add(const dMass *b)
{
	dAASSERT(b);
	
	vec_t denom = X_recip(_mass + b->_mass);
	
	for(int i=0; i<3; i++)
		_c[i] = (_c[i]*_mass + b->_c[i]*b->_mass)*denom;
		
	_mass += b->_mass;
	
	_I = _I + b->_I;
}

// return 1 if ok, 0 if bad
int	dMass::check()
{
	if(_mass <= 0)
	{
		dDEBUGMSG("mass must be > 0");
		return 0;
	}
	
	if(!dIsPositiveDefinite(_I,3))
	{
		dDEBUGMSG("inertia must be positive definite");
		return 0;
	}
	
	// verify that the center of mass position is consistent with the mass
	// and inertia matrix. this is done by checking that the inertia around
	// the center of mass is also positive definite. from the comment in
	// dMassTranslate(), if the body is translated so that its center of mass
	// is at the point of reference, then the new inertia is:
	//   I + mass*crossmat(c)^2
	// note that requiring this to be positive definite is exactly equivalent
	// to requiring that the spatial inertia matrix
	//   [ mass*eye(3,3)   M*crossmat(c)^T ]
	//   [ M*crossmat(c)   I               ]
	// is positive definite, given that I is PD and mass>0. see the theorem
	// about partitioned PD matrices for proof.
		
	matrix_c I2, chat;
	chat.zero();
	dCROSSMAT((vec_t*)chat, _c, 4, +, -);
	
	dMULTIPLY0_333((vec_t*)I2, (vec_t*)chat, (vec_t*)chat);
	
	/*
	for(i=0; i<3; i++)	I2[i] = _I[i] + _mass*I2[i];
	for(i=4; i<7; i++)	I2[i] = _I[i] + _mass*I2[i];
	for(i=8; i<11; i++)	I2[i] = _I[i] + _mass*I2[i];
	*/
	
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<4; j++)
		{
			I2[i][j] = _I[i][j] + I2[i][j] * _mass;
		}
	}
	
	
	if(!dIsPositiveDefinite(I2,3))
	{
		dDEBUGMSG("center of mass inconsistent with mass parameters");
		return 0;
	}
	
	return 1;
}
