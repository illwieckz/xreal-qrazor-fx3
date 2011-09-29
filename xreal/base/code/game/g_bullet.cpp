/*
===========================================================================
Copyright (C) 2011 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


extern "C" 
{
#include "g_local.h"
}

#include "LinearMath/btVector3.h"
#include "LinearMath/btGeometryUtil.h"
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include "btBulletDynamicsCommon.h"

class BulletPhysics
{
public:

	btAlignedObjectArray<btCollisionShape*>	_collisionShapes;

	btDynamicsWorld*				_dynamicsWorld;
	btCollisionConfiguration*		_collisionConfiguration;
	btCollisionDispatcher*			_dispatcher;
	btBroadphaseInterface*			_broadphase;
	btConstraintSolver*				_solver;

	BulletPhysics()
	{
		_collisionConfiguration = new btDefaultCollisionConfiguration();

		// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		_dispatcher = new btCollisionDispatcher(_collisionConfiguration);
		_dispatcher->registerCollisionCreateFunc(BOX_SHAPE_PROXYTYPE, BOX_SHAPE_PROXYTYPE, _collisionConfiguration->getCollisionAlgorithmCreateFunc(CONVEX_SHAPE_PROXYTYPE, CONVEX_SHAPE_PROXYTYPE));

		_broadphase = new btDbvtBroadphase();

		// the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
		_solver = sol;

		_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _collisionConfiguration);
		//_dynamicsWorld ->setDebugDrawer(&sDebugDrawer);
		_dynamicsWorld->getSolverInfo().m_splitImpulse=true;
		_dynamicsWorld->getSolverInfo().m_numIterations = 20;

		trap_Bullet_AddWorldBrushesToDynamicsWorld(&_collisionShapes, (plDynamicsWorldHandle*) _dynamicsWorld);
	}

	~BulletPhysics()
	{
		// delete collision shapes
		for (int i = 0; i < _collisionShapes.size(); i++)
		{
			btCollisionShape* shape = _collisionShapes[i];
			delete shape;
		}

		delete _dynamicsWorld;
		delete _solver;
		delete _broadphase;
		delete _dispatcher;
		delete _collisionConfiguration;
	}
};

static BulletPhysics* g_bulletPhysics = NULL;


extern "C"
{

void G_InitBulletPhysics()
{
	G_Printf("------- Bullet Physics Initialization -------\n");

	g_bulletPhysics = new BulletPhysics();
}

void G_ShutdownBulletPhysics()
{
	G_Printf("------- Bullet Physics Shutdown -------\n");

	if(g_bulletPhysics)
	{
		delete g_bulletPhysics;
		g_bulletPhysics = NULL;
	}
}

} // extern "C"