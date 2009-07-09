package xreal;

import javax.vecmath.Vector3f;

public interface EntityStateAccess {

	public int getEntityState_number();
	
	public int getEntityState_eType();
	
	public void setEntityState_eType(int eType);
	
	public Vector3f getEntityState_origin();
	
	public void setEntityState_origin(Vector3f origin);
	
	public void setEntityState_origin(float x, float y, float z);
}
