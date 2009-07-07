package xreal.common;

import javax.vecmath.Vector3f;

import xreal.PlayerStateAccess;
import xreal.UserCommand;

public class PlayerMove {

	// state (in / out)
	public PlayerStateAccess ps;

	// command (in)
	public UserCommand cmd;
	public int tracemask; // collide against these types of surfaces
	public int debugLevel; // if set, diagnostic output will be printed
	public int airControl; // if set, air control will be allowed
	public int fastWeaponSwitches; // if set, weapon lower and raise animations
									// will be skipped
	public boolean noFootsteps; // if the game is setup for no footsteps by the
								// server
	public boolean gauntletHit; // true if a gauntlet attack would actually hit
								// something

	public int framecount;

	// results (out)
	public int numtouch;
	// int touchents[MAXTOUCH];

	Vector3f mins, maxs; // bounding box size

	public int watertype;
	public int waterlevel;

	public float xyspeed;

	// fixed pmove
	public int fixedPmove;
	public int fixedPmoveFPS;

	// callbacks to test the world
	// these will be different functions during game and cgame
	// void (*trace) (trace_t * results, const vec3_t start, const vec3_t mins,
	// const vec3_t maxs, const vec3_t end,
	// int passEntityNum, int contentMask);
	// int (*pointcontents) (const vec3_t point, int passEntityNum);
}
