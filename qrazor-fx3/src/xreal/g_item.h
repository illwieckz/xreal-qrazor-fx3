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
#ifndef G_ITEM_H
#define G_ITEM_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_entity.h"



struct gitem_armor_t
{
	int		base_count;
	int		max_count;
	float		normal_protection;
	float		energy_protection;
	int		armor;
};


// g_item_c->flags
enum item_flags_e
{

	IT_WEAPON		= (1<<0),		// use makes active weapon
	IT_AMMO			= (1<<1),
	IT_ARMOR		= (1<<2),
	IT_STAY_COOP		= (1<<3),
	IT_KEY			= (1<<4),
	IT_POWERUP		= (1<<5)
};

// g_item_c->weapmodel for weapons indicates model index
enum
{
	WEAP_BLASTER		= 1, 
	WEAP_SHOTGUN,	
	WEAP_SUPERSHOTGUN, 
	WEAP_MACHINEGUN,
	WEAP_CHAINGUN,
	WEAP_GRENADES,
	WEAP_GRENADELAUNCHER,
	WEAP_ROCKETLAUNCHER,
	WEAP_HYPERBLASTER,
	WEAP_RAILGUN,
	WEAP_BFG,
	
	WEAP_IONBLASTER,
	WEAP_SIDEWINDER,
	
	WEAP_NAILGUN
};

//
// forward declarations
//
class g_player_c;



class g_item_c
{
public:
	//
	// constructor / destructor
	//
	virtual ~g_item_c();
	
	
	//
	// virtual functions
	//
	virtual bool	pickup(g_entity_c *ent, g_entity_c *player_c)	{return false;}
	virtual void	use(g_player_c *player)				{}
	virtual void	drop(g_player_c *player)			{}
	
		
	//
	// access
	//
	const char*	getClassname()			{return _classname.c_str();}
	
	bool		hasPickup()			{return _has_pickup;}
	bool		hasUse()			{return _has_use;}
	bool		hasDrop()			{return _has_drop;}
	
	bool		isWeapon() const		{return _flags & IT_WEAPON;}
	
	const char*	getPickupSound()		{return _pickup_sound.c_str();}
	const char*	getWorldModel()			{return _model_world.c_str();}
	const char*	getWorldModelAnimation()	{return _model_world_animation.c_str();}
	int		getWorldModelFlags()		{return _model_world_flags;}
	const char*	getPlayerModel()		{return _model_player.c_str();}
	const char*	getViewModel()			{return _model_view.c_str();}
	
	const char*	getIcon()			{return _icon.c_str();}
	const char*	getPickupName()			{return _pickup_name.c_str();}
	int		getCountWidth()			{return _count_width;}
	
	int		getQuantity()			{return _quantity;}
	const char*	getAmmo()			{return _ammo.c_str();}
	int		getWeaponModel()		{return _weapmodel;}
	
	int		getFlags()			{return _flags;}
	void*		getInfo()			{return _info;}
	int		getTag()			{return _tag;}
	
	const std::vector<std::string>&	getPrecaches()	{return _precaches;}

protected:
	
	// helper function
	animation_c*		registerAnimation(const std::string &name);
	

	std::string			_classname;	// spawning name

	bool				_has_pickup;
	bool				_has_use;
	bool				_has_drop;

	std::string			_pickup_sound;
	
	std::string			_model_world;
	std::string			_model_world_animation;
	uint_t				_model_world_flags;
	std::string			_model_player;
	std::string			_model_view;

	// client side info
	std::string			_icon;
	std::string			_pickup_name;		// for printing on pickup
	int				_count_width;		// number of digits to display by icon

	int				_quantity;		// for ammo how much, for weapons how much is used per shot
	std::string			_ammo;			// for weapons
	int				_flags;			// IT_* flags

	int				_weapmodel;		// weapon model index (for weapons)

	void*				_info;
	int				_tag;

	std::vector<std::string>	_precaches;		// string of all models, sounds, and images this item will use
};



class g_item_weapon_c : public g_item_c
{
public:
	g_item_weapon_c();
	
	virtual ~g_item_weapon_c();
	
	virtual bool	pickup(g_entity_c *ent, g_player_c *other);
	virtual void	use(g_player_c *player);
	virtual void	drop(g_player_c *player);
	virtual void	weaponThink(g_player_c *player);
	
	const char*	getActivateAnimationName()		{return _anim_activate->getName();}
	int		getActivateAnimationFramesNum()		{return _anim_activate->getFramesNum();}
	int		getActivateAnimationFrameRate()		{return _anim_activate->getFrameRate();}
//	int		getActivateAnimationFirstFrame()	{return _anim_activate->getFirstFrame();}
//	int		getActivateAnimationLastFrame()		{return _anim_activate->getLastFrame();}
	
	const char*	getIdleAnimationName()			{return _anim_idle->getName();}
	int		getIdleAnimationFramesNum()		{return _anim_idle->getFramesNum();}
	int		getIdleAnimationFrameRate()		{return _anim_idle->getFrameRate();}
//	int		getIdleAnimationFirstFrame()		{return _anim_idle->getFirstFrame();}
//	int		getIdleAnimationLastFrame()		{return _anim_idle->getLastFrame();}
	
	const char*	getAttack1AnimationName()		{return _anim_attack1->getName();}
	int		getAttack1AnimationFramesNum()		{return _anim_attack1->getFramesNum();}
	int		getAttack1AnimationFrameRate()		{return _anim_attack1->getFrameRate();}
//	int		getAttack1AnimationFirstFrame()		{return _anim_attack1->getFirstFrame();}
//	int		getAttack1AnimationLastFrame()		{return _anim_attack1->getLastFrame();}

	const char*	getReloadAnimationName()		{return _anim_reload->getName();}
	int		getReloadAnimationFramesNum()		{return _anim_reload->getFramesNum();}
	int		getReloadAnimationFrameRate()		{return _anim_reload->getFrameRate();}
//	int		getReloadAnimationFirstFrame()		{return _anim_reload->getFirstFrame();}
//	int		getReloadAnimationLastFrame()		{return _anim_reload->getLastFrame();}
	
	const char*	getNoAmmoAnimationName()		{return _anim_noammo->getName();}
	int		getNoAmmoAnimationFramesNum()		{return _anim_noammo->getFramesNum();}
	int		getNoAmmoAnimationFrameRate()		{return _anim_noammo->getFrameRate();}
//	int		getNoAmmoAnimationFirstFrame()		{return _anim_noammo->getFirstFrame();}
//	int		getNoAmmoAnimationLastFrame()		{return _anim_noammo->getLastFrame();}
	
	const char*	getDeActivateAnimationName()		{return _anim_deactivate->getName();}
	int		getDeActivateAnimationFramesNum()	{return _anim_deactivate->getFramesNum();}
	int		getDeActivateAnimationFrameRate()	{return _anim_deactivate->getFrameRate();}
//	int		getDeActivateAnimationFirstFrame()	{return _anim_deactivate->getFirstFrame();}
//	int		getDeActivateAnimationLastFrame()	{return _anim_deactivate->getLastFrame();}
	

	virtual void	attack1(g_player_c *player)		{};
	virtual void	attack2(g_player_c *player)		{};
	virtual void	reload(g_player_c *player)		{};
	
protected:		
	animation_c*		_anim_activate;
	
	animation_c*		_anim_idle;
	
	animation_c*		_anim_attack1;
	animation_c*		_anim_attack2;
	animation_c*		_anim_attack3;
	animation_c*		_anim_attack4;
	
	animation_c*		_anim_reload;
	animation_c*		_anim_noammo;
	
	animation_c*		_anim_deactivate;
};


class g_item_weapon_blaster_c : public g_item_weapon_c
{
public:
	g_item_weapon_blaster_c();
	
	virtual void	attack1(g_player_c *ent);
	virtual void	reload(g_player_c *ent);
};

class g_item_weapon_shotgun_c : public g_item_weapon_c
{
public:
	g_item_weapon_shotgun_c();
	
	virtual void	attack1(g_player_c *ent);
};

/*
class g_item_weapon_super_shotgun_c : public g_item_weapon_c
{
public:
	g_item_weapon_super_shotgun_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_machinegun_c : public g_item_weapon_c
{
public:
	g_item_weapon_machinegun_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_chaingun_c : public g_item_weapon_c
{
public:
	g_item_weapon_chaingun_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_grenade_launcher_c : public g_item_weapon_c
{
public:
	g_item_weapon_grenade_launcher_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_rocket_launcher_c : public g_item_weapon_c
{
public:
	g_item_weapon_rocket_launcher_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_hyper_blaster_c : public g_item_weapon_c
{
public:
	g_item_weapon_hyper_blaster_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_railgun_c : public g_item_weapon_c
{
public:
	g_item_weapon_railgun_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_bfg_c : public g_item_weapon_c
{
public:
	g_item_weapon_bfg_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_entity_c *ent);
};
*/

/*
class g_item_weapon_ionblaster_c : public g_item_weapon_c
{
public:
	g_item_weapon_ionblaster_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_sidewinder_c : public g_item_weapon_c
{
public:
	g_item_weapon_sidewinder_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/

/*
class g_item_weapon_nailgun_c : public g_item_weapon_c
{
public:
	g_item_weapon_nailgun_c();
	
	virtual void	weaponThink(g_player_c *ent);
	virtual void	fire(g_player_c *ent);
};
*/



class g_item_dropable_c : public g_entity_c
{
public:
	g_item_dropable_c(g_player_c *player, g_item_c *item, const vec3_c &position, const quaternion_c &orientation, const vec3_c &velocity);
	~g_item_dropable_c();
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
};


class g_item_spawnable_c : public g_entity_c
{
public:
	g_item_spawnable_c(g_item_c *item);
	
	virtual void	think();
	virtual bool	touch(g_entity_c *other, const plane_c &plane, csurface_c *surf);
	virtual void	activate();
};


#endif // G_ITEM_H
