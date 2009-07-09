package xreal.server.game;

import javax.vecmath.Vector3f;

import xreal.common.EntityType;

public class Rocket extends GameEntity {
	
	Rocket(Vector3f start, Vector3f dir)
	{
		setEntityState_origin(start);
		setEntityState_eType(EntityType.GENERAL);
		
		//Vector3f        mins = { -8, -8, -8 };
		//Vector3f		maxs = { 8, 8, 8 };

		//VectorNormalize(dir);

		//bolt = G_Spawn();
		//bolt->classname = "rocket";
		//bolt->nextthink = level.time + 15000;
		//bolt->think = G_ExplodeMissile;
		/*
		bolt->s.eType = ET_PROJECTILE;
		bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		bolt->s.weapon = WP_ROCKET_LAUNCHER;
		bolt->r.ownerNum = self->s.number;
		bolt->parent = self;
		bolt->damage = 100;
		bolt->splashDamage = 100;
		bolt->splashRadius = 120;
		bolt->methodOfDeath = MOD_ROCKET;
		bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
		bolt->clipmask = MASK_SHOT;
		bolt->target_ent = NULL;

		// make the rocket shootable
		bolt->r.contents = CONTENTS_SHOOTABLE;
		VectorCopy(mins, bolt->r.mins);
		VectorCopy(maxs, bolt->r.maxs);
		bolt->takedamage = qtrue;
		bolt->health = 50;
		bolt->die = G_Missile_Die;
		*/
		
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
			//bolt->s.pos.trType = TR_LINEAR;
			//VectorScale(dir, g_rocketVelocity.value, bolt->s.pos.trDelta);
		}

		//bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;	// move a bit on the very first frame
		//VectorCopy(start, bolt->s.pos.trBase);

		//SnapVector(bolt->s.pos.trDelta);	// save net bandwidth
		//VectorCopy(start, bolt->r.currentOrigin);
	}
}
