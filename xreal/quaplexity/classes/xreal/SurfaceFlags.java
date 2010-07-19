package xreal;

public abstract class SurfaceFlags {
	
	/**
	 * never give falling damage
	 */
	public static final int	NODAMAGE = 0x1;
	
	/**
	 * effects game physics
	 */
	public static final int	SLICK = 0x2;
	
	/**
	 * lighting from environment map
	 */
	public static final int	SKY = 0x4;
	public static final int	LADDER = 0x8;
	
	/**
	 * don't make missile explosions
	 */
	public static final int	NOIMPACT = 0x10;
	
	/**
	 * don't leave missile marks
	 */
	public static final int	NOMARKS = 0x20;
	
	/**
	 * make flesh sounds and effects
	 */
	public static final int	FLESH = 0x40;
	
	/**
	 * don't generate a drawsurface at all
	 */
	public static final int	NODRAW = 0x80;
	
	/**
	 * make a primary bsp splitter
	 */
	public static final int	HINT = 0x100;
	
	/**
	 * completely ignore, allowing non-closed brushes
	 */
	public static final int	SKIP = 0x200;
	
	/**
	 * surface doesn't need a lightmap
	 */
	public static final int	NOLIGHTMAP = 0x400;
	
	/**
	 * generate lighting info at vertexes
	 */
	public static final int	POINTLIGHT = 0x800;
	
	/**
	 * clanking footsteps
	 */
	public static final int	METALSTEPS = 0x1000;
	
	/**
	 * no footstep sounds
	 */
	public static final int	NOSTEPS = 0x2000;
	
	/**
	 * don't collide against curves with this set
	 */
	public static final int	NONSOLID = 0x4000;
	
	/**
	 * act as a light filter during xmap -light
	 */
	public static final int	LIGHTFILTER = 0x8000;
	
	/**
	 * do per-pixel light shadow casting in xmap
	 */
	public static final int	ALPHASHADOW = 0x10000;
	
	/**
	 *  don't draw but use for per polygon collision detection
	 */
	public static final int	COLLISION = 0x20000;
	
	/**
	 * leave a dust trail when walking on this surface
	 */
	public static final int DUST = 0x40000;
	
	
	// TA: custominfoparms below
	
	/**
	 * disallow alien building
	 */
	public static final int	NOALIENBUILD = 0x80000;
	public static final int	NOHUMANBUILD = 0x100000;	// disallow alien building
	public static final int	NOBUILD	= 0x200000;	// disallow alien building
	
	
	// Tr3B: add new parms below so the Tremulous stuff doesn't break
	
	/**
	 * allows player to walk on walls
	 */
	public static final int WALLWALK = 0x400000;
}
