/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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


/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cg_local.h"



static vec3_t avelocities [NUMVERTEXNORMALS];
static int cl_gravity = 800;	// Tr3B - TODO grab servers gravity
static int cl_rain_counter = 0;


cg_particle_t*	particles_active;
cg_particle_t*	particles_free;
cg_particle_t	particles[MAX_PARTICLES];
int		particles_num = MAX_PARTICLES;


void	CG_ClearParticles()
{
	particles_free = &particles[0];
	particles_active = NULL;
	
	memset(particles, 0, sizeof (particles));

	for(int i=0 ;i<particles_num ; i++)
		particles[i].next = &particles[i+1];
	particles[particles_num-1].next = NULL;
	
	cl_rain_counter = 0;
}

cg_particle_t*	CG_SpawnParticle()
{
	if(!particles_free)
		return NULL;

	cg_particle_t *p = particles_free;
	
	particles_free = p->next;
	p->next = particles_active;
	particles_active = p;
	
	p->time = cgi.CL_GetTime();
	
	return p;
}

void CG_ExplosionParticles (vec3_t org)
{
	int			i, j;
	cg_particle_t	*p;
	
	//cgi.Com_DPrintf ("CG_ExplosionParticles: called\n");

	for (i=0 ; i<256 ; i++)
	{
		if (!particles_free)
			return;
			
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();

		for (j=0 ; j<3 ; j++)
		{
			p->shared.origin[j] = org[j] + ((rand()%32)-16);
			p->vel[j] = (rand()%384)-192;
		}

		p->accel[0] = 0;
		p->accel[1] = 0;
		p->accel[2] = 0;	//-PARTICLE_GRAVITY;
		
		//p->alpha = 1.0;
		p->alphavel = -0.8 / (0.5 + X_frand()*0.3);
		
		p->shared.x = 3;
		p->shared.y = 3;
		
		p->weight = 0;
		
		p->shared.color[0] = 4 + X_frand() * 0.2;
		p->shared.color[1] = 4 + X_frand() * 0.2;
		p->shared.color[2] = 0;
		p->shared.color[3] = 1;
		
		p->shared.shader = cgi.R_RegisterParticle ("particles/dot.tga");
	}
}




void	CG_DiminishingTrail(vec3_t start, vec3_t end, cg_entity_t *old, int flags)
{
#if 0
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cg_particle_t	*p;
	float		dec;
	float		orgscale;
	float		velscale;

	Vector3_Copy (start, move);
	Vector3_Subtract (end, start, vec);
	len = Vector3_Normalize (vec);

	dec = 0.5;
	Vector3_Scale (vec, dec, vec);

	if (old->trailcount > 900)
	{
		orgscale = 4;
		velscale = 15;
	}
	else if (old->trailcount > 800)
	{
		orgscale = 2;
		velscale = 10;
	}
	else
	{
		orgscale = 1;
		velscale = 5;
	}

	while (len > 0)
	{
		len -= dec;

		if (!particles_free)
			return;

		// drop less particles as it flies
		if ((rand()&1023) < old->trailcount)
		{
			p = particles_free;
			particles_free = p->next;
			p->next = particles_active;
			particles_active = p;
			p->accel.clear();
		
			p->time = cgi.CL_GetTime();

			if (flags & EF_GIB)
			{
				//p->alpha = 1.0;
				p->alphavel = -1.0 / (1+X_frand()*0.4);
				
				for (j=0 ; j<3 ; j++)
				{
					p->shared.origin[j] = move[j] + X_crand()*orgscale;
					p->vel[j] = X_crand()*velscale;
					p->accel[j] = 0;
				}
				p->vel[2] -= PARTICLE_GRAVITY;
			}
			else
			{
				//p->alpha = 1.0;
				p->alphavel = -1.0 / (1+X_frand()*0.2);
				
				for (j=0 ; j<3 ; j++)
				{
					p->shared.origin[j] = move[j] + X_crand()*orgscale;
					p->vel[j] = X_crand()*velscale;
				}
				p->accel[2] = 20;
			}
		}

		old->trailcount -= 5;
		if (old->trailcount < 100)
			old->trailcount = 100;
		Vector3_Add (move, vec, move);
	}
#endif
}

void	CG_RocketTrail(vec3_t start, vec3_t end, cg_entity_t *old)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			j;
	cg_particle_t	*p;
	float		dec;

	// smoke
	//CG_DiminishingTrail (start, end, old, EF_ROCKET);

	// fire
	Vector3_Copy (start, move);
	Vector3_Subtract (end, start, vec);
	len = Vector3_Normalize (vec);

	dec = 1;
	Vector3_Scale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		if (!particles_free)
			return;

		if ( (rand()&7) == 0)
		{
			p = particles_free;
			particles_free = p->next;
			p->next = particles_active;
			particles_active = p;
			
			p->accel.clear();
			p->time = cgi.CL_GetTime();

			//p->alpha = 1.0;
			p->alphavel = -1.0 / (1+X_frand()*0.2);
 			
			for (j=0 ; j<3 ; j++)
			{
				p->shared.origin[j] = move[j] + X_crand()*5;
				p->vel[j] = X_crand()*20;
			}
			p->accel[2] = -PARTICLE_GRAVITY;
			
			p->accel[0] = 0;
			p->accel[1] = 0;
			p->accel[2] = 0;	//-PARTICLE_GRAVITY;
		
			//p->alpha = 1.0;
			p->alphavel = -0.8 / (0.5 + X_frand()*0.3);
		
			p->shared.x = 3;
			p->shared.y = 3;
		
			p->weight = 0;
		
			p->shared.color[0] = 5 + X_frand() * 0.2;
			p->shared.color[1] = 5 + X_frand() * 0.2;
			p->shared.color[2] = 5 + X_frand() * 0.2;
			p->shared.color[3] = 0.5;
		
			p->shared.shader = cgi.R_RegisterParticle("particles/smoke1.tga");

		}
		Vector3_Add (move, vec, move);
	}
}

void CG_RailTrail (vec3_t start, vec3_t end)
{
	vec3_c		move;
	vec3_c		vec;
	float		len;
	int			j;
	cg_particle_t	*p;
	float		dec;
	vec3_c		right, up;
	int			i;
	float		d, c, s;
	vec3_c		dir;

	Vector3_Copy (start, move);
	Vector3_Subtract (end, start, vec);
	len = Vector3_Normalize (vec);

	Vector3_MakeNormalVectors (vec, right, up);

	for (i=0 ; i<len ; i++)
	{
		if (!particles_free)
			return;

		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;
		
		p->time = cgi.CL_GetTime();
		p->accel.clear();

		d = i * 0.1;
		c = cos(d);
		s = sin(d);

		Vector3_Scale (right, c, dir);
		Vector3_MA (dir, s, up, dir);

		//p->alpha = 1.0;
		p->alphavel = -1.0 / (1+X_frand()*0.2);
		
		for (j=0 ; j<3 ; j++)
		{
			p->shared.origin[j] = move[j] + dir[j]*3;
			p->vel[j] = dir[j]*6;
		}

		Vector3_Add (move, vec, move);
	}

	dec = 0.75;
	Vector3_Scale (vec, dec, vec);
	Vector3_Copy (start, move);

	while (len > 0)
	{
		len -= dec;

		if (!particles_free)
			return;
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();
		p->accel.clear();

		//p->alpha = 1.0;
		p->alphavel = -1.0 / (0.6+X_frand()*0.2);

		for (j=0 ; j<3 ; j++)
		{
			p->shared.origin[j] = move[j] + X_crand()*3;
			p->vel[j] = X_crand()*3;
			p->accel[j] = 0;
		}

		Vector3_Add (move, vec, move);
	}
}


void	CG_BubbleTrail(vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i, j;
	cg_particle_t	*p;
	float		dec;

	Vector3_Copy (start, move);
	Vector3_Subtract (end, start, vec);
	len = Vector3_Normalize (vec);

	dec = 32;
	Vector3_Scale (vec, dec, vec);

	for (i=0 ; i<len ; i+=(int)dec)
	{
		if (!particles_free)
			return;

		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->accel.clear();
		p->time = cgi.CL_GetTime();

		//p->alpha = 1.0;
		p->alphavel = -1.0 / (1+X_frand()*0.1);
		
		for (j=0 ; j<3 ; j++)
		{
			p->shared.origin[j] = move[j] + X_crand()*2;
			p->vel[j] = X_crand()*10;
		}
		p->vel[2] += 6;
				
		p->shared.x = fabs(X_frand()*3);
		p->shared.y = fabs(X_frand()*3);
		
		p->weight = 0;
		
		p->shared.color[0] = 1;
		p->shared.color[1] = 1;
		p->shared.color[2] = 1;
		p->shared.color[3] = 1;
		
		p->shared.shader = cgi.R_RegisterParticle ("particles/bubble.tga");

		Vector3_Add (move, vec, move);
	}
}


#define	BEAMLENGTH			16
/*
static void CG_FlyParticles (vec3_t origin, int count)
{
	int			i;
	cg_particle_t	*p;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	float		ltime;


	if (count > NUMVERTEXNORMALS)
		count = NUMVERTEXNORMALS;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cgi.CL_GetTime() / 1000.0;
	for (i=0 ; i<count ; i+=2)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if (!particles_free)
			return;
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();

		dist = sin(ltime + i)*64;
		p->shared.origin[0] = origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH;
		p->shared.origin[1] = origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH;
		p->shared.origin[2] = origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH;

		p->vel.clear();
		p->accel.clear();

		p->colorvel = 0;

		//p->alpha = 1;
		p->alphavel = -100;
	}
}
*/


#define	BEAMLENGTH			16
void CG_BfgParticles (r_entity_t *ent)
{
	int			i;
	cg_particle_t	*p;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	vec3_t		v;
	float		ltime;
	
	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cgi.CL_GetTime() / 1000.0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		if (!particles_free)
			return;
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();

		dist = sin(ltime + i)*64;
		p->shared.origin[0] = ent->origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH;
		p->shared.origin[1] = ent->origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH;
		p->shared.origin[2] = ent->origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH;

		p->vel.clear();
		p->accel.clear();

		Vector3_Subtract (p->shared.origin, ent->origin, v);
		dist = Vector3_Length(v) / 90.0;

		p->colorvel = 0;

		//p->alpha = 1.0 - dist;
		p->alphavel = -100;
	}
}


void CG_BFGExplosionParticles (vec3_t org)
{
	int			i, j;
	cg_particle_t	*p;

	for (i=0 ; i<256 ; i++)
	{
		if (!particles_free)
			return;
			
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();

		for (j=0 ; j<3 ; j++)
		{
			p->shared.origin[j] = org[j] + ((rand()%32)-16);
			p->vel[j] = (rand()%384)-192;
		}

		p->accel[0] = p->accel[1] = 0;
		p->accel[2] = -PARTICLE_GRAVITY;
		//p->alpha = 1.0;

		p->alphavel = -0.8 / (0.5 + X_frand()*0.3);
	}
}


void	CG_AddParticles()
{
	cg_particle_t		*p, *next;
	float			time, time2;
	vec3_c			org;
	cg_particle_t		*active, *tail;

	active = NULL;
	tail = NULL;
	time = 0;

	for(p=particles_active; p; p=next)
	{
		next = p->next;
		
		if(p->alphavel != INSTANT_PARTICLE)
		{
			time = (cgi.CL_GetTime() - p->time) * 0.001;
			
			if((p->shared.color[3] + time * p->alphavel) <= 0)
			{
				// faded out
				p->next = particles_free;
				particles_free = p;
				continue;
			}
		}
		
		p->next = NULL;
		
		if(!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}

		X_clamp(p->shared.color[3], 0, 1);

		time2 = time*time;

		org[0] = (p->shared.origin[0] + p->vel[0]*time + (0.5 * p->accel[0]*time2));
		org[1] = (p->shared.origin[1] + p->vel[1]*time + (0.5 * p->accel[1]*time2));
		org[2] = (p->shared.origin[2] + p->vel[2]*time + (0.5 *(p->accel[2] - p->weight * cl_gravity)*time2));
		
		p->shared.origin = org;

		cgi.R_AddParticle(p->shared);
		
		if(p->alphavel == INSTANT_PARTICLE)
		{
			p->alphavel = 0.0;
			p->shared.color[3] = 0.0;
		}
	}

	particles_active = active;
}





static void	CG_SetParticleVelocity(cg_particle_type_e type, cg_particle_t *p, vec3_c &dir)
{
	int		j;
	
	switch(type)
	{
		//case PART_BUBBLE:
		//case PART_SMOKE:
		case PART_SPLASH:
		case PART_SPLASH1:
			p->vel[0] = SPLASH_VELX_BASE + dir[0] + X_crand()*(fabs(SPLASH_VELX_BASE*2)); // -20 to 20
			p->vel[1] = SPLASH_VELY_BASE + dir[1] + X_crand()*(fabs(SPLASH_VELY_BASE*2)); // -20 to 20
			p->vel[2] = SPLASH_VELZ_BASE + dir[0] + X_crand()*20;
			break;
		
		case PART_SPARK:
		case PART_SPARK1:
			for(j=0; j<3; j++)
				p->vel[j] = X_crand()*20;
			break;
			
		case PART_SPARK2:
			for(j=0; j<3; j++)
				p->vel[j] = dir[j]*30 + X_crand()*100;
			break;
			
		case PART_RAIN:
			Vector3_Set(p->vel, 0, 0, -40);
			break;
			
		//case PART_DUST:
		//case PART_STEAM:
		case PART_BLOOD:
		case PART_BLOOD1:
		case PART_BLOOD2:
		case PART_BLOOD3:
		case PART_BLOOD4:
			for(j=0; j<3; j++)
				p->vel[j] = dir[j]*30 + X_crand()*100;
			break;
			
		case PART_BLASTER:
		case PART_RAIL:
			p->vel.set(0, 0, 0);
			break;
		//case PART_LBLAST:
		//case PART_ROCKET:
		case PART_SNOW1:
		case PART_SNOW2:
		case PART_SNOW3:
		case PART_SNOW4:
		case PART_SNOW5:
		case PART_SNOW6:
		case PART_SNOW7:
		case PART_SNOW8:
		case PART_SNOW9:
		case PART_SNOW10:
		case PART_SNOW11:
		case PART_SNOW12:
			p->vel.set(0, 0, -30);
			break;
		
		case PART_HYPER:
			p->vel.set(0, 0, 0);
			break;
			
		case PART_EXPLODE:
			for(j=0; j<3; j++)
				p->vel[j] = ((rand()%384) - 192);
			break;
			
		case PART_RESPAWN:
			for(j=0; j<3; j++)
				p->vel[j] = X_crand()*8;
			break;
			
		case PART_EXPSMOKE:
			for(j=0; j<3; j++)
				p->vel[j] = ((rand()%100) - 50);
			break;
	
		//case PART_BOSSTELE:
		//case PART_DOT:
		default:
			break;
	}
}


static void	CG_SetParticleAcceleration(cg_particle_type_e type, cg_particle_t *p, vec3_c &dir)
{	
	switch(type)
	{
		//case PART_BUBBLE:
		//case PART_SMOKE:
		case PART_SPLASH:
		case PART_SPLASH1:
			p->accel[0] = SPLASH_ACCELX_BASE + X_crand()*20; // 80 to 100
			p->accel[1] = SPLASH_ACCELY_BASE + X_crand()*20; // 80 to 100
			p->accel[2] = SPLASH_ACCELZ_BASE;
			break;
		
		case PART_SPARK:
		case PART_SPARK1:
			p->accel[0] = X_frand()*5;
			p->accel[1] = X_frand()*5;
			p->accel[2] = -5 + X_frand()*10;
			break;
			
		case PART_SPARK2:
			p->accel[0] = 50 + X_frand()*20;
			p->accel[1] = 50 + X_frand()*20;
			p->accel[2] = 50 + X_frand()*20;
			break;
			
		case PART_RAIN:
			p->accel.set(0, 0, -40);
			break;
			
		//case PART_DUST:
		//case PART_STEAM:
		case PART_BLOOD:
		case PART_BLOOD1:
		case PART_BLOOD2:
		case PART_BLOOD3:
		case PART_BLOOD4:
			p->accel[0] = 50 + X_frand()*20;
			p->accel[1] = 50 + X_frand()*20;
			p->accel[2] = 50 + X_frand()*20;
			break;
			
		case PART_BLASTER:
		case PART_RAIL:
			p->accel.set(0, 0, 0);
			break;
		//case PART_LBLAST:
		//case PART_ROCKET:
		case PART_SNOW1:
		case PART_SNOW2:
		case PART_SNOW3:
		case PART_SNOW4:
		case PART_SNOW5:
		case PART_SNOW6:
		case PART_SNOW7:
		case PART_SNOW8:
		case PART_SNOW9:
		case PART_SNOW10:
		case PART_SNOW11:
		case PART_SNOW12:
			p->accel.set(0, 0, -10);
			break;
		
		case PART_HYPER:
			p->accel.set(0, 0, 0);
			break;
			
		case PART_EXPLODE:
			p->accel[0] = X_frand()*5;
			p->accel[1] = X_frand()*5;
			p->accel[2] = -5 + X_frand()*10;
			break;
			
		case PART_RESPAWN:
			p->accel.set(0, 0, 0);
			break;
			
		case PART_EXPSMOKE:
			p->accel[0] = p->vel[0]/10;	//X_frand()*5;
			p->accel[1] = p->vel[1]/10;	//X_frand()*5;
			p->accel[2] = 1 + X_frand()*5;
			break;
	
		//case PART_BOSSTELE:
		//case PART_DOT:
		default:
			break;
	}
}


void	CG_ParticleSpray(cg_particle_type_e type, const vec3_c &org, vec3_c &dir, const vec4_c &color, int count)
{
	int			j, k;
	cg_particle_t	*p;
	float	dir_var;

	
	//cgi.Com_DPrintf("CG_ParticleSpray: called\n");
	
	if(count <= 0)
		count = (int)(30+X_frand()*30);

	for(int i=0; i<count; i++)
	{
		p = CG_SpawnParticle();
		
		if(!p)
			return;

		p->shared.origin = org;
		
		CG_SetParticleVelocity(type, p, dir);
		CG_SetParticleAcceleration(type, p, dir);

		p->type = (cg_particle_type_e)type;
		
		//p->shared.color.set(0.5, 0.5, 0.5, 0.7);
		
		switch(type)
		{
			//case PART_BUBBLE:
			//case PART_SMOKE:
			case PART_SPLASH:
				p->shared.origin[2] += 8;
				
				p->alphavel = -1.0 / (0.5 + X_frand()*0.3);
				
				p->shared.x = 0.5 + fabs (X_frand());
				p->shared.y = 0.5 + fabs (X_frand());
				
				p->weight = (p->shared.x * 0.1) + SPLASH_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/wtrdrop.tga");
				
				if(color.isZero())
					p->shared.color.set(0.85, 0.85, 1.0, 0.8);
				
				break;
				
			case PART_SPLASH1:
				p->shared.origin[2] += 8;
				
				p->alphavel = -1.0 / (0.5 + X_frand()*0.3);
				
				p->shared.x = 1 + fabs (X_frand());
				p->shared.y = 1 + fabs (X_frand());
				
				p->weight = (p->shared.x * 0.1) + SPLASH_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/wtrdrop1.tga");
				
				if (color.isZero())
					p->shared.color.set(0.85, 0.85, 1.0, 0.8); 
				break;
				
			case PART_SPARK:
				dir_var = rand()&31;
				for (j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()&7)-4) + dir_var*dir[j];
				}
				
				p->alphavel = -1.0 / (0.2 + X_frand()*0.3);
				
				p->shared.x = fabs (X_frand()*10);
				p->shared.y = fabs (X_frand()*10);
				
				p->weight = (p->shared.x * 0.1) + SPARK_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/sparks.tga");
				
				if (color.isZero())
					p->shared.color.set(0.5+X_frand()*0.2, 0.5+X_frand()*0.2, 0, 0.7);
				break;
				
			case PART_SPARK1:
				dir_var = rand()&31;
				for (j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()&7)-4) + dir_var*dir[j];
				}
				
				p->alphavel = -1.0 / (0.2 + X_frand()*0.3);
				
				p->shared.x = 2 * fabs (X_frand()*10);
				p->shared.y = 2 * fabs (X_frand()*10);
				
				p->weight = (p->shared.x * 0.1) + SPARK_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/sparks1.tga");
				
				if (color.isZero())
					p->shared.color.set(0.5+X_frand()*0.2, 0.5+X_frand()*0.2, 0, 0.7);
				break;
				
			case PART_SPARK2:
				dir_var = rand()&31;
				
				for(j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()&7)-4) + dir_var * dir[j];
				}
				
				p->alphavel = -1.0 / (0.2 + X_frand()*0.3);
				
				p->shared.x = fabs(X_frand()*10);
				p->shared.y = fabs(X_frand()*10);
				
				p->weight = (p->shared.x * 0.1) + SPARK_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/sparks2.tga");
				
				if(color.isZero())
					p->shared.color.set(0.3+X_frand()*0.8, 0.3+X_frand()*0.8, 0.3, 1.0);
				break;
				
			case PART_RAIN:
				cl_rain_counter++;
				p->shared.origin[0] = org[0] + (-150 + X_frand()*300);
				p->shared.origin[1] = org[1] + (-150 + X_frand()*300);
				p->shared.origin[2] = org[2] + 20 + X_frand()*20;
				
				p->alphavel = -1.3;
				
				p->shared.x = 0.4;
				p->shared.y = 0.4;
				
				p->weight = (p->shared.x * 0.1) + RAIN_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle ("particles/rain1.tga");
				
				if (color.isZero())
					p->shared.color.set(0.7, 0.7, 0.9, 0.7);
				break;

			//case PART_DUST:
			//case PART_STEAM:
			//case PART_BLOOD:
			//case PART_BLOOD1:
			//case PART_BLOOD2:
			//case PART_BLOOD3:
			case PART_BLOOD4:
				dir_var = rand()&15;
				for (j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()&7)-4) + dir_var*dir[j];
				}
				
				p->alphavel = -1.0 / (0.2 + X_frand()*0.3);
				
				p->shared.x = 2 * fabs (X_frand()*5);
				p->shared.y = 2 * fabs (X_frand()*5);
				
				p->shared.origin[2] += p->shared.x;
				
				p->weight = (p->shared.x * 0.1) + BLOOD_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle ("particles/blood.tga");
				
				if (color.isZero())
					p->shared.color.set(0.5, 0.1, 0.1, 1);
				break;
				
			//TODO	

			case PART_BLASTER:
				dir_var = rand()&31;
				
				for(j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()&7)-4) + dir_var * dir[j];
				}
				
				p->alphavel = -1.0 / (0.2 + X_frand()*0.3);
				
				p->shared.x = fabs(X_frand()*10);
				p->shared.y = fabs(X_frand()*10);
				
				p->weight = (p->shared.x * 0.1) + SPARK_WEIGHT * X_frand();
				
				p->shared.shader = cgi.R_RegisterParticle("particles/sparks2.tga");
				
				if(color.isZero())
					p->shared.color.set(0.3+X_frand()*0.8, 0.3+X_frand()*0.8, 0.3, 1.0);
				break;
			
			
			//case PART_RAIL:
			//case PART_LBLAST:
			//case PART_ROCKET:
			case PART_SNOW1:
				cl_rain_counter++;
				p->shared.origin[0] = org[0] + (-150 + X_frand()*300);
				p->shared.origin[1] = org[1] + (-150 + X_frand()*300);
				p->shared.origin[2] = org[2] + 20 + X_frand()*20;
				
				p->alphavel = -1.3;
				
				p->shared.x = 0.4;
				p->shared.y = 0.4;
				
				p->weight = (p->shared.x * 0.1) + SNOW_WEIGHT * X_frand();
				
				k = 1 + (int)(X_frand()*12);
				
				switch (k)
				{
					case 1:
						p->type = PART_SNOW1;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow1.tga");
						break;
						
					case 2:
						p->type = PART_SNOW2;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow2.tga");
						break;
						
					case 3:
						p->type = PART_SNOW3;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow3.tga");
						break;
						
					case 4:
						p->type = PART_SNOW4;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow4.tga");
						break;
						
					case 5:
						p->type = PART_SNOW5;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow5.tga");
						break;
						
					case 6:
						p->type = PART_SNOW6;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow6.tga");
						break;
						
					case 7:
						p->type = PART_SNOW7;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow7.tga");
						break;
						
					case 8:
						p->type = PART_SNOW8;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow8.tga");
						break;
						
					case 9:
						p->type = PART_SNOW9;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow9.tga");
						break;
						
					case 10:
						p->type = PART_SNOW10;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow10.tga");
						break;
						
					case 11:
						p->type = PART_SNOW11;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow11.tga");
						break;
						
					case 12:
						p->type = PART_SNOW12;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow12.tga");
						break;
					
					default:
						p->type = PART_SNOW12;
						p->shared.shader = cgi.R_RegisterParticle ("particles/snow12.tga");
						break;
				}
				
				
				
				if (color.isZero())
					p->shared.color.set(1.0, 1.0, 1.0, 1.0);
				break;
				
			
			//case PART_HYPER:
			case PART_EXPLODE:
				for(j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + ((rand()%32) - 16);
				}
				
				p->alphavel = -0.8 / (0.5 + X_frand()*0.3);
				
				p->type = PART_DOT;
				
				p->shared.x = 3;
				p->shared.y = 3;
				
				p->weight = SPARK_WEIGHT;
				
				p->shared.shader = cgi.R_RegisterParticle ("particles/dot.tga");
				
				if(color.isZero())
				{
					p->shared.color[0] = 0.6+X_frand()*0.3;
					p->shared.color[1] = 0.6+X_frand()*0.3;
					p->shared.color[2] += 0.1;
					p->shared.color[3] = 1;
				}
				break;
				
			case PART_RESPAWN:
				for(j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + X_crand()*8;
				}
				
				p->alphavel = -0.66;
				
				p->type = PART_DOT;
				
				p->shared.x = 3;
				p->shared.y = 3;
				
				p->weight = 0;
				
				p->shared.shader = cgi.R_RegisterParticle ("particles/dot.tga");
				
				if(color.isZero())
					p->shared.color.set(1.0, 1.0, 1.0, 0.6);
				break;

				
			case PART_EXPSMOKE:
				//cgi.Com_DPrintf("CG_ParticleSpray: TODO PART_EXPSMOKE\n");
			
				for(j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + (float)((rand() % 100) -50.0);
				}
				p->shared.origin[2] += 25;
				
				p->alphavel = -0.5 + X_frand()*1;
				
				//p->type = PART_DOT;
				
				p->shared.x = 20 + X_frand()*20;
				p->shared.y = 20 + X_frand()*20;
				
				p->weight = -0.01;
				
				p->shared.shader = cgi.R_RegisterParticle("particles/dot.tga");
				
				if(color.isZero())
					p->shared.color.set(0.07+X_frand()*0.04, 0.07+X_frand()*0.04, 0.07+X_frand()*0.04, 1);
				break;

	
			case PART_BOSSTELE:
				//cgi.Com_DPrintf ("CG_ParticleSpray: TODO PART_BOSSTELE\n");
			
				for (j=0; j<3; j++)
				{
					p->shared.origin[j] = org[j] + X_crand()*8;
				}
				
				p->alphavel = -0.66;
				
				p->type = PART_DOT;
				
				p->shared.x = 3;
				p->shared.y = 3;
				
				p->weight = 0;
				
				p->shared.shader = cgi.R_RegisterParticle ("particles/dot.tga");
				
				if (color.isZero())
					p->shared.color.set(0.5, 0.5, 0.5, 0.7);
				break;


			//case PART_DOT:
			
			default:
				break;
		}
		
		
		
		
	}

	//TODO
}


void	CG_ParticleTrail(cg_particle_type_e type, const vec3_c &start, const vec3_c &end, const vec4_c &color, float interval)
{
	vec3_c		move;
	vec3_c		vec;
	float		len;
	cg_particle_t	*p;
	vec3_c		dir;

	move = start;
	vec = end - start;
	len = vec.normalize();

	vec.scale(5);
	
	//cgi.Com_DPrintf("CG_ParticleTrail: called\n");
	
	while (len > 0)
	{
		len -= interval;

		if (!particles_free)
			return;
			
		p = particles_free;
		particles_free = p->next;
		p->next = particles_active;
		particles_active = p;

		p->time = cgi.CL_GetTime();
		
		p->shared.origin = move;

		move += vec;
		
		CG_SetParticleVelocity(type, p, dir);
		CG_SetParticleAcceleration(type, p, dir);
		
		// default settings
		p->type = (cg_particle_type_e)type;
		
		p->weight = 0;
		
		p->shared.color = color;
		
		//p->extra.anim_frames_num = 0;
		//p->extra.anim_cur = 0;
		//p->extra.anim_time = 0;
		//p->extra.anim_last = p->time;
		
		
		switch(type)
		{
			case PART_RAIL:
				p->shared.origin[2] += 4;	
			
				p->alphavel = -1.0 / (0.8);
				
				p->shared.x = 4;
				p->shared.y = 4;
				
				p->weight = 0;
				
				p->shared.shader = cgi.R_RegisterParticle("particles/rail.tga");
				
				if (color.isZero())
					p->shared.color.set(0.0, 0.0, 1, 1);
				break;
					
			
			case PART_BLASTER:
			case PART_HYPER:
				p->shared.origin[2] += 2;
				
				p->alphavel = (-1.0 / (X_frand() * 0.1));
			
				p->shared.x = 2;
				p->shared.y = 2;
				
				p->weight = 0;
				
				p->shared.shader = cgi.R_RegisterParticle("particles/blaster1.tga");
				
				if(color.isZero())
					p->shared.color.set(1.0, 0.9, 0, 1);
				break;
				
			default:
				break;
		};		
	}
}

