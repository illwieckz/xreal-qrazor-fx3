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

#ifndef ODE_MASS_H
#define ODE_MASS_H

#include "common.h"

class dMass
{
public:
	void	setZero();
	
	void	setParameters(vec_t themass,
			 	vec_t cgx, vec_t cgy, vec_t cgz,
			 	vec_t I11, vec_t I22, vec_t I33,
			 	vec_t I12, vec_t I13, vec_t I23);
	
	void	setSphere(vec_t density, vec_t radius);
	void	setSphereTotal(vec_t total_mass, vec_t radius);
	
	void	setCappedCylinder(vec_t density, int direction, vec_t radius, vec_t length);
	void	setCappedCylinderTotal(vec_t total_mass, int direction,vec_t radius, vec_t length);
	
	void	setCylinder(vec_t density, int direction, vec_t radius, vec_t length);
	void	setCylinderTotal(vec_t total_mass, int direction, vec_t radius, vec_t length);
	
	void	setBox(vec_t density, vec_t lx, vec_t ly, vec_t lz);
	void	setBoxTotal(vec_t total_mass, vec_t lx, vec_t ly, vec_t lz);
	
	void	adjust(vec_t newmass);
	
	void	translate(vec_t x, vec_t y, vec_t z);
	
	void	rotate(const matrix_c &R);
	
	void	add(const dMass *b);
	
private:
	// return 1 if ok, 0 if bad
	int	check();
public:
	vec_t		_mass;
	dVector4	_c;
	matrix_c	_I;
};

#endif
