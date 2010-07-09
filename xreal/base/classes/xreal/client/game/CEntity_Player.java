package xreal.client.game;

import javax.vecmath.Matrix3f;
import javax.vecmath.Matrix4f;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import com.bulletphysics.collision.dispatch.CollisionObject;

import xreal.Angle3f;
import xreal.CVars;
import xreal.Engine;
import xreal.client.EntityState;
import xreal.client.renderer.Polygon;
import xreal.client.renderer.Renderer;
import xreal.client.renderer.Vertex;
import xreal.common.Config;

public class CEntity_Player extends CEntity {
	
//	private int				muzzleFlashTime;
	
	public CEntity_Player(EntityState es) {
		super(es);
		
		//Engine.println("CEntity_Player(entity number = " + es.getNumber() + ")");
	}

	/**
	 * A player just came into view or teleported, so reset all animation info.
	 */
	@Override
	public void reset() {
		
		super.reset();
		
		errorTime = -99999;	// guarantee no error decay added
		extrapolated = false;
	
		//CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.legs, cent->currentState.legsAnim);
		//CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.torso, cent->currentState.torsoAnim);
	
		lerpOrigin = currentState.pos.evaluatePosition(ClientGame.getTime());
		lerpAngles = new Angle3f(currentState.apos.evaluatePosition(ClientGame.getTime()));
	
		rawOrigin = new Vector3f(lerpOrigin);
		rawAngles = new Angle3f(lerpAngles);
		
		/*	
		memset(&cent->pe.legs, 0, sizeof(cent->pe.legs));
		cent->pe.legs.yawAngle = cent->rawAngles[YAW];
		cent->pe.legs.yawing = qfalse;
		cent->pe.legs.pitchAngle = 0;
		cent->pe.legs.pitching = qfalse;
	
		memset(&cent->pe.torso, 0, sizeof(cent->pe.legs));
		cent->pe.torso.yawAngle = cent->rawAngles[YAW];
		cent->pe.torso.yawing = qfalse;
		cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
		cent->pe.torso.pitching = qfalse;
	
		if(cg_debugPosition.integer)
		{
			CG_Printf("%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle);
		}
		 */
	}
	
	@Override
	public void addToRenderer() throws Exception 
	{
		super.addToRenderer();
		
		// get the player model information
		/*
		renderfx = 0;
		if(currentState.number == cg.snap->ps.clientNum)
		{
			if(!cg.renderingThirdPerson)
			{
				renderfx = RF_THIRD_PERSON;	// only draw in mirrors
			}
			else
			{
				if(cg_cameraMode.integer)
				{
					return;
				}
			}
		}
		*/
		
		if(CVars.cg_drawPlayerCollision.getBoolean()) {
			renderCollisonShape();
		}
	}
	
	private void renderCollisonShape() throws Exception 
	{
		Polygon			poly = new Polygon();
		Vertex			verts[] = new Vertex[4];
		int             i;
		Vector3f        mins = new Vector3f();
		Vector3f        maxs = new Vector3f();
		float           extx, exty, extz;
		Point3f         corners[] = new Point3f[8];
		
		//Engine.println("CEntity_Player.renderCollisionBox()");

		mins.set(Config.playerMins);
		maxs.set(Config.playerMaxs);

		// if it's us
		if(currentState.getNumber() == ClientGame.getClientNum())
		{
			if(!ClientGame.getCamera().isRenderingThirdPerson())
				return;
			
			// use the view height
			//maxs[2] = cg.predictedPlayerState.viewheight + 6;
		}
		else
		{
			int             x, zd, zu;

			// otherwise grab the encoded bounding box
			x = (currentState.solid & 255);
			zd = ((currentState.solid >> 8) & 255);
			zu = ((currentState.solid >> 16) & 255) - 32;

			mins.x = mins.y = -x;
			maxs.x = maxs.y = x;
			mins.z = -zd;
			maxs.z = zu;
		}

		// get the extents (size)
		extx = maxs.x - mins.x;
		exty = maxs.y - mins.y;
		extz = maxs.z - mins.z;
		
		for(i = 0; i < verts.length; i++)
		{
			verts[i] = new Vertex();
		}
		
		for(i = 0; i < corners.length; i++)
		{
			corners[i] = new Point3f();
		}

		// set the polygon's texture coordinates
		verts[0].st.x = 0;
		verts[0].st.y = 0;
		verts[1].st.x = 0;
		verts[1].st.y = 1;
		verts[2].st.x = 1;
		verts[2].st.y = 1;
		verts[3].st.x = 1;
		verts[3].st.y = 0;

		// set the polygon's vertex colors

		switch (currentState.generic1)
		{
			case CollisionObject.ACTIVE_TAG:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 1;
					verts[i].color.y = 1;
					verts[i].color.z = 1;
					verts[i].color.w = 1;
				}
				break;

			case CollisionObject.ISLAND_SLEEPING:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 0;
					verts[i].color.y = 1;
					verts[i].color.z = 0;
					verts[i].color.w = 1;
				}
				break;

			case CollisionObject.WANTS_DEACTIVATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 0;
					verts[i].color.y = 1;
					verts[i].color.z = 1;
					verts[i].color.w = 1;
				}
				break;

			case CollisionObject.DISABLE_DEACTIVATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 1;
					verts[i].color.y = 0;
					verts[i].color.z = 0;
					verts[i].color.w = 1;
				}
				break;

			case CollisionObject.DISABLE_SIMULATION:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 1;
					verts[i].color.y = 1;
					verts[i].color.z = 0;
					verts[i].color.w = 1;
				}
				break;

			default:
				for(i = 0; i < 4; i++)
				{
					verts[i].color.x = 1;
					verts[i].color.y = 0;
					verts[i].color.z = 0;
					verts[i].color.w = 1;
				}
				break;
		}

		Matrix3f rotation = new Matrix3f();
		rotation.setIdentity();
		rotation.set(lerpQuat);
		//lerpAngles.get(rotation);
		
		//MatrixFromAngles(rotation, cent->lerpAngles[PITCH], cent->lerpAngles[YAW], cent->lerpAngles[ROLL]);
		//MatrixSetupTransformFromRotation(transform, rotation, cent->lerpOrigin);
		
		Matrix4f transform = new Matrix4f(rotation, lerpOrigin, 1);

		//VectorAdd(cent->lerpOrigin, maxs, corners[3]);
		corners[3] = new Point3f(maxs);

		corners[2] = new Point3f(corners[3]);
		corners[2].x -= extx;

		corners[1] = new Point3f(corners[2]);
		corners[1].y -= exty;

		corners[0] = new Point3f(corners[1]);
		corners[0].x += extx;

		for(i = 0; i < 4; i++)
		{
			corners[i + 4] = new Point3f(corners[i]);
			corners[i + 4].z -= extz;
		}

		for(i = 0; i < corners.length; i++)
		{
			transform.transform(corners[i]);
		}
		
		

		// top
		poly.hMaterial = ClientGame.getMedia().debugPlayerAABB;
		
		poly.vertices.add(verts[0]);
		poly.vertices.add(verts[1]);
		poly.vertices.add(verts[2]);
		poly.vertices.add(verts[3]);
		
		verts[0].pos = corners[0];
		verts[1].pos = corners[1];
		verts[2].pos = corners[2];
		verts[3].pos = corners[3];
		
		Renderer.addPolygonToScene(poly);

		// bottom
		verts[0].pos = corners[7];
		verts[1].pos = corners[6];
		verts[2].pos = corners[5];
		verts[3].pos = corners[4];
		
		Renderer.addPolygonToScene(poly);

		// top side
		poly.hMaterial = ClientGame.getMedia().debugPlayerAABB_twoSided;
		
		verts[0].pos = corners[3];
		verts[1].pos = corners[2];
		verts[2].pos = corners[6];
		verts[3].pos = corners[7];
		
		Renderer.addPolygonToScene(poly);

		// left side
		verts[0].pos = corners[2];
		verts[1].pos = corners[1];
		verts[2].pos = corners[5];
		verts[3].pos = corners[6];
		
		Renderer.addPolygonToScene(poly);

		// right side
		verts[0].pos = corners[0];
		verts[1].pos = corners[3];
		verts[2].pos = corners[7];
		verts[3].pos = corners[4];
		
		Renderer.addPolygonToScene(poly);

		// bottom side
		verts[0].pos = corners[1];
		verts[1].pos = corners[0];
		verts[2].pos = corners[4];
		verts[3].pos = corners[5];
		
		Renderer.addPolygonToScene(poly);
	}
}
