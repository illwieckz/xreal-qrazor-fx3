package xreal;

public abstract class ContentFlags {
	// This file must be identical in the quake and utils directories

	// contents flags are seperate bits
	// a given brush can contribute multiple content bits

	// these definitions also need to be in q_shared.h!

	public static final int	SOLID = 1;	// an eye is never valid in a solid
	public static final int	LAVA = 8;
	public static final int	SLIME = 16;
	public static final int	WATER = 32;
	public static final int	FOG = 64;

	public static final int NOTTEAM1 = 0x0080;
	public static final int NOTTEAM2 = 0x0100;
	public static final int NOBOTCLIP = 0x0200;

	public static final int SHOOTABLE = 0x0400;	// Tr3B: used for game entities that can be destroyed like railgun spheres

	public static final int	AREAPORTAL = 0x8000;

	public static final int	PLAYERCLIP = 0x10000;
	public static final int	MONSTERCLIP = 0x20000;

	//bot specific contents types
	public static final int	TELEPORTER = 0x40000;
	public static final int	JUMPPAD = 0x80000;
	public static final int CLUSTERPORTAL = 0x100000;
	public static final int DONOTENTER = 0x200000;
	public static final int BOTCLIP = 0x400000;
	public static final int MOVER = 0x800000;

	public static final int	ORIGIN = 0x1000000;	// removed before bsping an entity

	public static final int	BODY = 0x2000000;	// should never be on a brush, only in game
	public static final int	CORPSE = 0x4000000;
	public static final int	DETAIL = 0x8000000;	// brushes not used for the bsp
	public static final int	STRUCTURAL = 0x10000000;	// brushes used for the bsp
	public static final int	TRANSLUCENT = 0x20000000;	// don't consume surface fragments inside
	public static final int	TRIGGER = 0x40000000;
	public static final int	NODROP = 0x80000000;	// don't leave bodies or items (death fog, lava)

	// TA: custominfoparms below
	public static final int	NOALIENBUILD = 0x1000;
	public static final int	NOHUMANBUILD = 0x2000;
	public static final int	NOBUILD = 0x4000;
}
