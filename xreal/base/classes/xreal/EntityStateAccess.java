package xreal;

import javax.vecmath.Vector3f;

public interface EntityStateAccess {

	public int getEntityState_number();
	
	public int getEntityState_eType();
	
	public void setEntityState_eType(int type);
	
	public int getEntityState_eFlags();
	
	public void setEntityState_eFlags(int flags);
	
	public Trajectory getEntityState_pos();
	
	public void setEntityState_pos(Trajectory tr);
	
	public Trajectory getEntityState_apos();
	
	public void setEntityState_apos(Trajectory tr);
	
	public int getEntityState_time();

	public void setEntityState_time(int time);

	public int getEntityState_time2();

	public void setEntityState_time2(int time2);
	
	public Vector3f getEntityState_origin();
	
	public void setEntityState_origin(Vector3f origin);
	
	public void setEntityState_origin(float x, float y, float z);
	
	public Vector3f getEntityState_origin2();
	
	public void setEntityState_origin2(Vector3f origin);
	
	public void setEntityState_origin2(float x, float y, float z);
	
	public Angle3f getEntityState_angles();
	
	public void setEntityState_angles(Angle3f angles);
	
	public void setEntityState_angles(float pitch, float yaw, float roll);
	
	public Angle3f getEntityState_angles2();
	
	public void setEntityState_angles2(Angle3f angles);
	
	public void setEntityState_angles2(float pitch, float yaw, float roll);

	public int getEntityState_otherEntityNum();

	public void setEntityState_otherEntityNum(int otherEntityNum);

	public int getEntityState_otherEntityNum2();

	public void setEntityState_otherEntityNum2(int otherEntityNum2);

	public int getEntityState_groundEntityNum();

	public void setEntityState_groundEntityNum(int groundEntityNum);

	public int getEntityState_constantLight();

	public void setEntityState_constantLight(int constantLight);

	public int getEntityState_loopSound();

	public void setEntityState_loopSound(int loopSound);

	public int getEntityState_modelindex();

	public void setEntityState_modelindex(int modelindex);

	public int getEntityState_modelindex2();

	public void setEntityState_modelindex2(int modelindex2);

	public int getEntityState_clientNum();

	public void setEntityState_clientNum(int clientNum);

	public int getEntityState_frame();

	public void setEntityState_frame(int frame);

	public int getEntityState_solid();

	public void setEntityState_solid(int solid);

	public int getEntityState_event();

	public void setEntityState_event(int event);

	public int getEntityState_eventParm();

	public void setEntityState_eventParm(int eventParm);

	public int getEntityState_powerups();

	public void setEntityState_powerups(int powerups);

//	public int getEntityState_weapon();

//	public void setEntityState_weapon(int weapon);

	public int getEntityState_legsAnim();

	public void setEntityState_legsAnim(int legsAnim);

	public int getEntityState_torsoAnim();

	public void setEntityState_torsoAnim(int torsoAnim);

	public int getEntityState_generic1();

	public void setEntityState_generic1(int generic1);
}
