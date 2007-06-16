/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2007 Jeremy Hughes <Encryption767@msn.com>

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
#include "../game/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#ifdef MISSIONPACK
#define CG_FONT_THRESHOLD 0.1
#endif

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1000
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	40
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3800

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		60
#define TEAMCHAT_HEIGHT		8

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10

// Ridah, trails
#define	STYPE_STRETCH	0
#define	STYPE_REPEAT	1

#define	TJFL_FADEIN		(1<<0)
#define	TJFL_CROSSOVER	(1<<1)
#define	TJFL_NOCULL		(1<<2)
#define	TJFL_FIXDISTORT	(1<<3)
#define TJFL_SPARKHEADFLARE (1<<4)
#define	TJFL_NOPOLYMERGE	(1<<5)
// done.

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"visor"
#define	DEFAULT_HEADMODEL		"visor"


#define DEFAULT_REDTEAM_NAME		"Stroggs"
#define DEFAULT_BLUETEAM_NAME		"Pagans"

typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
} impactSound_t;

//=================================================
#define MAX_TESLA_BOLTS				2
// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
	lerpFrame_t		legs, torso, flag;
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

	// railgun trail spawning
	vec3_t			railgunImpact;
	qboolean		railgunFlash;

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	int				RbarrelTime;
	qboolean		barrelSpinning;
	refEntity_t		legsRefEnt, torsoRefEnt, headRefEnt, gunRefEnt;

	vec3_t			teslaEndPoints[MAX_TESLA_BOLTS];
	int				teslaEndPointTimes[MAX_TESLA_BOLTS];// time the bolt stays valid
	vec3_t			teslaOffsetDirs[MAX_TESLA_BOLTS];	// bending direction from center or direct beam
	float			teslaOffsets[MAX_TESLA_BOLTS];		// amount to offset from center
	int				teslaOffsetTimes[MAX_TESLA_BOLTS];	// time the offset stays valid
	int				teslaEnemy[MAX_TESLA_BOLTS];
	int				teslaDamageApplyTime;
	qboolean		teslaFlash;
	int				teslaDamagedTime;		// time we were last hit by a tesla bolt
	int				lightningSoundTime;
	int				armorDamagedTime;		// time we were last hit by a tesla bolt


} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity
	playerState_t		*ps;

	int		parent;

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;
	int				teleportFlag;

	int				startTime;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;
	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	qboolean	takedamage;

	qboolean	inwater;
	qboolean	watertraced;

	int			damage;
	int			splashDamage;	// quad will increase this without increasing radius
	int			splashRadius;
	int			methodOfDeath;
	int			splashMethodOfDeath;

	vec3_t			beamEnd;

	// Ridah, trail effects
	int				headJuncIndex, headJuncIndex2, headJuncIndex3, headJuncIndex4;
	int				lastTrailTime;
	// done.

	// client-side lightning
	int				boltTimes[MAX_TESLA_BOLTS];
	vec3_t			boltLocs[MAX_TESLA_BOLTS];
	vec3_t			boltCrawlDirs[MAX_TESLA_BOLTS];

	lerpFrame_t		legs;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;
	byte			areamask[MAX_MAP_AREA_BYTES];
	vec3_t			axis[3];
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
//	LE_ROCKET,
//	LE_PLASMA,
	LE_BODYFRAGMENT,
	LE_QFRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_FADE_RGBA,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
	LE_ADDSPARK,
	LE_ADDROCKETSPARK,
	LE_ADDGRENADESPARK,
	LE_ROCKET_EXPLOSION,
	LE_ADDRAILSPARK,
	LE_ROCKETRING,
	LE_QUADSPHERE,
	LE_PQUADSPHERE,
	LE_ROCKETRING2,
	LE_FLARE,
	LE_ADDBFGSPARK,
	LE_RAIL_EFFECT,
	LE_RAIL_EFFECT2,
	LE_KAMIKAZE,
	LE_RAILEXPRING,
	LE_RAILEXPRING2,
	LE_RAILRING,
	LE_RAILDISC,
	LE_ROCKETEXP,
	LE_RGEXP,
	LE_RGPARTEXP,
	LE_HASTEFX,
	LE_PROPS,
#ifdef MISSIONPACK
	LE_INVULIMPACT,
	LE_INVULJUICED,
	LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2			 = 0x0008,			// sound 2 for kamikaze
	LEF_PUFF_SCALE_DOWN  = 0x0010,			// scale size down over time
	LEF_NO_RGB_FADE      = 0x0020,			// dont fade rgb values
	LEF_NO_ALPHA_FADE    = 0x0040,
	LEF_ON_FIRE		     = 0x0080,
	LEF_NEVER_FREE		 = 0x0100
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_QBLOOD,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS,
	LEBS_SHELLS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	entityState_t	currentState;
	centity_t		*Cent;
	leType_t		leType;
	int				leFlags;
    float			width;
	vec4_t			quatOrient;
	vec4_t			quatRot;
	vec3_t			rotAxis;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				lastTrailTime;
	int				headJuncIndex, headJuncIndex2, headJuncIndex4, headJuncIndex3;
	float angVel;

	qboolean		anim;
	int anims;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;
	float			endradius;
	int				generic1;
	float			light;
	vec3_t			lightColor;
	float			light2;
	vec3_t			lightColor2;

	float			effectWidth;
	int				effectFlags;
	struct localEntity_s	*chain;	
	int				ownerNum;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;
	int			shaderAnim;
	int			trailLength;
	qhandle_t hShader;
	float	startSize;
	float	endSize;
	int step;
	refEntity_t		refEntity;
	refEntity_t		legs;
	refEntity_t		head;
	refEntity_t		torso;
	int	NumBounces;
	int view;
	vec3_t			mins;
	vec3_t			maxs;
	int			entNumber;
	int			contents;
	qboolean		Take_Damage;
	int				splashTime;
	qboolean		inwater;
	qboolean		watertraced;
	int		speed;
	int		duration;
	int		count;

} localEntity_t;

typedef enum {
	CFXF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	CFXF_TUMBLE			  = 0x0002,			// tumble over time, used for ejecting shells
	CFXF_SOUND1			  = 0x0004,			// sound 1 for kamikaze
	CFXF_SOUND2			  = 0x0008,			// sound 2 for kamikaze
	CFXF_PUFF_SCALE_DOWN  = 0x0010,			// scale size down over time
	CFXF_NO_RGB_FADE      = 0x0020,			// dont fade rgb values
	CFXF_NO_ALPHA_FADE    = 0x0040,
	CFXF_ON_FIRE		  = 0x0080,
	CFXF_NEVER_FREE		  = 0x0100,
	CFXF_DISTANCE_SCALE	  = 0x0200
} cfxFlag_t;

typedef enum {
	CFX_CORONA
} cfxType_t;

typedef struct cameraFX_s {
	struct cameraFX_s	*prev, *next;
	entityState_t	currentState;
	centity_t		*Cent;
	cfxType_t		cfxType;
	int				cfxFlags;
	float angVel;
	int				startTime;
	int				endTime;
	int				fadeInTime;
	float			lifeRate;			// 1.0 / (endTime - startTime)
	trajectory_t	pos;
	trajectory_t	angles;
	struct cameraFX_s	*chain;	
	qhandle_t hShader;
	float	startSize;
	float	endSize;
	int step;
	refEntity_t		refEntity;
	vec3_t			mins;
	vec3_t			maxs;
	int		duration;
	float			color[4];
	float			radius;
	float			MaxRadius;
	float			MinRadius;
	vec3_t			origin;
} cameraFX_t;

typedef enum {
	MfXF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	MfXF_TUMBLE			  = 0x0002,			// tumble over time, used for ejecting shells
	MfXF_SOUND1			  = 0x0004,			// sound 1 for kamikaze
	MfXF_SOUND2			  = 0x0008,			// sound 2 for kamikaze
	MfXF_PUFF_SCALE_DOWN  = 0x0010,			// scale size down over time
	MfXF_NO_RGB_FADE      = 0x0020,			// dont fade rgb values
	MfXF_NO_ALPHA_FADE    = 0x0040,
	MfXF_ON_FIRE		  = 0x0080,
	MFXF_NEVER_FREE		  = 0x0100
} mfxFlag_t;

typedef enum {
	MFX_CORONA
} mfxType_t;

typedef struct mapFX_s {
	struct mapFX_s	*prev, *next;
	entityState_t	currentState;
	centity_t		*Cent;
	mfxType_t		mfxType;
	int				mfxFlags;
	float angVel;
	int				startTime;
	int				endTime;
	int				fadeInTime;
	float			lifeRate;			// 1.0 / (endTime - startTime)
	trajectory_t	pos;
	trajectory_t	angles;
	struct mapFX_s	*chain;	
	qhandle_t hShader;
	float	startSize;
	float	endSize;
	int step;
	vec3_t			mins;
	vec3_t			maxs;
	int		duration;
	float			color[4];
	float			radius;
	float			endradius;
	vec3_t			origin;
} mapFX_t;

//======================================================================
#define	MAX_SHADER_ANIMS		32
#define	MAX_SHADER_ANIM_FRAMES	64

typedef struct {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	int				ref;
	qboolean		perfect;
	int				team;
} score_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	256

typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	vec3_t			color;
	vec3_t			color1;
	vec3_t			color2;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				armor;
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;

	int				breathPuffTime;

	// when clientinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	char			headModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
	char			redTeam[MAX_TEAMNAME];
	char			blueTeam[MAX_TEAMNAME];
	qboolean		deferred;

	qboolean		newAnims;		// true if using the new mission pack animations
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t       legsAnimation;
	qhandle_t       torsoAnimation;

	qhandle_t		headModel;
	qhandle_t		headSkin;
	qhandle_t		modelIcon;

	animation_t		animations[MAX_TOTALANIMATIONS];

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
	int			RailRing;
	int			RailTrail;

	int			accuracy_shotrg;		// total number of hits
	int			accuracy_shotrl;		// total number of hits
	int			accuracy_shotsg;		// total number of hits
	int			accuracy_shotmg;		// total number of hits
	int			accuracy_shotbfg;		// total number of hits
	int			accuracy_shotpg;		// total number of hits
	int			accuracy_shotgl;		// total number of hits
	int			accuracy_shotlg;		// total number of hits

	int			accuracy_hitrg;		// total number of hits
	int			accuracy_hitrl;		// total number of hits
	int			accuracy_hitsg;		// total number of hits
	int			accuracy_hitmg;		// total number of hits
	int			accuracy_hitbfg;		// total number of hits
	int			accuracy_hitpg;		// total number of hits
	int			accuracy_hitgl;		// total number of hits
	int			accuracy_hitlg;		// total number of hits
	int			statnum;

	int			pickupgl, pickuprl,pickupmg,pickuprg,pickuplg,pickuppg,pickupbfg,pickupsg;

	qboolean			pickgl, pickrl,pickrg,pickirg,picklg,pickpg,pickbfg,picksg,pickft;


	int			pickupmega,pickupyarmor,pickuprarmor,pickupshard,pickup5h,pickup20h,pickup40h;
	int			pickupquad,pickupsuit,pickuphaste,pickupregen,pickupinvis,pickuprflag,pickupbflag,pickupflight;

	int			pickupHealthT, pickupArmorT;
	int			damageRT, damageGT;
	int			charge;
	lerpFrame_t		legs;

} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	float			flashDlight2;
	vec3_t			flashDlightColor;
	vec3_t			flashDlightColor2;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	void			(*missileTrailFunc1)( centity_t *, const struct weaponInfo_s *wi , vec3_t axis);
	void			(*missileTrailFunc2)( centity_t *, const struct weaponInfo_s *wi , vec3_t axis);


	float			missileDlight;
	vec3_t			missileDlightColor;
	int				missileRenderfx;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
	qboolean		loopFireSound;
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
	qboolean		quadregi;
	qboolean		flightregi;
	qboolean		hasteregi;
	qboolean		speedregi;
	qboolean		battleregi;
	qboolean		invisregi;
} itemInfo_t;


typedef struct {
	int				itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL		10

typedef struct {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		40

//view flags
#define	VFL_ZOOM			0x00000010
//view flags

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	17
 
//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction

typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;
	
	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	int				logoTime, lastLogoTime;	
	int				logoTime1, lastLogoTime1;	
	int				logoTime2, lastLogoTime2;	
	int				logoTime3, lastLogoTime3;	

	int				statTime, laststatTime;	

	int				statTime1, laststatTime1;
		int				selectflash, lastselectflash;	


	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon
	qboolean	mapUpdate;
	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;
	int			weaponSelect2;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef[5];
	vec3_t		refdefViewAngles[5];		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[1024];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	int				spectatorPaintX;										// current paint x
	int				spectatorPaintX2;										// current paint x
	int				spectatorOffset;										// current offset from start
	int				spectatorPaintLen; 									// current offset from start

	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardModel[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headYaw;
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;
	int     nextOrbitTime;

	//qboolean cameraMode;		// if rendering from a loaded camera
	int		otherEntityNum;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;
	trace_t		groundTrace;
//unlagged - optimized prediction
	int			lastPredictedCommand;
	int			lastServerTime;
	playerState_t savedPmoveStates[NUM_SAVED_STATES];
	int			stateHead, stateTail;
//unlagged - optimized prediction
	int		VFlags;
	float	zoomFov;
	qboolean	PortalSky;
	vec3_t		PortalSkyOrg;
	vec3_t		DeadViewOrg;
	vec3_t		DeadViewAngles;
	int			teslaDamageNum;

	refdef_t	DeadView;
	qboolean	DeadSet;
	float	DeadfovSubX;

	refdef_t	CameraFXView;

//	int			weaponSelect;
//	int			holdableSelectTime;
} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	charsetShader;
	qhandle_t	ZcharsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;
	qhandle_t	tracerTrailzShader;
	qhandle_t	rocketTrailzShader;

	qhandle_t	rocketCoronaShader;
	qhandle_t	railCoronaShader;

	qhandle_t	colorflare;

	qhandle_t	smallFire1;
	qhandle_t	smallFire2;
	qhandle_t	smallFire3;
	qhandle_t	smallFire4;
	qhandle_t	smallFire5;
	qhandle_t	smallFire6;
	qhandle_t	smallFire7;
	qhandle_t	smallFire8;
	qhandle_t	smallFire9;
	qhandle_t	smallFire10;
	qhandle_t	smallFire11;
	qhandle_t	smallFire12;
	qhandle_t	smallFire13;
	qhandle_t	smallFire14;
	qhandle_t	smallFire15;
	qhandle_t	smallFire16;
	qhandle_t	smallFire17;

	qhandle_t	smallrFire1;
	qhandle_t	smallrFire2;
	qhandle_t	smallrFire3;
	qhandle_t	smallrFire4;
	qhandle_t	smallrFire5;
	qhandle_t	smallrFire6;
	qhandle_t	smallrFire7;
	qhandle_t	smallrFire8;
	qhandle_t	smallrFire9;
	qhandle_t	smallrFire10;
	qhandle_t	smallrFire11;
	qhandle_t	smallrFire12;
	qhandle_t	smallrFire13;
	qhandle_t	smallrFire14;
	qhandle_t	smallrFire15;
	qhandle_t	smallrFire16;
	qhandle_t	smallrFire17;

	qhandle_t	smallbFire1;
	qhandle_t	smallbFire2;
	qhandle_t	smallbFire3;
	qhandle_t	smallbFire4;
	qhandle_t	smallbFire5;
	qhandle_t	smallbFire6;
	qhandle_t	smallbFire7;
	qhandle_t	smallbFire8;
	qhandle_t	smallbFire9;
	qhandle_t	smallbFire10;
	qhandle_t	smallbFire11;
	qhandle_t	smallbFire12;
	qhandle_t	smallbFire13;
	qhandle_t	smallbFire14;
	qhandle_t	smallbFire15;
	qhandle_t	smallbFire16;
	qhandle_t	smallbFire17;

	qhandle_t	weaponZselect;

	qhandle_t	PlasmaRingBShader;
	qhandle_t	PlasmaRingRShader;
	qhandle_t	PlasmaRingGShader;

	qhandle_t	plasmaExplosionrShader;
	qhandle_t	plasmaExplosiongShader;
	qhandle_t	energyMarkrShader;
	qhandle_t	energyMarkgShader;

//	qhandle_t	redCubeModel;
//	qhandle_t	blueCubeModel;
//	qhandle_t	redCubeIcon;
//	qhandle_t	blueCubeIcon;
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
//	qhandle_t	neutralFlagModel;
	//NT - more flag shaders for return-the-flag
	qhandle_t	redFlagShader[4];
	qhandle_t	blueFlagShader[4];
	qhandle_t	flagShader[4];
	qhandle_t   redBlueFlagShader;

	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	qhandle_t	zoomtopModel;
	qhandle_t	zoombotModel;

	qhandle_t	redFlagFlapSkin;
	qhandle_t	blueFlagFlapSkin;
	qhandle_t	neutralFlagFlapSkin;

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

#ifdef MISSIONPACK
	qhandle_t	overloadBaseModel;
	qhandle_t	overloadTargetModel;
	qhandle_t	overloadLightsModel;
	qhandle_t	overloadEnergyModel;

	qhandle_t	harvesterModel;
	qhandle_t	harvesterRedSkin;
	qhandle_t	harvesterBlueSkin;
	qhandle_t	harvesterNeutralModel;
#endif

	qhandle_t	armorModel;
	qhandle_t	armorIcon;

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

//	qhandle_t	smoke2;

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

	qhandle_t	railRings1Shader;
	qhandle_t	railRings2Shader;
	qhandle_t	railRings3Shader;

//	qhandle_t	tipShader;
//	qhandle_t	tip1Shader;

	qhandle_t	railCoreShader;
	qhandle_t	railCoreglowShader;
	qhandle_t	irailCorebShader;
	qhandle_t	irailCorerShader;
//	qhandle_t	rocketsparkShader;
//	qhandle_t	bfgsparkShader;
	qhandle_t	lightningShader;
	qhandle_t	lightningEXPShader;

	qhandle_t	friendShader;

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;
//	qhandle_t	sparkShader;

//	qhandle_t	rlfireflShader;
	qhandle_t	railexpringShader;
	qhandle_t	railexpring2Shader;

	qhandle_t	PGPartShader;

	qhandle_t	selectShader;
	qhandle_t	viewBloodShader;
	qhandle_t	tracerShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	hastezTrailShader;

//	qhandle_t	bloodzTrailShader;

	qhandle_t	zoomringShader;

//	qhandle_t	plasmazTrailShader;
	qhandle_t	smokeTrailShader;
//	qhandle_t	wsmokeTrailShader;
	qhandle_t	smokePuffShader;

	qhandle_t	PLSmokeShader;
	qhandle_t	GREYSmokeShader;
	qhandle_t	GREYSmokeRingShader;
	qhandle_t	LIGHTSmokeRingShader;
	qhandle_t	BLACKSmokeShader;
//	qhandle_t	BLSmokeShader;
//	qhandle_t	smokePuffShader1;
//	qhandle_t	smokePuffRageProShader;
	qhandle_t	shotgunSmokePuffShader;
	qhandle_t	waterBubbleShader;
	qhandle_t	bloodTrailShader;

	qhandle_t	zbloodSpray1Shader;
	qhandle_t	zbloodSpray2Shader;

	qhandle_t	zbloodSplat1Shader;
	qhandle_t	zbloodSplat2Shader;
	qhandle_t	zbloodSplat3Shader;

	qhandle_t	FireParticleShader;
	qhandle_t	FireSparksShader;
#ifdef MISSIONPACK
	qhandle_t	nailPuffShader;
	qhandle_t	blueProxMine;
#endif

	qhandle_t	numberShaders[11];
	qhandle_t	digitalnumShaders[11];
	qhandle_t	deaddigitalShader;
	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
	qhandle_t	splashrippleShader;
	qhandle_t	bloodMarkShader;

//	qhandle_t	qbloodMarkShader;

	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;

//	qhandle_t	bulletMarkgShader;
//	qhandle_t	burnMarkgShader;
//	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;
	qhandle_t	energy1MarkShader;
	qhandle_t	energy21MarkShader;

	// powerup shaders
	qhandle_t	quadexpShader;
	qhandle_t	rlfireexpShader;
	qhandle_t	quadRexpShader;
	qhandle_t	quadShader;
	qhandle_t	bluetShader;
	qhandle_t	redtShader;
	qhandle_t	quadGibs;

	qhandle_t	rpQuadShader;
	qhandle_t	pQuadShader;

	qhandle_t	redQuadShader;
	qhandle_t	quadWeaponShader;
	qhandle_t	redquadWeaponShader;
	qhandle_t	invisShader;
	qhandle_t	regenShader;
	qhandle_t	battleSuitShader;
	qhandle_t	hasteSuitShader;
	qhandle_t	battleWeaponShader;
//	qhandle_t	hastePuffShader;
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;

	// damage shaders
	qhandle_t	lghitShader;
	qhandle_t	lghitweapShader;

	qhandle_t	EArmorShader;

	// weapon effect models
//	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	dish2FlashModel;
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
//	qhandle_t	railExplosionShader;
	qhandle_t	rgExplosionShader;
	qhandle_t	railExplosion2Shader;

	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	bfgExplosionShader;
	qhandle_t	bloodExplosionShader;
	qhandle_t	chargeMeterShader;
	qhandle_t	chargeMetergShader;
	qhandle_t	simpleShockWaveShader;

	qhandle_t	chargefxMeterShader;

	qhandle_t	rocketShockWave;
	qhandle_t	rgringShockWave;
	qhandle_t	rocketShockWaveShader;
	qhandle_t	rocketShockRing;
	qhandle_t	rocketRingShader;


	// special effects models
	qhandle_t	teleportEffectModel;
	qhandle_t	teleportEffectShader;
	qhandle_t	teleportREffectShader;
	qhandle_t	teleportBEffectShader;

	qhandle_t	PlasmaWave;
	qhandle_t	quadSphereModel;
	qhandle_t	kamikazeEffectModel;
	qhandle_t	kamikazeShockWave;
	qhandle_t	kamikazeHeadModel;
	qhandle_t	kamikazeHeadTrail;
	qhandle_t	dustPuffShader;

	qhandle_t	teampwbModel;
	qhandle_t	teampwrModel;

#ifdef MISSIONPACK
	qhandle_t	guardPowerupModel;
	qhandle_t	scoutPowerupModel;
	qhandle_t	doublerPowerupModel;
	qhandle_t	ammoRegenPowerupModel;
	qhandle_t	invulnerabilityImpactModel;
	qhandle_t	invulnerabilityJuicedModel;
	qhandle_t	medkitUsageModel;

	qhandle_t	heartShader;
#endif
	qhandle_t	invulnerabilityPowerupModel;

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;
	qhandle_t	m3dmedalImpressive;
	qhandle_t	m3dmedalExcellent;
	qhandle_t	m3dmedalGauntlet;
	qhandle_t	m3dmedalDefend;
	qhandle_t	m3dmedalAssist;
	qhandle_t	m3dmedalCapture;
	qhandle_t	flamethrowerFireStream;
	qhandle_t	flamethrowerBlueStream;
	// sounds

	//stopped here
	sfxHandle_t	flameBlowSound;
	sfxHandle_t	flameStreamSound;
	sfxHandle_t	flameSound;
	sfxHandle_t	quadSound;
	sfxHandle_t	armorHitSound;

	sfxHandle_t	waterHitSound;
	sfxHandle_t	waterHitsmallSound;
	sfxHandle_t	waterHitHugeSound;

	sfxHandle_t	spawnViewSound;


	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	sfxHandle_t	sfx_ric4;
	sfxHandle_t	sfx_ric5;
	sfxHandle_t	sfx_ric6;

	sfxHandle_t	sfx_flesh1;
	sfxHandle_t	sfx_flesh2;
	sfxHandle_t	sfx_flesh3;
	sfxHandle_t	sfx_flesh4;
	sfxHandle_t	sfx_flesh5;

	sfxHandle_t	sfx_railg;
	sfxHandle_t	sfx_rockexp;
	sfxHandle_t	sfx_grenadeExp;
	sfxHandle_t	sfx_plasmaexp;
	sfxHandle_t	sfx_plasmaexp2;
	sfxHandle_t kamikazeExplodeSound;
	sfxHandle_t kamikazeImplodeSound;
	sfxHandle_t kamikazeFarSound;

#ifdef MISSIONPACK

	sfxHandle_t	sfx_proxexp;
	sfxHandle_t	sfx_nghit;
	sfxHandle_t	sfx_nghitflesh;
	sfxHandle_t	sfx_nghitmetal;
	sfxHandle_t	sfx_chghit;
	sfxHandle_t	sfx_chghitflesh;
	sfxHandle_t	sfx_chghitmetal;


	sfxHandle_t useInvulnerabilitySound;
	sfxHandle_t invulnerabilityImpactSound1;
	sfxHandle_t invulnerabilityImpactSound2;
	sfxHandle_t invulnerabilityImpactSound3;
	sfxHandle_t invulnerabilityJuicedSound;
	sfxHandle_t obeliskHitSound1;
	sfxHandle_t obeliskHitSound2;
	sfxHandle_t obeliskHitSound3;
	sfxHandle_t	obeliskRespawnSound;
	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;
	sfxHandle_t	youSuckSound;
#endif
	sfxHandle_t	gibSound;
	sfxHandle_t	gibSound2;
	sfxHandle_t	gibSound3;
	sfxHandle_t	gibSound4;
	sfxHandle_t	gibSound5;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;

	sfxHandle_t	brassBounce1Sound;
	sfxHandle_t	brassBounce2Sound;
	sfxHandle_t	brassBounce3Sound;
	sfxHandle_t	brassBounce4Sound;

	sfxHandle_t	shellBounce1Sound;
	sfxHandle_t	shellBounce2Sound;

	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
	sfxHandle_t votecallSound;
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
	sfxHandle_t jumpPadSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;
	sfxHandle_t FraglimithitSound;

	sfxHandle_t hitSound;
	sfxHandle_t hitLowSound;
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
	sfxHandle_t hitTeamSound;
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;

	sfxHandle_t weaponHoverSound;

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t neutralFlagReturnedSound;
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t	youHaveFlagSound;
	//NT - two new CTF sounds for return-the-flag
	sfxHandle_t yourTeamHasYourFlagSound;
	sfxHandle_t youHaveYourFlagSound;
	sfxHandle_t enemyHasFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
	sfxHandle_t holyShitSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

#ifdef MISSIONPACK
	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t teamLeaderShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;
	qhandle_t flagShaders[3];
	sfxHandle_t	countPrepareTeamSound;

	sfxHandle_t ammoregenSound;
	sfxHandle_t doublerSound;
	sfxHandle_t guardSound;
	sfxHandle_t scoutSound;
#endif
	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;

	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
	sfxHandle_t	n_healthSound;
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;
	sfxHandle_t	wstbimplSound;
	sfxHandle_t	wstbimpmSound;
	sfxHandle_t	wstbimpdSound;
	sfxHandle_t	wstbactvSound;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glConfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

// motd messages limit 62 characters
	char			motd1[MAX_STRING_CHARS];
	char			motd2[MAX_STRING_CHARS];
	char			motd3[MAX_STRING_CHARS];
	char			motd4[MAX_STRING_CHARS];
	char			motd5[MAX_STRING_CHARS];
	char			motd6[MAX_STRING_CHARS];
// motd messages limit 62 characters

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				addview;
// instagib mode
	int				InstaGib;
	int				map_update;
	int				InstaWeapon;
	int				InstaPowUps;
// instagib mode
	int				g_Footsteps;

	int				maxclients;
	char			mapname[MAX_QPATH];
	char			map[MAX_QPATH];
	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];
	char			alternateFireVersion[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// orders
	int currentOrder;
	qboolean orderPending;
	int orderTime;
	int currentVoiceClient;
	int acceptOrderTime;
	int acceptTask;
	int acceptLeader;
	char acceptVoice[MAX_NAME_LENGTH];

	// media
	cgMedia_t		media;

//unlagged - client options
	// this will be set to the server's g_delagHitscan
	int				delagHitscan;
//unlagged - client options

	vec3_t		mvieworigin[MAX_CLIENTS];
	int			mvviewnum[5];

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];
extern	localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
extern	localEntity_t	cg_activeLocalEntities;		// double linked list
extern	localEntity_t	*cg_freeLocalEntities;		// single linked list

extern	cameraFX_t	cg_cameraFX[MAX_CAMERAFX];
extern	cameraFX_t	cg_activeCameraFX;		// double linked list
extern	cameraFX_t	*cg_freeCameraFX;		// single linked list

extern	mapFX_t	cg_mapFX[MAX_MAPFX];
extern	mapFX_t	cg_activeMapFX;		// double linked list
extern	mapFX_t	*cg_freeMapFX;		// single linked list

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_forceModel;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_noVoiceChats;
extern  vmCvar_t		cg_scorePlum;
extern	vmCvar_t		rt_ringspace;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_noTaunt;
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_oldRail;
extern	vmCvar_t		cg_oldRocket;
extern	vmCvar_t		cg_QSParticles;
extern	vmCvar_t		cg_MGParticles;
extern	vmCvar_t		cg_SGParticles;
extern	vmCvar_t		cg_BFGParticles;
extern	vmCvar_t		cg_GLParticles;
extern	vmCvar_t		cg_RGParticles;
extern	vmCvar_t		cg_currentSelectedPlayer;
extern	vmCvar_t		cg_enableDust;
extern	vmCvar_t		cg_enableBreath;
extern	vmCvar_t		cg_QSLights;
#ifdef MISSIONPACK
//extern	vmCvar_t		cg_redTeamName;
//extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_currentSelectedPlayerName;
extern	vmCvar_t		cg_singlePlayer;
extern	vmCvar_t		cg_singlePlayerActive;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;
extern	vmCvar_t		cg_obeliskRespawnDelay;
#endif
//unlagged - client options
extern	vmCvar_t		cg_delag;
extern	vmCvar_t		cg_debugDelag;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_cmdTimeNudge;
extern	vmCvar_t		sv_fps;
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cl_timeNudge;
extern	vmCvar_t		cg_latentSnaps;
extern	vmCvar_t		cg_latentCmds;
extern	vmCvar_t		cg_plOut;
//unlagged - client options
//extern	vmCvar_t		MultiView;
extern	vmCvar_t		FragMessageSize;
extern	vmCvar_t		con_notifytime;
extern	vmCvar_t		WeaponSelStyle;

void CG_PredictMissleEffects( centity_t *cent );

//unlagged - cg_unlagged.c
//unlagged - cg_unlagged.c

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore();
void CG_BuildSpectatorString();

void CG_ItemPickup( int itemNum , int clientNum);
//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_Zoom_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );

void CG_DrawActiverefs( void );
//void CG_CalcVrect1 (int ClientNum1, int ClientNum2, int ClientNum3, int ClientNum4);
void CG_CalcVrect2 (int ClientNum);
void CG_CalcVrect3 (int ClientNum);
void CG_CalcVrect4 (int ClientNum);



//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string, 
				   float charWidth, float charHeight, const float *modulate );
int CH_DrawStringExt22( vec3_t x, vec3_t y, const char *string, int charWidth, int charHeight, const float *hcolor, int count, qboolean forcecolor);


int CH_DrawStringExt( int x, int y, const char *string, int charWidth, int charHeight, const float *hcolor, int count, qboolean forcecolor);
int CH_DrawStringExtTiny( int x, int y, const char *string, int charWidth, int charHeight, const float *hcolor, int count, qboolean forcecolor);
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawStringExtTimer( int x, int y, const char *string, const float *setColor, qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );


int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

//void CG_DrawActiveMulti( void );
//
// cg_draw.c, cg_newDraw.c
//

void CG_DrawChar( int x, int y, int width, int height, int ch ,vec4_t color);

extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];
void CG_ScrollMOTD( void );

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
//refdef_t PortalSky_REFDEF(void);
void CG_DrawActive( stereoFrame_t stereoView ,int view);
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer();
void CG_SelectNextPlayer();
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead();
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat();
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText();
const char *CG_GetKillerText();
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending();
const char *CG_GameTypeString();
qboolean CG_YourTeamHasFlag();
qboolean CG_OtherTeamHasFlag();
qhandle_t CG_StatusHandle(int task);

void CG_DrawCrosshair(void);

//
// cg_player.c
//
//void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
//					float speed, float *angle, qboolean *swinging );
//void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale );
void CG_DeadPlayer( centity_t *cent );
void CG_DeadPlayerHead( centity_t *cent );
void CG_DeadPlayerLegs( centity_t *cent );

void CG_DeadQPlayer( centity_t *cent );
void CG_DeadQPlayerHead( centity_t *cent );
void CG_DeadQPlayerLegs( centity_t *cent );

void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ,centity_t *cent );
void CG_AddProjectileWithPowerups( refEntity_t *ent, entityState_t *state, int team ,centity_t *cent );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );
void CG_HitArmorEvent( centity_t *cent, int armor );

//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( int view,int clientNum ,qboolean	MView);
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t outDeltaAngles );
void CG_PositionEntityOnTag2( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );

void CG_View1( playerState_t	*ps);

//
// cg_weapons.c
//
//void CG_RailTrail3( clientInfo_t *ci, vec3_t start, vec3_t end,vec3_t dir);
void CG_RailgunPrefire( centity_t *cent );

void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType,qboolean quad,qboolean player);
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum , int clientNum,qboolean quad);
void CG_ShotgunFire( entityState_t *es );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( void );	// should this be in pmove?
//float CG_MachinegunSpinAngle( centity_t *cent );
//void RailPlayer( vec3_t origin, vec3_t dir, int entityNum );
//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader, 
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary );




//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( int view );

//
// cg_camerafx
//
void	CG_InitCameraFX( void );
cameraFX_t	*CG_AllocCameraFX( void );
void	CG_AddCameraFX( void );
void	CG_RocketCorona(vec3_t org ,int duration,int radius);
void	CG_RailCorona( vec3_t org ,int duration,int radius) ;
void	CG_ParseFXFile( const char *filename);

//
// cg_mapfx
//
/*void	CG_InitMapFX( void );
mapFX_t	*CG_AllocMapFX( void );
void	CG_AddMapFX( void );*/

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p, 
				   const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi, vec3_t *axis , int team) ;
void CG_PlasmaTrail2( centity_t *cent, const weaponInfo_t *wi, vec3_t axis[3] , int team) ;
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org ,int team);

void CG_KamikazeEffect( vec3_t org );
#ifdef MISSIONPACK
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_GibPlayer( vec3_t playerOrigin, int clientNum,qhandle_t model, vec3_t gdir,int PercentOnFire);
void CG_GibPlayerHead( vec3_t playerOrigin, int clientNum,qhandle_t model, vec3_t gdir,int PercentOnFire);
void CG_GibPlayerLegs( vec3_t playerOrigin, int clientNum,qhandle_t model, vec3_t gdir,int PercentOnFire);


void CG_GibPlayerQ( vec3_t playerOrigin ,vec3_t dir,int team,int clientNum,int PercentOnFire);
void CG_GibPlayerQLegs( vec3_t playerOrigin ,vec3_t dir,int team,int clientNum,int PercentOnFire);
void CG_GibPlayerQHead( vec3_t playerOrigin ,vec3_t dir,int team,int clientNum,int PercentOnFire);



void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );
void CG_HitArmor( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite , int radius);
localEntity_t *CG_MakeExplosion2( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite , int radius,int team);

localEntity_t *CG_MakeRLExplosion( vec3_t origin, vec3_t dir, char *animStr,
								int duration, qboolean isSprite , int radius,
								qhandle_t hModel);

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );
//unlagged - early transitioning
//void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
qboolean CG_DrawZPMScoreboard( void );
qboolean  CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_LoadVoiceChats( void );
void CG_ShaderStateChanged(void);
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );
int			trap_FS_Seek( fileHandle_t f, long offset, int origin ); // fsOrigin_t

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs,
					  clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points, 
			const vec3_t projection,
			int maxPoints, vec3_t pointBuffer,
			int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );	// empty name stops music
void	trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t       trap_R_RegisterModel(const char *name);	// returns rgb axis if not found
qhandle_t       trap_R_RegisterAnimation(const char *name);
qhandle_t       trap_R_RegisterSkin(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShader(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShaderNoMip(const char *name);	// returns all white if not found
qhandle_t       trap_R_RegisterShaderLightAttenuation(const char *name);

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );
void            trap_R_AddRefLightToScene(const refLight_t * light);

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void	    trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h, 
			float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
					   float frac, const char *tagName );
int             trap_R_BuildSkeleton(refSkeleton_t * skel, qhandle_t anim, int startFrame, int endFrame, float frac, qboolean clearOrigin);
int             trap_R_BlendSkeleton(refSkeleton_t * skel, const refSkeleton_t * blend, float frac);
int             trap_R_BoneIndex(qhandle_t hModel, const char *boneName);
int				trap_R_AnimNumFrames(qhandle_t hAnim);
int				trap_R_AnimFrameRate(qhandle_t hAnim);
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glConfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );	

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );
int			trap_RealTime(qtime_t *qtime);


typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t; // bk001201 - warning: useless keyword or type name in empty declaration


int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

void trap_SnapVector( float *v );

qboolean	trap_loadCamera(const char *name);
void		trap_startCamera(int time);
qboolean	trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean	trap_GetEntityToken( char *buffer, int bufferSize );

int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount );

void	CG_ClearParticles (void);
void	CG_AddParticles (int view);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t dir);
void	CG_ParticleSparks (vec3_t org, vec3_t vel);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
void	CG_ParticleSMExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
void	CG_ParticleSmokeExplosion (qhandle_t pshader, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );
void CG_QSphereEffect( vec3_t org , vec3_t dir,int team,int duration,int weapon,qboolean quad);

void CG_RingEffect( vec3_t org , vec3_t dir,char *animStr,int duration,qboolean sprite);
void CG_RingEffect2( vec3_t org , vec3_t dir,char *animStr,int duration,qboolean sprite);
void CG_ParticleBloodCloud (centity_t *cent, vec3_t origin, vec3_t dir);
void CG_RocketExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2);
void CG_GrenadeExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2);
void CG_RocketSMExplosion( vec3_t dir, vec3_t org ,char *animStr,int duration,int radius);
void CG_ScrollStatFinal(void);
void CG_ParseStatisticsInfo(void);
void CG_Particle_Bleed (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration,int size,int endsize);
// ON FIRE!!
void CG_Particle_Fire (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration,int size,int endsize,int randorg);
void CG_Particle_FireTeam (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration,int size,int endsize,int randorg,int team);
void CG_Particle_FireSparks (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration,int size,int endsize,int randorg);
void CG_Particle_FireSmoke (qhandle_t pshader, vec3_t start, vec3_t dir, int fleshEntityNum, int duration,int size,int endsize,int randorg);
void CG_OnFireSmoke(vec3_t Origin,int startsize,int endsize,int randorg);
void CG_OnFireSparks(vec3_t Origin,int startsize,int endsize,int randorg);
void CG_OnFireBase(vec3_t Origin,int startsize,int endsize,int randorg);
// ON FIRE!!
void CG_RailEXPRingEffect( vec3_t org , vec3_t dir,int duration,clientInfo_t *ci);
void CG_RailEXPRingEffect2( vec3_t org , vec3_t dir,int duration,clientInfo_t *ci);
void CG_RgExplosion( vec3_t dir, vec3_t org ,int duration, float light, vec3_t lightColor, float light2, vec3_t lightColor2);
void CG_FireTrail( centity_t *ent ) ;
void CG_FlarePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ,char *animStr,qboolean animated);
void CG_RLSMOKE( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,char *animStr);
qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane );
void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ,qboolean animated);
void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] );
void CG_AddToTeamChat( const char *str );
void CG_Portal( centity_t *cent );
void CG_Particle_HalfBurstSparks (qhandle_t pshader, vec3_t start,vec3_t dir,int team);
void CG_Particle_Full360BurstSparks (qhandle_t pshader, vec3_t start,vec3_t dir);
void CG_ParticleDirtBulletDebris_Core (vec3_t org, vec3_t vel, int duration, 
									   float width, float height, float alpha, char *shadername);
void CG_AddBulletSplashParticles( vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale,
							   float width, float height, float alpha, char *shadername);
void CG_ParticleBubbles (qhandle_t pshader, vec3_t origin, float turb,float randsize);
void CameraFXView_REFDEF(void);
void CG_Particle_Full360BurstBlood (qhandle_t pshader, vec3_t start,vec3_t dir);

