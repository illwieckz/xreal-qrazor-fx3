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
// shared -------------------------------------------------------------------
#include "../x_panim.h"

// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_player.h"
#include "g_item.h"
#include "g_projectile.h"


//static bool	is_quad;
//static byte	is_silenced;




g_item_weapon_c::g_item_weapon_c()
{
	_has_pickup		= true;
	_has_use		= true;
	_has_drop		= true;
}

g_item_weapon_c::~g_item_weapon_c()
{
	//TODO
}

bool	g_item_weapon_c::pickup(g_entity_c *ent, g_player_c *other)
{
	int			index;
	g_item_c		*ammo;

	index = G_GetNumForItem(ent->_item);

	if(((dmflags->getInteger() & DF_WEAPONS_STAY) || coop->getInteger()) && other->_pers.inventory[index])
	{
		if (!(ent->_spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->_pers.inventory[index]++;

	if (!(ent->_spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = G_FindItem (ent->_item->getAmmo());
		
		if(dmflags->getInteger() & DF_INFINITE_AMMO)
			other->addAmmo(ammo, 1000);
		else
			other->addAmmo(ammo, ammo->getQuantity());

		if(!(ent->_spawnflags & DROPPED_PLAYER_ITEM))
		{
			if(deathmatch->getInteger())
			{
				if(dmflags->getInteger() & DF_WEAPONS_STAY)
					ent->_flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if(coop->getInteger())
				ent->_flags |= FL_RESPAWN;
		}
	}

	if(	other->_pers.weapon != ent->_item && (other->_pers.inventory[index] == 1) && (!deathmatch->getInteger() || other->_pers.weapon == G_FindItem("blaster")) )
	{
		other->_newweapon = (g_item_weapon_c*)ent->_item;
	}

	return true;
}






/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void	g_item_weapon_c::use(g_player_c *player)
{
	int			ammo_index;
	g_item_c		*ammo_item;

	// see if we're already using it
	if(this == player->_pers.weapon)
		return;

	if(getAmmo() && !g_select_empty->getInteger() && !(getFlags() & IT_AMMO))
	{
		ammo_item = G_FindItem(getAmmo());
		ammo_index = G_GetNumForItem(ammo_item);

		if(!player->_pers.inventory[ammo_index])
		{
			trap_SV_CPrintf(player, PRINT_HIGH, "No %s for %s.\n", ammo_item->getPickupName(), getPickupName());
			return;
		}

		if(player->_pers.inventory[ammo_index] < getQuantity())
		{
			trap_SV_CPrintf(player, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->getPickupName(), getPickupName());
			return;
		}
	}

	// change to this weapon when down
	player->_newweapon = this;
}



void	g_item_weapon_c::drop(g_player_c *player)
{
	int		index;

	if(dmflags->getInteger() & DF_WEAPONS_STAY)
		return;

	index = G_GetNumForItem(this);
	
	// see if we're already using it
	if(((this == player->_pers.weapon) || (this == player->_newweapon))&& (player->_pers.inventory[index] == 1))
	{
		trap_SV_CPrintf(player, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	player->dropItem(this);
	
	player->_pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
void	g_item_weapon_c::weaponThink(g_player_c *player)
{
	if(player->_deadflag || player->_s.index_model != 255) // VWep animations screw up corpses
		return;
		
	switch(player->_weapon_state)
	{
		case WEAPON_ACTIVATING:
		{
			if(player->_r.ps.gun_anim_frame >= getActivateAnimationLastFrame())
			{
				player->_weapon_state = WEAPON_READY;
				player->_r.ps.gun_anim_frame = getIdleAnimationFirstFrame();
				player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getIdleAnimationName());
				break;
			}
			
			player->incWeaponFrame();
			break;
		}
	
		case WEAPON_DEACTIVATING:
		{
			if(player->_r.ps.gun_anim_frame >= getDeActivateAnimationLastFrame())
			{
				player->changeWeapon();
				break;
			}
			/*
			else if((getDeActivateAnimationLastFrame() - player->_r.ps.gun_anim_frame) == (PLAYER_ANIM_UPPER_FLIPOUT_TIME -1))
			{
				
				player->_anim_priority = ANIM_REVERSE;
				if(player->_r.ps.pmove.pm_flags & PMF_DUCKED)
				{
					player->_s.frame = FRAME_crpain4+1;
				player->_anim_end = FRAME_crpain1;
				}
				else
				{
					player->_s.frame = FRAME_pain304+1;
					player->_anim_end = FRAME_pain301;
					
				}
				
				//FIXME
				player->_anim_priority = ANIM_PAIN;
				player->_anim_current = PLAYER_ANIM_IDLE;
				player->_anim_time = PLAYER_ANIM_UPPER_FLIPOUT_TIME;
			}
			*/

			player->incWeaponFrame();
			break;
		}

		case WEAPON_READY:
		{
			if( 	((player->_buttons_latched|player->_buttons) & BUTTON_ATTACK) ||
				((player->_buttons_latched|player->_buttons) & BUTTON_ATTACK2) )
			{
				player->_buttons_latched &= ~BUTTON_ATTACK;
                        	player->_buttons_latched &= ~BUTTON_ATTACK2;
			
				if((!player->getAmmoIndex()) || (player->_pers.inventory[player->getAmmoIndex()] >= player->_pers.weapon->getQuantity()))
				{
					player->_weapon_state = WEAPON_ATTACKING1;
					player->_r.ps.gun_anim_frame = getAttack1AnimationFirstFrame();
					player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getAttack1AnimationName());

					// start the animation
					/*
					player->_anim_priority = ANIM_ATTACK;
					if (player->_r.ps.pmove.pm_flags & PMF_DUCKED)
					{
						player->_s.frame = FRAME_crattak1-1;
						player->_anim_end = FRAME_crattak9;
					}
					else
					{
						player->_s.frame = FRAME_attack1-1;
						player->_anim_end = FRAME_attack8;
					}
					*/
				
					//FIXME
					player->_anim_priority = ANIM_ATTACK;
					player->_anim_current = PLAYER_ANIM_IDLE;
					player->_anim_time = PLAYER_ANIM_UPPER_ATTACK1_TIME;
				}
				else
				{
					/*
					if (level.time >= player->_pain_debounce_time)
					{
						trap_SV_StartSound(NULL, ent, CHAN_VOICE, trap_SV_SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
						player->_pain_debounce_time = level.time + 1;
					}
					*/
				
					player->noAmmoWeaponChange();
				
					player->_weapon_state = WEAPON_DEACTIVATING;
					player->_r.ps.gun_anim_frame = getDeActivateAnimationFirstFrame();
					break;
				}
			}
			else
			{
				if(player->_newweapon)
				{	
					player->_weapon_state = WEAPON_DEACTIVATING;
					player->_r.ps.gun_anim_frame = getDeActivateAnimationFirstFrame();
					player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getDeActivateAnimationName());
					break;
				}
			
				if(player->_r.ps.gun_anim_frame >= getIdleAnimationLastFrame())
				{
					player->_r.ps.gun_anim_frame = getIdleAnimationFirstFrame();
					player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getIdleAnimationName());
					break;
				}
				
				player->incWeaponFrame();
				break;
			}
		}

		case WEAPON_ATTACKING1:
		{
			/*
			int n;
		
			for(n=0; _frames_fire[n]; n++)
			{
				if(player->_r.ps.gun_anim_frame == _frames_fire[n])
				{
					//if(player->_quad_framenum > level.framenum)
					//	trap_SV_StartSound(NULL, ent, CHAN_ITEM, trap_SV_SoundIndex("items/damage3.wav"), 1, ATTN_NORM, 0);
				
					//TODO
					//player->applyQuadSound();
					//player->applyHasteSound();

					fire(player);
					break;
				}
			}

			if(!_frames_fire[n])
				player->_r.ps.gun_anim_frame++;
			*/
			
			if(player->_r.ps.gun_anim_frame == getAttack1AnimationFirstFrame())
			{
				if(!player->_weapon_fired)
				{
					attack1(player);
					player->_weapon_fired = true;
				}
			}
			else if(player->_r.ps.gun_anim_frame >= getAttack1AnimationLastFrame())
			{
				player->_weapon_state = WEAPON_READY;
				player->_weapon_fired = false;
				player->_r.ps.gun_anim_frame = getIdleAnimationFirstFrame();
				player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getIdleAnimationName());
			}
		
			player->incWeaponFrame();
			break;
		}
		
		case WEAPON_RELOADING:
		{
			if(player->_r.ps.gun_anim_frame >= getReloadAnimationLastFrame())
			{
				player->_weapon_state = WEAPON_READY;
				player->_r.ps.gun_anim_frame = getIdleAnimationFirstFrame();
				player->_r.ps.gun_anim_index = trap_SV_AnimationIndex(getIdleAnimationName());
				break;
			}
			
			player->incWeaponFrame();
			break;
		}
		
		default:
		{
			break;
		}
	}
}


/*
#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void	weapon_grenade_fire(g_entity_c *ent, bool held)
{
	vec3_c	offset;
	vec3_c	forward, right, up;
	vec3_c	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	Vector3_Set(offset, 8, 8, ent->viewheight-8);
	Angles_ToVectors (ent->_v_angle, forward, right, up);
	P_ProjectSource (ent->getClient(), ent->s.origin, offset, forward, right, start);

	timer = ent->_grenade_time - level.time;
	speed = (int)(GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER));
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->getInteger() & DF_INFINITE_AMMO ) )
		ent->_pers.inventory[ent->getAmmoIndex()]--;

	ent->_grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->_r.ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->_anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->_anim_end = FRAME_crattak3;
	}
	else
	{
		ent->_anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->_anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (g_entity_c *ent)
{
	if ((ent->_newweapon) && (ent->_weapon_state == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->_weapon_state == WEAPON_ACTIVATING)
	{
		ent->_weapon_state = WEAPON_READY;
		ent->_r.ps.gunframe = 16;
		return;
	}

	if (ent->_weapon_state == WEAPON_READY)
	{
		if ( ((ent->_buttons_latched|ent->_buttons) & BUTTON_ATTACK) )
		{
			ent->_buttons_latched &= ~BUTTON_ATTACK;
			if (ent->_pers.inventory[ent->getAmmoIndex()])
			{
				ent->_r.ps.gunframe = 1;
				ent->_weapon_state = WEAPON_FIRING;
				ent->_grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					trap_SV_StartSound(NULL, ent, CHAN_VOICE, trap_SV_SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->_r.ps.gunframe == 29) || (ent->_r.ps.gunframe == 34) || (ent->_r.ps.gunframe == 39) || (ent->_r.ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->_r.ps.gunframe > 48)
			ent->_r.ps.gunframe = 16;
		return;
	}

	if (ent->_weapon_state == WEAPON_FIRING)
	{
		if (ent->_r.ps.gunframe == 5)
			trap_SV_StartSound(NULL, ent, CHAN_WEAPON, trap_SV_SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->_r.ps.gunframe == 11)
		{
			if (!ent->_grenade_time)
			{
				ent->_grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->_weapon_sound = trap_SV_SoundIndex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->_grenade_blew_up && level.time >= ent->_grenade_time)
			{
				ent->_weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->_grenade_blew_up = true;
			}

			if (ent->_buttons & BUTTON_ATTACK)
				return;

			if (ent->_grenade_blew_up)
			{
				if (level.time >= ent->_grenade_time)
				{
					ent->_r.ps.gunframe = 15;
					ent->_grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->_r.ps.gunframe == 12)
		{
			ent->_weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->_r.ps.gunframe == 15) && (level.time < ent->_grenade_time))
			return;

		ent->_r.ps.gunframe++;

		if (ent->_r.ps.gunframe == 16)
		{
			ent->_grenade_time = 0;
			ent->_weapon_state = WEAPON_READY;
		}
	}
}
*/

/*
================================================================================
			GRENADE LAUNCHER
================================================================================
*/
/*
g_item_weapon_grenade_launcher_c::g_item_weapon_grenade_launcher_c()
{
	_classname 	= "weapon_grenadelauncher";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_launch/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_launch/tris.md2";
	
	_icon		= "textures/pics/w_glauncher.pcx";
	_pickup_name	= "Grenade Launcher";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Grenades";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_GRENADELAUNCHER;

	_info		= NULL;
	_tag		= 0;

	_precaches.push_back("models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav";
	
	//
	// setup frames
	//
	int 	i, j=0;
	
	// setup activate frames
	for(i=0; i<5; i++, j++)
		_frames_activate.push_back(j);
		
	// setup fire frames
	for(i=0; i<11; i++, j++)
		_frames_attack.push_back(j);
	
	// setup idle frames
	for(i=0; i<37; i++, j++)
		_frames_idle.push_back(j);
		
	// setup deactivate frames
	for(i=0; i<5; i++, j++)
		_frames_deactivate.push_back(j);
	
	
	//_frames_pause.push_back(20);
	_frames_pause.push_back(0);
	
	//_frames_fire.push_back(74);
	_frames_fire.push_back(0);

}

void	g_item_weapon_grenade_launcher_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {34, 51, 59, 0};
	//static int	fire_frames[]	= {6, 0};

	thinkGeneric(ent);//, 0, 5, 16, 59, 64, pause_frames, fire_frames);
}

void	g_item_weapon_grenade_launcher_c::fire(g_player_c *ent)
{
	vec3_c	offset;
	vec3_c	forward, right, up;
	vec3_c	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	//if(is_quad)
	//	damage *= 4;

	Vector3_Set(offset, 8, 8, ent->_viewheight-8);
	Angles_ToVectors(ent->_v_angle, forward, right, up);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);

	Vector3_Scale (forward, -2, ent->_kick_origin);
	ent->_kick_angles[0] = -1;

	//if (ent->_buttons & BUTTON_ATTACK2)
	//	fire_proxy_grenade (ent, start, forward, damage, 600, 2.5, radius);
	//else
		fire_grenade(ent, start, forward, damage, 600, 2.5, radius);
	

	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_GRENADE);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_anim_frame++;

	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!((int)dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;
}
*/


/*
================================================================================
			ROCKET LAUNCHER
================================================================================
*/
/*
g_item_weapon_rocket_launcher_c::g_item_weapon_rocket_launcher_c()
{
	_classname 	= "weapon_rocketlauncher";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	//_world_model 		= "models/weapons/g_rocket/tris.md2";
	_world_model		= "models/weapons2/rocketl/rocketl.md3";
	_world_model_flags	= EF_NONE;//EF_ROTATE;
	_view_model 		= "models/weapons2/rocketl/rocketl_hand.md2";
	
	_icon		= "icons/iconw_rocket";
	_pickup_name	= "Rocket Launcher";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Rockets";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_ROCKETLAUNCHER;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "";
}

void	g_item_weapon_rocket_launcher_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {25, 33, 42, 50, 0};
	//static int	fire_frames[]	= {5, 0};

	//thinkGeneric(ent, 4, 12, 50, 54, pause_frames, fire_frames);
}

void	g_item_weapon_rocket_launcher_c::fire(g_player_c *ent)
{
	vec3_c	offset, start;
	vec3_c	forward, right, up;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	
	
	//if(is_quad)
	//{
	//	damage *= 4;
	//	radius_damage *= 4;
	//
	//

	Angles_ToVectors(ent->_v_angle, forward, right, up);

	Vector3_Scale(forward, -2, ent->_kick_origin);
	ent->_kick_angles[0] = -1;

	Vector3_Set(offset, 8, 8, ent->_viewheight-8);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);
	
#if 0
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
#else
	fire_sidewinder (ent, start, forward, damage, 650, damage_radius, radius_damage);
#endif

	// send muzzle flash
	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_ROCKET);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_frame++;

	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!((int)dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;
}

*/





/*
static void	Blaster_Fire(g_player_c *ent, vec3_t g_offset, int damage, bool hyper, int effect)
{
	vec3_c	forward, right, up;
	vec3_c	start;
	vec3_c	offset;

	//if(is_quad)
	//	damage *= 4;
		
	Angles_ToVectors(ent->_v_angle, forward, right, up);
	
	offset.set(24, 8, ent->_viewheight - 8);
	offset += g_offset;
	
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);

	ent->_kick_origin = forward * -2;
	
	ent->_kick_angles[0] = -1;

//	new g_projectile_bolt_c(ent, start, forward, damage, 1000, hyper);
	new g_projectile_rocket_c(ent, start, forward, damage, 1000, 120, 120);

	//fire_blaster(ent, start, forward, damage, 1000, effect, hyper);

	// send muzzle flash
	//trap_SV_WriteByte(SVC_MUZZLEFLASH);
	//trap_SV_WriteShort(ent->_s.getNumber());
	//if(hyper)
	//	trap_SV_WriteByte(MZ_HYPERBLASTER);
	//else
	//	trap_SV_WriteByte(MZ_BLASTER);
	//trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	//PlayerNoise(ent, start, PNOISE_WEAPON);
}
*/


/*
================================================================================
				BLASTER
================================================================================
*/
g_item_weapon_blaster_c::g_item_weapon_blaster_c()
{
	_classname 	= "weapon_blaster";
	
	_has_pickup		= false;
	_has_use		= true;
	_has_drop		= false;
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_model_world 		= "";
	_model_world_flags	= 0;
	_model_player		= "";
	_model_view 		= "models/weapons/pistol_view/viewpistol.md5mesh";
	
	_icon			= "textures/pics/w_blaster.pcx";
	_pickup_name		= "Blaster";
	_count_width		= 0;

	_quantity		= 0;			
	_ammo			= "Cells";			
	_flags			= IT_WEAPON | IT_STAY_COOP;	
	
	_weapmodel		= WEAP_BLASTER;

	_info			= NULL;
	_tag			= 0;

	_anim_activate		= registerAnimation("models/md5/weapons/pistol_view/raise.md5anim");
	
	_anim_idle		= registerAnimation("models/md5/weapons/pistol_view/idle.md5anim");
	
	_anim_attack1		= registerAnimation("models/md5/weapons/pistol_view/fire1.md5anim");
	_anim_attack2		= registerAnimation("models/md5/weapons/pistol_view/fire1.md5anim");
	_anim_attack3		= registerAnimation("models/md5/weapons/pistol_view/fire1.md5anim");
	_anim_attack4		= registerAnimation("models/md5/weapons/pistol_view/fire1.md5anim");
	
	_anim_reload		= registerAnimation("models/md5/weapons/pistol_view/reload_empty.md5anim");
	_anim_noammo		= registerAnimation("models/md5/weapons/pistol_view/empty_reload_start.md5anim");
	
	_anim_deactivate	= registerAnimation("models/md5/weapons/pistol_view/put_away.md5anim");
}

void	g_item_weapon_blaster_c::attack1(g_player_c *player)
{
	//trap_Com_Printf("g_item_weapon_blaster_c::attack1:\n");

	int		damage;

	if(deathmatch->getInteger())
		damage = 15;
	else
		damage = 10;
	
	vec3_c	start;
	vec3_c	offset(24, 8, player->getViewHeight() - 8);
	
	player->projectSource(offset, start);
	
	player->_kick_origin = player->getViewForward() * -2;
	player->_kick_angles[0] = -1;

//	new g_projectile_bolt_c(ent, start, forward, damage, 1000.0);
	new g_projectile_rocket_c(player, start, player->getViewQuaternion(), damage, 800, 120, 120);
}

void	g_item_weapon_blaster_c::reload(g_player_c *ent)
{

}


/*
================================================================================
				SHOTGUN
================================================================================
*/

g_item_weapon_shotgun_c::g_item_weapon_shotgun_c()
{
	_classname		= "weapon_shotgun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_model_world		= "models/weapons2/rocketl/rocketl.md3";
//	_model_world		= "models/weapons/shotgun/w_shotgun2.lwo";
	_model_world_flags	= EF_NONE;	//EF_ROTATE;
	_model_player		= "models/md5/weapons/shotgun_world/worldshotgun.md5mesh";
	_model_view		= "models/md5/weapons/shotgun_view/viewshotgun.md5mesh";
	
	_icon			= "textures/pics/w_shotgun.pcx";
	_pickup_name		= "Shotgun";
	_count_width		= 0;

	_quantity		= 1;
	_ammo			= "Shells";
	_flags			= IT_WEAPON | IT_STAY_COOP;

	_weapmodel		= WEAP_SHOTGUN;
	
	_info			= NULL;
	_tag			= 0;
	
	//_precaches		= "weapons/shotgf1b.wav misc/shotgr1b.wav";
	
	
	_anim_activate		= registerAnimation("models/md5/weapons/shotgun_view/raise.md5anim");
	
	_anim_idle		= registerAnimation("models/md5/weapons/shotgun_view/idle.md5anim");
	
	_anim_attack1		= registerAnimation("models/md5/weapons/shotgun_view/fire1.md5anim");
	_anim_attack2		= registerAnimation("models/md5/weapons/shotgun_view/fire2.md5anim");
	_anim_attack3		= registerAnimation("models/md5/weapons/shotgun_view/fire3.md5anim");
	_anim_attack4		= NULL;
	
	_anim_reload		= registerAnimation("models/md5/weapons/shotgun_view/reload_start.md5anim");
	_anim_noammo		= registerAnimation("models/md5/weapons/shotgun_view/empty.md5anim");
	
	_anim_deactivate	= registerAnimation("models/md5/weapons/shotgun_view/lower.md5anim");
}

void	g_item_weapon_shotgun_c::attack1(g_player_c *ent)
{
#if 0
	vec3_c		start;
	vec3_c		forward, right, up;
	vec3_c		offset;
	int			damage = 4;
	int			kick = 8;

	if(ent->_r.ps.gun_frame == 9)
	{
		ent->_r.ps.gun_frame++;
		return;
	}

	Angles_ToVectors (ent->_v_angle, forward, right, up);

	Vector3_Scale (forward, -2, ent->_kick_origin);
	ent->_kick_angles[0] = -2;

	Vector3_Set(offset, 0, 8,  ent->_viewheight-8);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);

	//if (is_quad)
	//{
	//	damage *= 4;
	//	kick *= 4;
	//}
	
	if(deathmatch->getInteger())
		fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_SHOTGUN);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_frame++;
	
	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!((int)dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;
#endif
}


/*
================================================================================
				HYPER BLASTER
================================================================================
*/
/*
g_item_weapon_hyper_blaster_c::g_item_weapon_hyper_blaster_c()
{
	_classname 	= "weapon_hyperblaster";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_hyperb/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_hyperb/tris.md2";
	
	_icon		= "textures/pics/w_hyperblaster.pcx";
	_pickup_name	= "HyperBlaster";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Cells";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_HYPERBLASTER;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav";
}
*/

/*
void	g_item_weapon_hyper_blaster_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {0};
	//static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	//thinkGeneric(ent, 5, 20, 49, 53, pause_frames, fire_frames);
}
*/

/*
void	g_item_weapon_hyper_blaster_c::fire(g_player_c *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->_weapon_sound = trap_SV_SoundIndex("weapons/hyprbl1a.wav");

	if (!(ent->_buttons & BUTTON_ATTACK))
	{
		ent->_r.ps.gunframe++;
	}
	else
	{
		if (! ent->_pers.inventory[ent->getAmmoIndex()] )
		{
			if (level.time >= ent->_pain_debounce_time)
			{
				trap_SV_StartSound(NULL, ent, CHAN_VOICE, trap_SV_SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->_pain_debounce_time = level.time + 1;
			}
			ent->noAmmoWeaponChange();
		}
		else
		{
			rotation = (ent->_r.ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->_r.ps.gunframe == 6) || (ent->_r.ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->getInteger())
				damage = 15;
			else
				damage = 20;
				
			Blaster_Fire (ent, offset, damage, true, effect);
			
			if (! ( (int)dmflags->getInteger() & DF_INFINITE_AMMO ) )
				ent->_pers.inventory[ent->getAmmoIndex()]--;

			ent->_anim_priority = ANIM_ATTACK;
			if(ent->_r.ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->_s.frame = FRAME_crattak1 - 1;
				ent->_anim_end = FRAME_crattak9;
			}
			else
			{
				ent->_s.frame = FRAME_attack1 - 1;
				ent->_anim_end = FRAME_attack8;
			}
		}

		ent->_r.ps.gunframe++;
		if (ent->_r.ps.gunframe == 12 && ent->_pers.inventory[ent->getAmmoIndex()])
			ent->_r.ps.gunframe = 6;
	}

	if (ent->_r.ps.gunframe == 12)
	{
		trap_SV_StartSound(NULL, ent, CHAN_AUTO, trap_SV_SoundIndex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->_weapon_sound = 0;
	}

}
*/


/*
================================================================================
				MACHINEGUN
================================================================================
*/
/*
g_item_weapon_machinegun_c::g_item_weapon_machinegun_c()
{
	_classname 	= "weapon_machinegun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_machn/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_machn/tris.md2";
	
	_icon		= "textures/pics/w_machinegun.pcx";
	_pickup_name	= "Machinegun";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Bullets";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_MACHINEGUN;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav";
}
*/

/*
void	g_item_weapon_machinegun_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {23, 45, 0};
	//static int	fire_frames[]	= {4, 5, 0};

	//thinkGeneric(ent, 3, 5, 45, 49, pause_frames, fire_frames);
}
*/

/*
void	g_item_weapon_machinegun_c::fire(g_player_c *ent)
{
	int	i;
	vec3_c		start;
	vec3_c		forward, right, up;
	vec3_c		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->_buttons & BUTTON_ATTACK))
	{
		ent->_machinegun_shots = 0;
		ent->_r.ps.gunframe++;
		return;
	}

	if (ent->_r.ps.gunframe == 5)
		ent->_r.ps.gunframe = 4;
	else
		ent->_r.ps.gunframe = 5;

	if (ent->_pers.inventory[ent->getAmmoIndex()] < 1)
	{
		ent->_r.ps.gunframe = 6;
		if (level.time >= ent->_pain_debounce_time)
		{
			trap_SV_StartSound(NULL, ent, CHAN_VOICE, trap_SV_SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->_pain_debounce_time = level.time + 1;
		}
		ent->noAmmoWeaponChange();
		return;
	}

	
	for (i=1 ; i<3 ; i++)
	{
		ent->_kick_origin[i] = crandom() * 0.35;
		ent->_kick_angles[i] = crandom() * 0.7;
	}
	ent->_kick_origin[0] = crandom() * 0.35;
	ent->_kick_angles[0] = ent->_machinegun_shots * -1.5;

	// raise the gun as it is firing
	if (!deathmatch->getInteger())
	{
		ent->_machinegun_shots++;
		if (ent->_machinegun_shots > 9)
			ent->_machinegun_shots = 9;
	}

	// get start / end positions
	Vector3_Add (ent->_v_angle, ent->_kick_angles, angles);
	Angles_ToVectors (angles, forward, right, up);
	Vector3_Set(offset, 0, 8, ent->_viewheight-8);
	P_ProjectSource (ent, ent->_s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_MACHINEGUN);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!((int)dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;

	ent->_anim_priority = ANIM_ATTACK;
	if(ent->_r.ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->_s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->_anim_end = FRAME_crattak9;
	}
	else
	{
		ent->_s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->_anim_end = FRAME_attack8;
	}
}
*/


/*
================================================================================
				CHAINGUN
================================================================================
*/
/*
g_item_weapon_chaingun_c::g_item_weapon_chaingun_c()
{
	_classname 	= "weapon_chaingun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_chain/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_chain/tris.md2";
	
	_icon		= "textures/pics/w_chaingun.pcx";
	_pickup_name	= "Chaingun";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Bullets";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_CHAINGUN;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav";
}
*/

/*
void	g_item_weapon_chaingun_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {38, 43, 51, 61, 0};
	//static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	//thinkGeneric(ent, 4, 31, 61, 64, pause_frames, fire_frames);
}
*/

/*
void	g_item_weapon_chaingun_c::fire(g_player_c *ent)
{
	int			i;
	int			shots;
	vec3_c		start;
	vec3_c		forward, right, up;
	float		r, u;
	vec3_c		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->getInteger())
		damage = 6;
	else
		damage = 8;

	if (ent->_r.ps.gunframe == 5)
		trap_SV_StartSound(NULL, ent, CHAN_AUTO, trap_SV_SoundIndex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->_r.ps.gunframe == 14) && !(ent->_buttons & BUTTON_ATTACK))
	{
		ent->_r.ps.gunframe = 32;
		ent->_weapon_sound = 0;
		return;
	}
	else if ((ent->_r.ps.gunframe == 21) && (ent->_buttons & BUTTON_ATTACK)
		&& ent->_pers.inventory[ent->getAmmoIndex()])
	{
		ent->_r.ps.gunframe = 15;
	}
	else
	{
		ent->_r.ps.gunframe++;
	}

	if (ent->_r.ps.gunframe == 22)
	{
		ent->_weapon_sound = 0;
		trap_SV_StartSound(NULL, ent, CHAN_AUTO, trap_SV_SoundIndex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->_weapon_sound = trap_SV_SoundIndex("weapons/chngnl1a.wav");
	}

	ent->_anim_priority = ANIM_ATTACK;
	if (ent->_r.ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->_s.frame = FRAME_crattak1 - (ent->_r.ps.gunframe & 1);
		ent->_anim_end = FRAME_crattak9;
	}
	else
	{
		ent->_s.frame = FRAME_attack1 - (ent->_r.ps.gunframe & 1);
		ent->_anim_end = FRAME_attack8;
	}

	if (ent->_r.ps.gunframe <= 9)
		shots = 1;
	else if (ent->_r.ps.gunframe <= 14)
	{
		if (ent->_buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->_pers.inventory[ent->getAmmoIndex()] < shots)
		shots = ent->_pers.inventory[ent->getAmmoIndex()];

	if (!shots)
	{
		if (level.time >= ent->_pain_debounce_time)
		{
			trap_SV_StartSound(NULL, ent, CHAN_VOICE, trap_SV_SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->_pain_debounce_time = level.time + 1;
		}
		ent->noAmmoWeaponChange();
		return;
	}


	for (i=0 ; i<3 ; i++)
	{
		ent->_kick_origin[i] = crandom() * 0.35;
		ent->_kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		Angles_ToVectors (ent->_v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		Vector3_Set(offset, 0, r, u + ent->_viewheight-8);
		P_ProjectSource (ent, ent->_s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	trap_SV_WriteByte (SVC_MUZZLEFLASH);
	trap_SV_WriteShort (ent->_s.getNumber());
	trap_SV_WriteByte ((MZ_CHAINGUN1 + shots - 1));
	trap_SV_Multicast (ent->_s.origin, MULTICAST_PVS);

	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->getInteger() & DF_INFINITE_AMMO ) )
		ent->_pers.inventory[ent->getAmmoIndex()] -= shots;
}
*/






/*
================================================================================
				SUPER SHOTGUN
================================================================================
*/
/*
g_item_weapon_super_shotgun_c::g_item_weapon_super_shotgun_c()
{
	_classname 	= "weapon_supershotgun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_shotg2/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_shotg2/tris.md2";
	
	_icon		= "textures/pics/w_sshotgun.pcx";
	_pickup_name	= "Super Shotgun";
	_count_width	= 0;

	_quantity	= 2;
	_ammo		= "Shells";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_SUPERSHOTGUN;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "weapons/sshotf1b.wav";
}

void	g_item_weapon_super_shotgun_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {29, 42, 57, 0};
	//static int	fire_frames[]	= {7, 0};

	//thinkGeneric(ent, 6, 17, 57, 61, pause_frames, fire_frames);
}

void	g_item_weapon_super_shotgun_c::fire(g_player_c *ent)
{
	vec3_c		start;
	vec3_c		forward, right, up;
	vec3_c		offset;
	vec3_c		v;
	int			damage = 6;
	int			kick = 12;

	Angles_ToVectors (ent->_v_angle, forward, right, up);

	Vector3_Scale (forward, -2, ent->_kick_origin);
	ent->_kick_angles[0] = -2;

	Vector3_Set(offset, 0, 8,  ent->_viewheight-8);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);
	
	//if (is_quad)
	//{
	//	damage *= 4;
	//	kick *= 4;
	//}

	v[PITCH] = ent->_v_angle[PITCH];
	v[YAW]   = ent->_v_angle[YAW] - 5;
	v[ROLL]  = ent->_v_angle[ROLL];
	Angles_ToVectors (v, forward, right, up);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->_v_angle[YAW] + 5;
	Angles_ToVectors (v, forward, right, up);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_SSHOTGUN);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_frame++;
	
	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!(dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()] -= 2;
}
*/


/*
================================================================================
				RAILGUN
================================================================================
*/
/*
g_item_weapon_railgun_c::g_item_weapon_railgun_c()
{
	_classname 	= "weapon_railgun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_rail/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_rail/tris.md2";
	
	_icon		= "textures/pics/w_railgun.pcx";
	_pickup_name	= "Railgun";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Slugs";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_RAILGUN;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "weapons/rg_hum.wav";
}

void	g_item_weapon_railgun_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {56, 0};
	//static int	fire_frames[]	= {4, 0};

	//thinkGeneric(ent, 3, 18, 56, 61, pause_frames, fire_frames);
}

void	g_item_weapon_railgun_c::fire(g_player_c *ent)
{
	vec3_c		start;
	vec3_c		forward, right, up;
	vec3_c		offset;
	int			damage;
	int			kick;

	if(deathmatch->getInteger())
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	//if (is_quad)
	//{
	//	damage *= 4;
	//	kick *= 4;
	//}

	Angles_ToVectors (ent->_v_angle, forward, right, up);

	Vector3_Scale (forward, -3, ent->_kick_origin);
	ent->_kick_angles[0] = -3;

	Vector3_Set(offset, 0, 7,  ent->_viewheight-8);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);
	fire_rail(ent, start, forward, damage, kick);

	// send muzzle flash
	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_RAILGUN);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_frame++;
	
	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!(dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;
}
*/


/*
================================================================================
				BIG FREAKIN GUN
================================================================================
*/
/*
g_item_weapon_bfg_c::g_item_weapon_bfg_c()
{
	_classname 	= "weapon_bfg";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model 		= "models/weapons/g_bfg/tris.md2";
	_world_model_flags	= EF_ROTATE;
	_view_model 		= "models/weapons/v_bfg/tris.md2";
	
	_icon		= "textures/pics/w_bfg.pcx";
	_pickup_name	= "BFG10K";
	_count_width	= 0;

	_quantity	= 50;
	_ammo		= "Cells";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_BFG;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav";
}
*/

/*
void	g_item_weapon_bfg_c::weaponThink(g_player_c *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	thinkGeneric(ent, 8, 32, 55, 58, pause_frames, fire_frames);
}
*/

/*
void	g_item_weapon_bfg_c::fire(g_entity_c *ent)
{
	vec3_c	offset, start;
	vec3_c	forward, right, up;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->getInteger())
		damage = 200;
	else
		damage = 500;

	if (ent->_r.ps.gunframe == 9)
	{
		// send muzzle flash
		trap_SV_WriteByte (SVC_MUZZLEFLASH);
		trap_SV_WriteShort (ent->s.number);
		trap_SV_WriteByte (MZ_BFG | is_silenced);
		trap_SV_Multicast (ent->s.origin, MULTICAST_PVS);

		ent->_r.ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->_pers.inventory[ent->getAmmoIndex()] < 50)
	{
		ent->_r.ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	Angles_ToVectors (ent->_v_angle, forward, right, up);

	Vector3_Scale (forward, -2, ent->_kick_origin);

	// make a big pitch kick with an inverse fall
	ent->_v_dmg_pitch = -40;
	ent->_v_dmg_roll = crandom()*8;
	ent->_v_dmg_time = level.time + DAMAGE_TIME;

	Vector3_Set(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->getClient(), ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->_r.ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->getInteger() & DF_INFINITE_AMMO ) )
		ent->_pers.inventory[ent->getAmmoIndex()] -= 50;
}
*/






/*
================================================================================
				IONBLASTER
================================================================================
*/
/*
g_item_weapon_ionblaster_c::g_item_weapon_ionblaster_c()
{
	_classname 	= "weapon_ionblaster";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	//_world_model		= "models/e1/a_ionb.md2";
	_world_model		= "models/weapons2/rocketl/rocketl.md3";
	_world_model_flags	= EF_ROTATE;
	_view_model		= "models/e1/w_ionblaster.md2";
	
	_icon		= "textures/pics/w_machinegun.pcx";
	_pickup_name	= "Ion Blaster";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Cells";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_IONBLASTER;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "";
	
	
	int 	i, j=0;
	
	// setup amba + ambb frames
	for(i=0; i<(20+40); i++, j++)
		_frames_idle.push_back(j);
		
	// setup away frames
	for(i=0; i<7; i++, j++)
		_frames_deactivate.push_back(j);
	
	// setup ready frames
	for(i=0; i<6; i++, j++)
		_frames_activate.push_back(j);
	
	// setup shoota frames
	for(i=0; i<8; i++, j++)
		_frames_attack.push_back(j);
		
	
	_frames_pause.push_back(20);
	_frames_pause.push_back(0);
	
	_frames_fire.push_back(74);
	_frames_fire.push_back(0);
}

void	g_item_weapon_ionblaster_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {20, 0};
	//static int	fire_frames[]	= {74, 75, 76, 77, 78, 79, 80, 0};
	//static int	fire_frames[]	= {74, 0};

	thinkGeneric(ent);	// 68, 73, 74, 79, 1, 60, 41, 47, pause_frames, fire_frames);
}

void	g_item_weapon_ionblaster_c::fire(g_player_c *ent)
{
	Blaster_Fire(ent, vec3_origin, 15, false, EF_BLASTER);
	
	ent->_r.ps.gun_frame++;
}
*/

/*
================================================================================
				SIDEWINDER
================================================================================
*/
/*
g_item_weapon_sidewinder_c::g_item_weapon_sidewinder_c()
{
	_classname 	= "weapon_sidewinder";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	_world_model		= "models/weapons2/rocketl/rocketl.md3";
	_world_model_flags	= EF_NONE;//EF_ROTATE;
	_view_model		= "models/e1/w_sidewinder.md2";
	
	_icon		= "icons/iconw_rocket.tga";
	_pickup_name	= "Sidewinder";
	_count_width	= 0;

	_quantity	= 1;
	_ammo		= "Rockets";
	_flags		= IT_WEAPON | IT_STAY_COOP;

	_weapmodel	= WEAP_SIDEWINDER;

	_info		= NULL;
	_tag		= 0;

	_precaches	= "";
	
	
	
	int 	i, j=0;
	
	// setup amba + ambb frames
	for(i=0; i<(26+35); i++, j++)
		_frames_idle.push_back(j);
		
	// setup away frames
	for(i=0; i<7; i++, j++)
		_frames_deactivate.push_back(j);
	
	// setup ready frames
	for(i=0; i<7; i++, j++)
		_frames_activate.push_back(j);
	
	// setup shoot frames
	for(i=0; i<25; i++, j++)
		_frames_attack.push_back(j);
		
	
	_frames_pause.push_back(26);
	_frames_pause.push_back(0);
	
	_frames_fire.push_back(76);
	_frames_fire.push_back(0);
	
	
}

void	g_item_weapon_sidewinder_c::weaponThink(g_player_c *ent)
{
	//TODO
	
	//static int	pause_frames[]	= {26, 35, 0};
	//static int	fire_frames[]	= {76, 0};

	thinkGeneric(ent);	//, 69, 75, 76, 100, 1, 61, 62, 68, pause_frames, fire_frames);
}

void	g_item_weapon_sidewinder_c::fire(g_player_c *ent)
{
	vec3_c	offset, start;
	vec3_c	forward, right, up;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	
	//if (is_quad)
	//{
	//	damage *= 4;
	//	radius_damage *= 4;
	//}

	Angles_ToVectors (ent->_v_angle, forward, right, up);

	Vector3_Scale (forward, -2, ent->_kick_origin);
	ent->_kick_angles[0] = -1;

	Vector3_Set(offset, 8, 8, ent->_viewheight-8);
	P_ProjectSource(ent, ent->_s.origin, offset, forward, right, start);
	
#if 1
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
#else
	fire_sidewinder (ent, start, forward, damage, 650, damage_radius, radius_damage);
#endif

	// send muzzle flash
	trap_SV_WriteByte(SVC_MUZZLEFLASH);
	trap_SV_WriteShort(ent->_s.getNumber());
	trap_SV_WriteByte(MZ_SIDEWINDER);
	trap_SV_Multicast(ent->_s.origin, MULTICAST_PVS);

	ent->_r.ps.gun_frame++;

	//PlayerNoise(ent, start, PNOISE_WEAPON);

	if(!((int)dmflags->getInteger() & DF_INFINITE_AMMO))
		ent->_pers.inventory[ent->getAmmoIndex()]--;
}
*/



/*
================================================================================
				NAILGUN
================================================================================
*/
/*
g_item_weapon_nailgun_c::g_item_weapon_nailgun_c()
{
	_classname 	= "weapon_nailgun";
	
	_pickup_sound 		= "misc/w_pkup.wav";
	//_world_model 		= "models/weapons/nailgun/w_nailgun.md5mesh";
	_world_model		= "models/weapons2/rocketl/rocketl.md3";
	_world_model_flags	= 0;
	_view_model 		= "models/weapons/nailgun/v_nailgun.md5mesh";
	
	_icon			= "textures/pics/w_blaster.pcx";
	_pickup_name		= "Nailgun";
	_count_width		= 0;

	_quantity		= 0;		
	_ammo			= "Cells";			
	_flags			= IT_WEAPON | IT_STAY_COOP;	
	
	_weapmodel		= WEAP_NAILGUN;

	_info			= NULL;
	_tag			= 0;

	_precaches		= "";
	
	_anim_activate		= "models/weapons/nailgun/v_nailgun_activate.md5anim";
	
	_anim_idle		= "models/weapons/nailgun/v_nailgun_idle.md5anim";
	
	_anim_fire1		= "models/weapons/nailgun/v_nailgun_firex.md5anim";
	_anim_fire2		= "models/weapons/nailgun/v_nailgun_firex.md5anim";
	_anim_fire3		= "models/weapons/nailgun/v_nailgun_firex.md5anim";
	_anim_fire4		= "models/weapons/nailgun/v_nailgun_firex.md5anim";
	
	_anim_reload		= "models/weapons/nailgun/v_nailgun_reload.md5anim";
	_anim_noammo		= "models/weapons/nailgun/v_nailgun_noammo.md5anim";
	
	_anim_deactivate	= "models/weapons/nailgun/v_nailgun_deactivate.md5anim";
	
	
	int 	i, j=0;
	
	// setup amba + ambb frames
	for(i=0; i<(20+40); i++, j++)
		_frames_idle.push_back(j);
		
	// setup away frames
	for(i=0; i<7; i++, j++)
		_frames_deactivate.push_back(j);
	
	// setup ready frames
	for(i=0; i<6; i++, j++)
		_frames_activate.push_back(j);
	
	// setup shoota frames
	for(i=0; i<8; i++, j++)
		_frames_attack.push_back(j);
		
	
	_frames_pause.push_back(20);
	_frames_pause.push_back(0);
	
	_frames_fire.push_back(74);
	_frames_fire.push_back(0);
}


void	g_item_weapon_nailgun_c::weaponThink(g_player_c *ent)
{
	thinkGeneric(ent);
}

void	g_item_weapon_nailgun_c::fire(g_player_c *ent)
{
	int		damage;

	if(deathmatch->getInteger())
		damage = 15;
	else
		damage = 10;
		
	Blaster_Fire(ent, vec3_origin, damage, false, EF_BLASTER);
	
	ent->_r.ps.gun_frame++;

}
*/
