package xreal;

public class Engine {
	
	/**
	 * Absolute limit. Don't change this here unless you increase it in q_shared.h
	 */
	public static final int MAX_CLIENTS = 64; // absolute limit
	
	/**
	 * Absolute limit. Don't change this here unless you increase it in q_shared.h
	 */
	public static final int MAX_LOCATIONS = 64;

	/**
	 * Absolute limit. Don't change this here unless you increase it in q_shared.h
	 * The Engine won't send more bits for entityState_t::index
	 */
	private static final int GENTITYNUM_BITS = 11;
	public static final int MAX_GENTITIES = (1 << GENTITYNUM_BITS);

	// entitynums are communicated with GENTITY_BITS, so any reserved
	// values that are going to be communcated over the net need to
	// also be in this range
	public static final int ENTITYNUM_NONE = (MAX_GENTITIES - 1);
	public static final int ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
	public static final int ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);

	// Tr3B: if you increase GMODELNUM_BITS then:
	// increase MAX_CONFIGSTRINGS to 2048 and double MAX_MSGLEN
	public static final int GMODELNUM_BITS = 8; // don't need to send any more
	public static final int MAX_MODELS = (1 << GMODELNUM_BITS); // references
																// entityState_t::modelindex

	public static final int MAX_SOUNDS = 256; // so they cannot be blindly
												// increased
	public static final int MAX_EFFECTS = 256;

	public static final int MAX_CONFIGSTRINGS = 1024;
	
	public synchronized native static void print(String s);

	public synchronized static void println(String s) {
		print(s + "\n");
	}

	public synchronized native static void error(String s);

	public synchronized native static int milliseconds();

	public synchronized native static int consoleArgc();

	public synchronized native static String consoleArgv(int n);

	public synchronized native static String consoleArgs();

	// public native static void sendConsoleCommand(int exec_when, String text);
	// void trap_DropClient(int clientNum, const char *reason);
	// void trap_SendServerCommand(int clientNum, const char *text);
	public synchronized native static String getConfigstring(int num);

	public synchronized native static void setConfigstring(int num,
			String string);

	public synchronized native static String getUserinfo(int num);

	public synchronized native static String setUserinfo(int num, String buffer);

	public synchronized native static String getServerinfo();
}
