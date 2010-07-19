package xreal;


/**
 * CVARS (console variables)
 * 
 * Many variables can be used for cheating purposes, so when cheats is zero,
 * force all unspecified variables to their default values.
 * 
 * NOTE: The Cvar flags must be the same as in q_shared.h for cvar_t.
 * 
 * @author Robert Beckebans
 * 
 */
public class CVar {

	/**
	 * Set to cause it to be saved to xreal.cfg or xreal_server.cfg. Used for
	 * system variables, not for player specific configurations.
	 */
	public static final int ARCHIVE = 1;

	/**
	 * Sent to server on connect or change.
	 */
	public static final int USERINFO = 2;

	/**
	 * Sent in response to front end requests.
	 */
	public static final int SERVERINFO = 4;

	/**
	 * These cvars will be duplicated on all clients.
	 */
	public static final int SYSTEMINFO = 8;

	/**
	 * Don't allow change from console at all, but can be set from the command
	 * line.
	 */
	public static final int INIT = 16;

	/**
	 * Will only change when C code next does. A Cvar_Get(), so it can't be
	 * changed without proper initialization. Modified will be set, even though
	 * the value hasn't changed yet.
	 */
	public static final int LATCH = 32;

	/**
	 * Display only, cannot be set by user at all.
	 */
	public static final int ROM = 64;

	/**
	 * Created by a set command. NOTE: Do not use this in the game logic.
	 */
	// private static final int USER_CREATED = 128;
	/**
	 * Can be set even when cheats are disabled, but is not archived.
	 */
	public static final int TEMP = 256;

	/**
	 * Can not be changed if cheats are disabled.
	 */
	public static final int CHEAT = 512;

	/**
	 * Do not clear when a cvar_restart is issued.
	 */
	public static final int NORESTART = 1024;

	/**
	 * CVar was created by a server the client connected to.
	 */
	// private static final int SERVER_CREATED = 2048;
	/**
	 * Pointer to the native cvar_t object for fast cv->string access.
	 */
	private int handle;

	// private int modificationCount;
	private String name;

	// private float value;
	// private int integer;
	// private String string;

	// private static ArrayList<CVar> list = new ArrayList<CVar>();

	/*
	 * public static void updateVars() {
	 * 
	 * Engine.print("CVar.updateVars()\n");
	 * 
	 * //synchronized (list) { for(CVar cv : list) { Engine.print("updating " +
	 * cv.toString()); cv.update(); } } }
	 */

	/**
	 * Call Cvar_Get and return the pointer of the constructed cvar_t.
	 */
	private static native int register0(String name, String value, int flags);

	/**
	 * Call Cvar_Set.
	 */
	private static native void set0(String name, String value);

	private static native String getString0(int handle);

	private static native float getValue0(int handle);

	private static native int getInteger0(int handle);

	/**
	 * Same as trap_Cvar_Register
	 * 
	 * @param name
	 * @param value
	 * @param flags
	 *            CVar.Archive and so on
	 */
	public CVar(String name, String value, int flags) {
		this.name = name;

		handle = register0(name, value, flags);

		// this.string = getString0(handle);
		// this.value = Float.valueOf(this.string);
		// this.integer = Integer.valueOf(this.string);

		// list.add(this);
	}

	/*
	 * public synchronized void update() { string = getString0(handle); value =
	 * Float.valueOf(string); integer = Integer.valueOf(string); }
	 */

	public synchronized void set(String value) {
		set0(name, value);
		// update();
	}

	public synchronized float getValue() {
		// return value;
		return getValue0(handle);
	}

	public synchronized int getInteger() {
		// return integer;
		return getInteger0(handle);
	}
	
	public synchronized boolean getBoolean() {
		return (getInteger0(handle) > 0);
	}

	public synchronized String getString() {
		// return string;
		return getString0(handle);
	}

	public String toString() {
		return "CVar '" + name + "' = '" + getString() + "'";
	}
}
