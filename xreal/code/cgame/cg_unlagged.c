/*
===========================================================================
Copyright (C) 2006-2007 Neil “haste” Toronto <http://www.ra.is/unlagged/>

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

#include "cg_local.h"

// we'll need these prototypes
void            CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum);
void            CG_Bullet(vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum);

// and this as well
#define MACHINEGUN_SPREAD	200

/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects(centity_t * cent)
{
	vec3_t          muzzlePoint, forward, right, up;
	entityState_t  *ent = &cent->currentState;

	// if the client isn't us, forget it
	if(cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		return;
	}

	// if it's not switched on server-side, forget it
	if(!cgs.delagHitscan)
	{
		return;
	}

	// get the muzzle point
	VectorCopy(cg.predictedPlayerState.origin, muzzlePoint);
	muzzlePoint[2] += cg.predictedPlayerState.viewheight;

	// get forward, right, and up
	AngleVectors(cg.predictedPlayerState.viewangles, forward, right, up);
	VectorMA(muzzlePoint, 14, forward, muzzlePoint);

	// was it a rail attack?
	if(ent->weapon == WP_RAILGUN)
	{
		// do we have it on for the rail gun?
		if(cg_delag.integer & 1 || cg_delag.integer & 16)
		{
			trace_t         trace;
			vec3_t          endPoint;

			// trace forward
			VectorMA(muzzlePoint, 8192, forward, endPoint);

			// THIS IS FOR DEBUGGING!
			// you definitely *will* want something like this to test the backward reconciliation
			// to make sure it's working *exactly* right
			if(0)
			{
				// trace forward
				CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cent->currentState.number,
						 CONTENTS_BODY | CONTENTS_SOLID);

				// did we hit another player?
				if(trace.fraction < 1.0f && (trace.contents & CONTENTS_BODY))
				{
					// if we have two snapshots (we're interpolating)
					if(cg.nextSnap)
					{
						centity_t      *c = &cg_entities[trace.entityNum];
						vec3_t          origin1, origin2;

						// figure the two origins used for interpolation
						BG_EvaluateTrajectory(&c->currentState.pos, cg.snap->serverTime, origin1);
						BG_EvaluateTrajectory(&c->nextState.pos, cg.nextSnap->serverTime, origin2);

						// print some debugging stuff exactly like what the server does

						// it starts with "Int:" to let you know the target was interpolated
						CG_Printf("^3Int: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n",
								  cg.oldTime, cg.snap->serverTime, cg.nextSnap->serverTime,
								  c->lerpOrigin[0], c->lerpOrigin[1], c->lerpOrigin[2]);
						CG_Printf("^5frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n",
								  cg.frameInterpolation, origin1[0], origin1[1], origin1[2], origin2[0], origin2[1], origin2[2]);
					}
					else
					{
						// we haven't got a next snapshot
						// the client clock has either drifted ahead (seems to happen once per server frame
						// when you play locally) or the client is using timenudge
						// in any case, CG_CalcEntityLerpPositions extrapolated rather than interpolated
						centity_t      *c = &cg_entities[trace.entityNum];
						vec3_t          origin1, origin2;

						c->currentState.pos.trTime = TR_LINEAR_STOP;
						c->currentState.pos.trTime = cg.snap->serverTime;
						c->currentState.pos.trDuration = 1000 / sv_fps.integer;

						BG_EvaluateTrajectory(&c->currentState.pos, cg.snap->serverTime, origin1);
						BG_EvaluateTrajectory(&c->currentState.pos, cg.snap->serverTime + 1000 / sv_fps.integer, origin2);

						// print some debugging stuff exactly like what the server does

						// it starts with "Ext:" to let you know the target was extrapolated
						CG_Printf("^3Ext: time: %d, j: %d, k: %d, origin: %0.2f %0.2f %0.2f\n",
								  cg.oldTime, cg.snap->serverTime, cg.snap->serverTime,
								  c->lerpOrigin[0], c->lerpOrigin[1], c->lerpOrigin[2]);
						CG_Printf("^5frac: %0.4f, origin1: %0.2f %0.2f %0.2f, origin2: %0.2f %0.2f %0.2f\n",
								  cg.frameInterpolation, origin1[0], origin1[1], origin1[2], origin2[0], origin2[1], origin2[2]);
					}
				}
			}

			// find the rail's end point
			CG_Trace(&trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, cg.predictedPlayerState.clientNum, CONTENTS_SOLID);

			// do the magic-number adjustment
			VectorMA(muzzlePoint, 4, right, muzzlePoint);
			VectorMA(muzzlePoint, -1, up, muzzlePoint);

			// draw a rail trail
			CG_RailTrail(&cgs.clientinfo[cent->currentState.number], muzzlePoint, trace.endpos);
			//Com_Printf( "Predicted rail trail\n" );

			// explosion at end if not SURF_NOIMPACT
			if(!(trace.surfaceFlags & SURF_NOIMPACT))
			{
				// predict an explosion
				CG_MissileHitWall(ent->weapon, cg.predictedPlayerState.clientNum, trace.endpos, trace.plane.normal,
								  IMPACTSOUND_DEFAULT);
			}
		}
	}
	// was it a shotgun attack?
	else if(ent->weapon == WP_SHOTGUN)
	{
		// do we have it on for the shotgun?
		if(cg_delag.integer & 1 || cg_delag.integer & 4)
		{
			int             contents;
			vec3_t          endPoint, v;
			vec3_t          up;

			// do everything like the server does

			SnapVector(muzzlePoint);

			VectorScale(forward, 4096, endPoint);
			SnapVector(endPoint);

			VectorSubtract(endPoint, muzzlePoint, v);
			VectorNormalize(v);
			VectorScale(v, 32, v);
			VectorAdd(muzzlePoint, v, v);

			contents = trap_CM_PointContents(muzzlePoint, 0);
			if(!(contents & CONTENTS_WATER))
			{
				VectorSet(up, 0, 0, 8);
				CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
			}

			// do the shotgun pellets
			CG_ShotgunPattern(muzzlePoint, endPoint, cg.oldTime % 256, cg.predictedPlayerState.clientNum);
			//Com_Printf( "Predicted shotgun pattern\n" );
		}
	}
	// was it a machinegun attack?
	else if(ent->weapon == WP_MACHINEGUN)
	{
		// do we have it on for the machinegun?
		if(cg_delag.integer & 1 || cg_delag.integer & 2)
		{
			// the server will use this exact time (it'll be serverTime on that end)
			int             seed = cg.oldTime % 256;
			float           r, u;
			trace_t         tr;
			qboolean        flesh;
			int             fleshEntityNum = 0;
			vec3_t          endPoint;

			// do everything exactly like the server does

			r = Q_random(&seed) * M_PI * 2.0f;
			u = sin(r) * Q_crandom(&seed) * MACHINEGUN_SPREAD * 16;
			r = cos(r) * Q_crandom(&seed) * MACHINEGUN_SPREAD * 16;

			VectorMA(muzzlePoint, 8192 * 16, forward, endPoint);
			VectorMA(endPoint, r, right, endPoint);
			VectorMA(endPoint, u, up, endPoint);

			CG_Trace(&tr, muzzlePoint, NULL, NULL, endPoint, cg.predictedPlayerState.clientNum, MASK_SHOT);

			if(tr.surfaceFlags & SURF_NOIMPACT)
			{
				return;
			}

			// snap the endpos to integers, but nudged towards the line
			SnapVectorTowards(tr.endpos, muzzlePoint);

			// do bullet impact
			if(tr.entityNum < MAX_CLIENTS)
			{
				flesh = qtrue;
				fleshEntityNum = tr.entityNum;
			}
			else
			{
				flesh = qfalse;
			}

			// do the bullet impact
			CG_Bullet(tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshEntityNum);
			//Com_Printf( "Predicted bullet\n" );
		}
	}
}
