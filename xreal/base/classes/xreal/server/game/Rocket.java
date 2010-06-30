package xreal.server.game;

import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

import xreal.Engine;
import xreal.Trajectory;
import xreal.TrajectoryType;
import xreal.common.EntityType;
import xreal.common.WeaponType;

public class Rocket extends GameEntity {
	
	Rocket(Vector3f start, final Vector3f dir)
	{
		Engine.println("Rocket(start = " + start + ", dir = " + dir);
		
		setEntityState_origin(start);
		setEntityState_eType(EntityType.GENERAL);
		setEntityState_modelindex("models/meshes/ppodium.md5mesh");
		
		//setEntityState_weapon(WeaponType.ROCKET_LAUNCHER);
		
		Trajectory pos = new Trajectory();
		
		/*
		if(g_rocketAcceleration.integer)
		{
			// use acceleration instead of linear velocity
			bolt->s.pos.trType = TR_ACCELERATION;
			bolt->s.pos.trAcceleration = g_rocketAcceleration.value;
			VectorScale(dir, g_rocketVelocity.value, bolt->s.pos.trDelta);
		}
		else
		*/
		{
			pos.trType = TrajectoryType.LINEAR;
			pos.trDelta.scale(3, new Vector4f(dir));
			//VectorScale(dir, g_rocketVelocity.value, bolt->s.pos.trDelta);
		}

		pos.trTime = Game.getLevelTime();// - MISSILE_PRESTEP_TIME;	// move a bit on the very first frame
		pos.trBase = new Vector4f(start);

		//SnapVector(bolt->s.pos.trDelta);	// save net bandwidth ?
		
		setEntityState_pos(pos);
		
		link();
		
		
		
		
		//VectorCopy(start, bolt->r.currentOrigin);
		//Vector3f        mins = { -8, -8, -8 };
		//Vector3f		maxs = { 8, 8, 8 };

		//VectorNormalize(dir);

		//bolt = G_Spawn();
		//bolt->classname = "rocket";
		//bolt->nextthink = level.time + 15000;
		//bolt->think = G_ExplodeMissile;
		
		//bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		
		
		//bolt->r.ownerNum = self->s.number;
		//bolt->parent = self;
		//bolt->damage = 100;
		//bolt->splashDamage = 100;
		//bolt->splashRadius = 120;
		//bolt->methodOfDeath = MOD_ROCKET;
		//bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
		//bolt->clipmask = MASK_SHOT;
		//bolt->target_ent = NULL;

		// make the rocket shootable
		//bolt->r.contents = CONTENTS_SHOOTABLE;
		//VectorCopy(mins, bolt->r.mins);
		//VectorCopy(maxs, bolt->r.maxs);
		//bolt->takedamage = qtrue;
		//bolt->health = 50;
		//bolt->die = G_Missile_Die;
	}
}
