package xreal.client.game;

import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.client.PlayerState;
import xreal.client.renderer.Camera;
import xreal.client.ui.UserInterface;
import xreal.common.PlayerMovementType;

public class ClientCamera extends Camera {
	
	private static final int	WAVE_AMPLITUDE	= 1;
	private static final float	WAVE_FREQUENCY	= 0.4f;
	
	public boolean calcViewValues(PlayerState ps)
	{
		time = ClientGame.getTime();

		// calculate size of 3D view
		calcVrect(ps);
		
	/*
		if (cg.cameraMode) {
			vec3_t origin, angles;
			if (trap_getCameraInfo(cg.time, &origin, &angles)) {
				VectorCopy(origin, cg.refdef.vieworg);
				angles[ROLL] = 0;
				VectorCopy(angles, cg.refdefViewAngles);
				AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
				return CG_CalcFov();
			} else {
				cg.cameraMode = qfalse;
			}
		}
	*/

	//#if defined(USE_JAVA)
		//{
		position = new Vector3f(ps.origin);
		
		//VectorCopy(ps->origin, cg.refdef.vieworg);
		//	VectorCopy(ps->viewangles, cg.refdefViewAngles);
		//	AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);
		
			ps.viewAngles.getQuaternion(quat);

			return calcFov(ps);
		//}
	//#endif


		/*
		// intermission view
		if(ps->pm_type == PM_INTERMISSION)
		{
			VectorCopy(ps->origin, cg.refdef.vieworg);
			VectorCopy(ps->viewangles, cg.refdefViewAngles);
			AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

			return CG_CalcFov();
		}

		cg.bobcycle = (ps->bobCycle & 128) >> 7;
		cg.bobfracsin = fabs(sin((ps->bobCycle & 127) / 127.0 * M_PI));
		cg.xyspeed = sqrt(ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1]);

		VectorCopy(ps->origin, cg.refdef.vieworg);

		//if(BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLCLIMBER))
		{
			CG_smoothWWTransitions(ps, ps->viewangles, cg.refdefViewAngles);
		}
//		else if(BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLJUMPER))
//		{
//			CG_smoothWJTransitions(ps, ps->viewangles, cg.refdefViewAngles);
//		}
//		else
//		{
//			VectorCopy(ps->viewangles, cg.refdefViewAngles);
//		}

		// clumsy logic, but it needs to be this way round because the CS propogation
		// delay screws things up otherwise
	#if 0
		if(!BG_ClassHasAbility(ps->stats[STAT_PCLASS], SCA_WALLJUMPER))
		{
			if(!(ps->stats[STAT_STATE] & SS_WALLCLIMBING))
				VectorSet(cg.lastNormal, 0.0f, 0.0f, 1.0f);
		}
	#endif

		if(cg_cameraOrbit.integer)
		{
			if(cg.time > cg.nextOrbitTime)
			{
				cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
				cg_thirdPersonAngle.value += cg_cameraOrbit.value;
			}
		}
		// add error decay
		if(cg_errorDecay.value > 0)
		{
			int             t;
			float           f;

			t = cg.time - cg.predictedErrorTime;
			f = (cg_errorDecay.value - t) / cg_errorDecay.value;

			if(f > 0 && f < 1)
			{
				VectorMA(cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg);
			}
			else
			{
				cg.predictedErrorTime = 0;
			}
		}

		if(cg.renderingThirdPerson)
		{
			// back away from character
			CG_OffsetThirdPersonView();
		}
		else
		{
			// offset for local bobbing and kicks
			CG_OffsetFirstPersonView();
		}

		// position eye reletive to origin
		AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

		if(cg.hyperspace)
		{
			cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
		}
		*/

		// field of view
		//return calcFov(ps);
	}
	
	
	/**
	 * Sets the coordinates of the rendered window.
	 */
	private void calcVrect(PlayerState ps)
	{
		int             size;

		// the intermission should allways be full screen
		if(ps.pm_type == PlayerMovementType.INTERMISSION)
		{
			size = 100;
		}
		else
		{
			// bound normal viewsize
			if(CVars.cg_viewsize.getInteger() < 30)
			{
				CVars.cg_viewsize.set("30");
				size = 30;
			}
			else if(CVars.cg_viewsize.getInteger() > 100)
			{
				CVars.cg_viewsize.set("100");
				size = 100;
			}
			else
			{
				size = CVars.cg_viewsize.getInteger();
			}
		}

		width = UserInterface.getVidWidth() * size / 100;
		width &= ~1;

		height = UserInterface.getVidHeight() * size / 100;
		height &= ~1;

		x = (UserInterface.getVidWidth() - width) / 2;
		y = (UserInterface.getVidHeight() - height) / 2;
	}
	

	

	/**
	 * Fixed fov at intermissions, otherwise account for fov variable and zooms.
	 * 
	 * @return
	 */
	private boolean calcFov(PlayerState ps)
	{
		float           x;
		float           phase;
		float           v;
		int             contents;
		float           fov_x, fov_y;
		float           zoomFov;
		float           f;
		boolean         inwater;

		if(ps.pm_type == PlayerMovementType.INTERMISSION)
		{
			// if in intermission, use a fixed value
			fov_x = 90;
		}
		else if(ps.pm_type == PlayerMovementType.SPECTATOR)
		{
			// if a spectator, use a fixed value
			fov_x = 90;
		}
		else
		{
			/*
			// user selectable
			if(cgs.dmflags & DF_FIXED_FOV)
			{
				// dmflag to prevent wide fov for all clients
				fov_x = 90;
			}
			else
			*/
			{
				fov_x = CVars.cg_fov.getValue();
				if(fov_x < 1)
				{
					fov_x = 1;
				}
				else if(fov_x > 160)
				{
					fov_x = 160;
				}
			}

			/*
			// account for zooms
			zoomFov = cg_zoomFov.value;
			if(zoomFov < 1)
			{
				zoomFov = 1;
			}
			else if(zoomFov > 160)
			{
				zoomFov = 160;
			}

			if(cg.zoomed)
			{
				f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;
				if(f > 1.0)
				{
					fov_x = zoomFov;
				}
				else
				{
					fov_x = fov_x + f * (zoomFov - fov_x);
				}
			}
			else
			{
				f = (cg.time - cg.zoomTime) / (float)ZOOM_TIME;
				if(f > 1.0)
				{
					fov_x = fov_x;
				}
				else
				{
					fov_x = zoomFov + f * (fov_x - zoomFov);
				}
			}
			*/
		}

		x = (float) (width / Math.tan(fov_x / 360 * Math.PI));
		fov_y = (float) Math.atan2(height, x);
		fov_y = (float) (fov_y * 360 / Math.PI);

		/*
		// warp if underwater
		contents = CG_PointContents(cg.refdef.vieworg, -1);

		if(contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
		{
			phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
			v = WAVE_AMPLITUDE * sin(phase);
			fov_x += v;
			fov_y -= v;
			inwater = qtrue;
		}
		else
		*/
		{
			inwater = false;
		}


		// set it
		fovX = fov_x;
		fovY = fov_y;

		/*
		if(!cg.zoomed)
		{
			cg.zoomSensitivity = 1;
		}
		else
		{
			cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
		}
		*/

		return inwater;
	}
}
