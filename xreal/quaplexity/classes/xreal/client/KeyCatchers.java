package xreal.client;

/**
 * in order from highest priority to lowest if none of the catchers are active,
 * bound key strings will be executed
 * 
 * @author Robert Beckebans
 */
public abstract class KeyCatchers {
	public static final int CONSOLE = 0x0001;
	public static final int UI = 0x0002;
	public static final int MESSAGE = 0x0004;
	public static final int CGAME = 0x0008;
}
