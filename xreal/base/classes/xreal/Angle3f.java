package xreal;

import javax.vecmath.*;

/**
 * A 3 element Quake2 angle that is represented by single precision floating
 * point pitch, yaw, roll coordinates.
 */
public class Angle3f extends Tuple3f {

	/**
	 * Constructs and initializes a Point3f to (0,0,0).
	 */
	public Angle3f() {
		super();
	}

	/**
	 * Constructs and initializes an Angle3f from the specified pitch/yaw/roll
	 * coordinates.
	 * 
	 * @param pitch
	 *            float
	 * @param yaw
	 *            float
	 * @param roll
	 *            float
	 */
	public Angle3f(float pitch, float yaw, float roll) {
		super(pitch, yaw, roll);
	}

	/**
	 * Create and set an Angle3f to point from Point3f p1 to Point3f p2
	 */
	public Angle3f(Point3f p1, Point3f p2) {
		super();
		convert(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	}

	/**
	 * Create an Angle3f object pointing in the same direction as the specified
	 * vector.
	 * 
	 * @param v
	 *            javax.vecmath.Vector3f
	 */
	public Angle3f(Vector3f v) {
		super();
		convert(v.x, v.y, v.z);
	}

	/**
	 * Create an Angle3f object by copying the specified Angle3f.
	 * 
	 * @param a
	 *            q2java.Angle3f
	 */
	public Angle3f(Angle3f a) {
		super(a);
	}
	
	public Angle3f(Quat4f q) {
		super();
		set(q);
	}

	/**
	 * Convert from vector to angle format.
	 */
	private void convert(float dx, float dy, float dz) {
		double forward;
		float yaw, pitch;

		if (dy == 0 && dx == 0) {
			yaw = 0;
			if (dz > 0)
				pitch = 90;
			else
				pitch = 270;
		} else {
			yaw = (int) (Math.atan2(dy, dx) * 180 / Math.PI);
			if (yaw < 0)
				yaw += 360;

			forward = Math.sqrt((dx * dx) + (dy * dy));
			pitch = (int) (Math.atan2(dz, forward) * 180 / Math.PI);
			if (pitch < 0)
				pitch += 360;
		}

		x = -pitch;
		y = yaw;
		z = 0;
	}

	/**
	 * calculate 3 unit vectors that point forward, right, and up
	 */
	public final void getVectors(Vector3f forward, Vector3f right, Vector3f up) {
		double angle;
		double sr, sp, sy, cr, cp, cy;

		// pitch
		angle = x * (Math.PI / 180.0);
		sp = Math.sin(angle);
		cp = Math.cos(angle);

		// yaw
		angle = y * (Math.PI / 180.0);
		sy = Math.sin(angle);
		cy = Math.cos(angle);

		if (forward != null) {
			forward.x = (float) (cp * cy);
			forward.y = (float) (cp * sy);
			forward.z = (float) -sp;
		}

		if ((right != null) || (up != null)) {
			
			// roll - only needed for right and up
			angle = z * (Math.PI / 180.0);
			sr = Math.sin(angle);
			cr = Math.cos(angle);

			if (right != null) {
				right.x = (float) (cr * sy - sr * sp * cy);
				right.y = (float) -(sr * sp * sy + cr * cy);
				right.z = (float) -(sr * cp);
			}

			if (up != null) {
				up.x = (float) (cr * sp * cy + sr * sy);
				up.y = (float) (cr * sp * sy - sr * cy);
				up.z = (float) (cr * cp);
			}
		}
	}

	/**
	 * Set this Angle3f to point from Point3f p1 to Point3f p2
	 */
	public void set(Point3f p1, Point3f p2) {
		convert(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	}

	/**
	 * Set this Angle3f to point in the same direction as the specified vector.
	 * 
	 * @param v
	 *            javax.vecmath.Vector3f
	 */
	public void set(Vector3f v) {
		convert(v.x, v.y, v.z);
	}
	
	//#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
	//#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )
	
	/**
	 * 
	 */
	public void set(Quat4f q) {
		Quat4f q2 = new Quat4f();
		q2.x = q.x * q.x;

		q2.x = q.x * q.x;
		q2.y = q.y * q.y;
		q2.z = q.z * q.z;
		q2.w = q.w * q.w;

		this.x = (float) ((Math.asin(-2 * (q.z * q.x - q.w * q.y))) * 180.0f / Math.PI);
		this.y = (float) ((Math.atan2(2 * (q.z * q.w + q.x * q.y), (q2.z - q2.w - q2.x + q2.y))) * 180.0f / Math.PI);
		this.z = (float) ((Math.atan2(2 * (q.w * q.x + q.z * q.y), (-q2.z - q2.w + q2.x + q2.y))) * 180.0f / Math.PI);	
	}
	
	
	/**
	 * Convert a float angle to the short format used by UserCommand and PlayerState.
	 */
	public static short toShort(float x) {
		return (short) ((int)((x)*65536/360) & 65535);
	}
	
	/**
	 * Convert a short angle to a float.
	 */
	public static float toFloat(short x) {
		return (x * (360.0f / 65536));
	}
	
	/**
	 * Returns angle normalized to the range [0 <= angle < 360]
	 */
	public static float normalize360(float angle)
	{
		return (float) ((360.0 / 65536) * ((int)(angle * (65536 / 360.0)) & 65535));
	}
	
	/**
	 * Returns angle normalized to the range [-180 < angle <= 180]
	 */
	public static float normalize180(float angle)
	{
		angle = normalize360(angle);

		if(angle > 180.0)
		{
			angle -= 360.0;
		}

		if(angle < -180.0)
		{
			angle += 360.0;
		}
		
		return angle;
	}
	
	/**
	 * Normalize all angles of this Angle3f to the range [0 <= angle < 360]
	 */
	public void normalize360()
	{
		x = normalize360(x);
		y = normalize360(y);
		z = normalize360(z);
	}
	
	/**
	 * Normalize all angles of this Angle3f to the range [-180 < angle <= 180]
	 */
	public void normalize180()
	{
		x = normalize180(x);
		y = normalize180(y);
		z = normalize180(z);
	}
}