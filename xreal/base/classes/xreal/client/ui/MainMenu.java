package xreal.client.ui;

import java.io.File;
import java.io.IOException;

import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.Color;
import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Camera;
import xreal.client.renderer.Font;
import xreal.client.renderer.RefEntity;
import xreal.client.renderer.Renderer;
import xreal.client.ui.border.Border;
import xreal.client.ui.border.LineBorder;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.menu.MenuFrame;

public class MainMenu extends MenuFrame {

	private static final int MAIN_MENU_VERTICAL_SPACING = 34;
	
	Button singlePlayer;
	
	private int podiumModel;
	
	MainMenu() 
	{
		super();
		
		fullscreen = true;
		wrapAround = true;
		showlogo = true;
		
		podiumModel = Renderer.registerModel("models/meshes/ppodium.md5mesh", true);
		
		/*
		int y = 134;
		singlePlayer = new Button("SINGLEPLAYER", 27, Font.DROPSHADOW) {
			@Override
			public void keyPressed(KeyEvent e) {
				Engine.println("clicked single player button");
				
				Client.startLocalSound(soundMove, SoundChannel.LOCAL_SOUND);
				//super.keyPressed(e);
			}
		};
		singlePlayer.setXCenter(UserInterface.SCREEN_WIDTH / 2);
		singlePlayer.setYCenter(y);
		//singlePlayer.setCenter(0, 0);
		singlePlayer.setBorder(new LineBorder(Color.Red));
		
		children.add(singlePlayer);
		*/

		/*
		y = 134;
		s_main.singleplayer.generic.type = MTYPE_PTEXT;
		s_main.singleplayer.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.singleplayer.generic.x = 320;
		s_main.singleplayer.generic.y = y;
		s_main.singleplayer.generic.id = ID_SINGLEPLAYER;
		s_main.singleplayer.generic.callback = Main_MenuEvent;
		s_main.singleplayer.string = "SINGLEPLAYER";
		s_main.singleplayer.color = color_white;
		s_main.singleplayer.style = style;

		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.multiplayer.generic.type = MTYPE_PTEXT;
		s_main.multiplayer.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.multiplayer.generic.x = 320;
		s_main.multiplayer.generic.y = y;
		s_main.multiplayer.generic.id = ID_MULTIPLAYER;
		s_main.multiplayer.generic.callback = Main_MenuEvent;
		s_main.multiplayer.string = "MULTIPLAYER";
		s_main.multiplayer.color = color_white;
		s_main.multiplayer.style = style;

		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.setup.generic.type = MTYPE_PTEXT;
		s_main.setup.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.setup.generic.x = 320;
		s_main.setup.generic.y = y;
		s_main.setup.generic.id = ID_SETUP;
		s_main.setup.generic.callback = Main_MenuEvent;
		s_main.setup.string = "SETUP";
		s_main.setup.color = color_white;
		s_main.setup.style = style;

		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.demos.generic.type = MTYPE_PTEXT;
		s_main.demos.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.demos.generic.x = 320;
		s_main.demos.generic.y = y;
		s_main.demos.generic.id = ID_DEMOS;
		s_main.demos.generic.callback = Main_MenuEvent;
		s_main.demos.string = "DEMOS";
		s_main.demos.color = color_white;
		s_main.demos.style = style;
		*/

	/*	y += MAIN_MENU_VERTICAL_SPACING;
		s_main.cinematics.generic.type = MTYPE_PTEXT;
		s_main.cinematics.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.cinematics.generic.x = 320;
		s_main.cinematics.generic.y = y;
		s_main.cinematics.generic.id = ID_CINEMATICS;
		s_main.cinematics.generic.callback = Main_MenuEvent;
		s_main.cinematics.string = "CINEMATICS";
		s_main.cinematics.color = color_white;
		s_main.cinematics.style = style;
	*/
	/*	if(UI_TeamArenaExists())
		{
			  Tr3B - don't need it
			   teamArena = qtrue;
			   y += MAIN_MENU_VERTICAL_SPACING;
			   s_main.teamArena.generic.type            = MTYPE_PTEXT;
			   s_main.teamArena.generic.flags           = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
			   s_main.teamArena.generic.x               = 320;
			   s_main.teamArena.generic.y               = y;
			   s_main.teamArena.generic.id              = ID_TEAMARENA;
			   s_main.teamArena.generic.callback        = Main_MenuEvent;
			   s_main.teamArena.string                  = "TEAM ARENA";
			   s_main.teamArena.color                   = color_white;
			   s_main.teamArena.style                   = style;

		}
	*/
		
		/*
		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.mods.generic.type = MTYPE_PTEXT;
		s_main.mods.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.mods.generic.x = 320;
		s_main.mods.generic.y = y;
		s_main.mods.generic.id = ID_MODS;
		s_main.mods.generic.callback = Main_MenuEvent;
		s_main.mods.string = "MODS";
		s_main.mods.color = color_white;
		s_main.mods.style = style;

		y += MAIN_MENU_VERTICAL_SPACING;
		s_main.exit.generic.type = MTYPE_PTEXT;
		s_main.exit.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
		s_main.exit.generic.x = 320;
		s_main.exit.generic.y = y;
		s_main.exit.generic.id = ID_EXIT;
		s_main.exit.generic.callback = Main_MenuEvent;
		s_main.exit.string = "EXIT";
		s_main.exit.color = color_white;
		s_main.exit.style = style;

		Menu_AddItem(&s_main.menu, &s_main.singleplayer);
		Menu_AddItem(&s_main.menu, &s_main.multiplayer);
		Menu_AddItem(&s_main.menu, &s_main.setup);
		Menu_AddItem(&s_main.menu, &s_main.demos);
		//Menu_AddItem(&s_main.menu, &s_main.cinematics);
		//if(teamArena)
		//{
		//  Menu_AddItem(&s_main.menu, &s_main.teamArena);
		//}
		Menu_AddItem(&s_main.menu, &s_main.mods);
		Menu_AddItem(&s_main.menu, &s_main.exit);

		trap_Key_SetCatcher(KEYCATCH_UI);
		UI_PushMenu(&s_main.menu);
		*/
	}

	@Override
	public void render() {
		
		String message = "Use the console with Shift + Escape";
		
		fontVera.paintText(UserInterface.SCREEN_WIDTH / 2, UserInterface.SCREEN_HEIGHT / 2, 16, Color.White, message, 0, 0, Font.CENTER);
		
		/*
		Rectangle rect = fontVera.getTextBounds(message, 16, 0);
		rect.setCenter(UserInterface.SCREEN_WIDTH / 2, UserInterface.SCREEN_HEIGHT / 2);
		
		Border border = new LineBorder(Color.Red);
		border.paintBorder(rect.x,rect.y,rect.width,rect.height);
		*/
		
		/*
		rect = textFont.getStringBounds(message, 0.5f, 0);
		rect.x = SCREEN_WIDTH / 2;
		rect.y = SCREEN_HEIGHT / 2;
		adjustFrom640(rect);
		*/
		
		// stress test
		/*
		message = "x";
		float textWidth = fontVera.getTextWidth(message, 8, 0);
		float textHeight = fontVera.getTextHeight(message, 8, 0);
		
		int count = 0;
		for (float x = 0; x < UserInterface.SCREEN_WIDTH; x += textWidth) {
			for (float y = 0; y <UserInterface. SCREEN_HEIGHT; y += textHeight) {
				fontVera.paintText(x, y, 8, Color.White, message, 0, 0, Font.LEFT);
				count++;
			}
		}
		
		Engine.println("textWidth=" + textWidth + ", textHeight=" + textHeight + ", count=" + count);
		
		fontVera.paintText(320, 470, 12, Color.White, count + " letters drawn", 0, 0,  Font.CENTER);
		*/
		fontVera.paintText(320, 470, 12, Color.White, "XreaL(c) 2005-2009, XreaL Team - http://xreal.sourceforge.net", 0, 0,
				  Font.CENTER | Font.DROPSHADOW);
		
		//renderViewTest(320, UserInterface.SCREEN_HEIGHT / 2, 200, 300, UserInterface.getRealTime());
		
		super.render();
	}
	
	
	void renderViewTest(float x, float y, float w, float h, int time)
	{
		//     body;
		RefEntity	podium = new RefEntity();
		Camera        refdef = new Camera();

		//Vector3f          legsAngles = new Vector3f();
		//vec3_t          torsoAngles;
		//vec3_t          headAngles;

		Vector3f          podiumAngles = new Vector3f();

		Vector3f          origin = new Vector3f();
		int             	renderfx;

		Vector3f          mins = new Vector3f( -16, -16, -24 );
		Vector3f          maxs = new Vector3f( 16, 16, 32 );
		float           len;
		float           xx;

		
		//if(!pi->bodyModel)
		//	return;

		//if(!pi->bodySkin)
		//	return;

		//dp_realtime = time;

		//UI_DrawRect(x, y, w, h, colorYellow);

		Rectangle rect = new Rectangle(x, y, w, h);
		UserInterface.adjustFrom640(rect);
		//UI_AdjustFrom640(&x, &y, &w, &h);

		//memset(&refdef, 0, sizeof(refdef));
		//memset(&body, 0, sizeof(body));
		//memset(&podium, 0, sizeof(podium));

		refdef.rdflags = Camera.RDF_NOWORLDMODEL | Camera.RDF_NOSHADOWS;

		//AxisClear(refdef.viewaxis);
		//refdef.quat

		refdef.x = (int) Math.floor(rect.x);
		refdef.y = (int) Math.floor(rect.y);
		refdef.width = (int) Math.floor(rect.width);
		refdef.height = (int) Math.floor(rect.height);

		refdef.fovX = (int)((float)refdef.width / 640.0f * 60.0f);
		xx = (float) (refdef.width / Math.tan(refdef.fovX / 360 * Math.PI));
		refdef.fovY = (float) Math.atan2(refdef.height, xx);
		refdef.fovY *= (360 / Math.PI);

		// calculate distance so the player nearly fills the box
		len = 0.7f * (maxs.z - mins.z);
		origin.x = (float) (len / Math.tan(Math.toRadians(refdef.fovX) * 0.5) + 10);
		origin.y = 0.5f * (mins.y + maxs.y);
		origin.z = -0.5f * (mins.z + maxs.z);
		origin.z -= len - 20;

		refdef.time = time;
		
		Renderer.clearScene();

		// get the rotation information

		// Quake 2 style
		/*
		legsAngles.x = Angle3f.normalize360(((float)(uis.realtime / 30.0)); //180 - 30;
		legsAngles[PITCH] = 0;
		legsAngles[ROLL] = 0;

		AnglesToAxis(legsAngles, body.axis);

		renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

		// add the body
		VectorCopy(origin, body.origin);
		VectorCopy(body.origin, body.oldorigin);

		body.hModel = pi->bodyModel;
		body.customSkin = pi->bodySkin;
		body.shaderTime = 1.0f;

		body.renderfx = renderfx;
		VectorCopy(origin, body.lightingOrigin);
		body.lightingOrigin[0] -= 150;			// + = behind, - = in front
		body.lightingOrigin[1] += 150;			// + = left, - = right
		body.lightingOrigin[2] += 3000;			// + = above, - = below

		body.backlerp = 1.0f;
		body.frame = 1;
		body.oldframe = 0;

		// modify bones and set proper local bounds for culling
		if(!trap_R_BuildSkeleton(&body.skeleton, pi->animations[LEGS_IDLE].handle, body.oldframe, body.frame, 1.0 - body.backlerp, qfalse))
		{
			Com_Printf("Can't build animation\n");
			return;
		}

		if(body.skeleton.type == SK_RELATIVE)
		{
			// transform relative bones to absolute ones required for vertex skinning
			UI_XPPM_TransformSkeleton(&body.skeleton, NULL);
		}


		//UI_PlayerFloatSprite(pi, origin, trap_R_RegisterShaderNoMip("sprites/balloon3"));
		trap_R_AddRefEntityToScene(&body);
		*/

		//VectorCopy(legsAngles, podiumAngles);
		//AnglesToAxis(podiumAngles, podium.axis);

		// add the podium
		podium.origin = new Vector3f(origin);
		podium.origin.z += 1;

		podium.oldOrigin = new Vector3f(podium.origin);

		podium.hModel = podiumModel;

		//podium.customSkin = pi->bodySkin;
		//podium.shaderTime = 1.0f;

		//podium.renderfx = renderfx;
		podium.lightingOrigin = new Vector3f(origin);
		podium.lightingOrigin.x -= 150;			// + = behind, - = in front
		podium.lightingOrigin.y += 150;			// + = left, - = right
		podium.lightingOrigin.z += 3000;			// + = above, - = below

		podium.lerp = 1.0f;
		podium.frame = 1;
		podium.oldFrame = 0;

		//UI_PlayerFloatSprite(pi, origin, trap_R_RegisterShaderNoMip("sprites/balloon3"));
		
		Renderer.addRefEntityToScene(podium);


	//#if 1
	//	origin[0] -= 150;			// + = behind, - = in front
	//	origin[1] += 150;			// + = left, - = right
	//	origin[2] += 150;			// + = above, - = below
	//	trap_R_AddLightToScene(origin, 300, 1.0, 1.0, 1.0);
	//#endif

	//#if 1
	//	origin[0] -= 150;
	//	origin[1] -= 150;
	//	origin[2] -= 150;
	//	trap_R_AddLightToScene(origin, 400, 1.0, 1.0, 1.0);
	//#endif

		Renderer.renderScene(refdef);
	}
	
	@Override
	public void keyPressed(KeyEvent e) {
		KeyCode key = e.getKey();
		
		if(!e.isDown())
			return;
		
		//Engine.println("MainMenu.keyPressed(event = " + e + ")");
		
		switch(key)
		{
			case CHAR_b:
				Client.startLocalSound(soundMove, SoundChannel.LOCAL_SOUND);
				break;
		
			case CHAR_m:
				Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/02-Parabellum.ogg", "");
				break;
		}
		
		//Client.startLocalSound(soundMove, SoundChannel.LOCAL_SOUND);
		//super.keyPressed(e);
		
		//
	}
}
