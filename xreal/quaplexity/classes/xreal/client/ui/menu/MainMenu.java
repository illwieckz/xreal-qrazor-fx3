package xreal.client.ui.menu;

import java.util.Vector;

import xreal.Color;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.FocusEvent.FocusType;


/**
 * @author Robert Beckebans
 */
public class MainMenu extends MenuFrame 
{
	Label						label;
	StackPanel					stackPanel;
	Button						singleplayerButton;
	Button						multiplayerButton;
	Button						optionsButton;
	Button						extrasButton;
	Button						quitButton;
	
	public MainMenu() 
	{
		super("menuback");
		//super("screenshots/MainMenu");
		//super("ui/wallpapers/retro_cans");
		
		backgroundImage.color.set(Color.LtGrey);
		
		fullscreen = true;
		wrapAround = true;
		showlogo = true;
		
		Color backgroundColor = new Color(0.0f, 0.0f, 0.0f, 0.5f);
		
		
		label = new Label("MAIN MENU");
		label.height = 32;
		label.margin.bottom = 26;
		label.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		label.textBlock.fontSize = 34;
		label.textBlock.color.set(Color.LtGrey);
		label.backgroundImage = new Image("white");
		label.backgroundImage.color.set(backgroundColor);
		
		
		singleplayerButton = new MenuButton("SINGLEPLAYER");
		singleplayerButton.height = 28;
		singleplayerButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		singleplayerButton.textBlock.fontSize = 22;
		singleplayerButton.textBlock.color.set(Color.LtGrey);
		singleplayerButton.backgroundImage = new Image("white");
		singleplayerButton.backgroundImage.color.set(backgroundColor);
		
		multiplayerButton = new MenuButton("MULTIPLAYER");
		multiplayerButton.height = 28;
		multiplayerButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		multiplayerButton.textBlock.fontSize = 22;
		multiplayerButton.textBlock.color.set(Color.LtGrey);
		multiplayerButton.backgroundImage = new Image("white");
		multiplayerButton.backgroundImage.color.set(backgroundColor);
		
		optionsButton = new MenuButton("OPTIONS");
		optionsButton.setFocusable(false);
		optionsButton.height = 28;
		optionsButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		optionsButton.textBlock.fontSize = 22;
		optionsButton.textBlock.color.set(Color.DkGrey);
		optionsButton.backgroundImage = new Image("white");
		optionsButton.backgroundImage.color.set(backgroundColor);
		
		extrasButton = new MenuButton("EXTRAS");
		extrasButton.setFocusable(false);
		extrasButton.height = 28;
		extrasButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		extrasButton.textBlock.fontSize = 22;
		extrasButton.textBlock.color.set(Color.DkGrey);
		extrasButton.backgroundImage = new Image("white");
		extrasButton.backgroundImage.color.set(backgroundColor);
		
		quitButton = new MenuButton("QUIT")
		{
			public void keyPressed(KeyEvent e)
			{
				KeyCode key = e.getKey();
				
				if(!e.isDown())
					return;
				
				Engine.println("quitButton.keyPressed(event = " + e + ")");
				
				switch(key)
				{
					case ENTER:
						// TODO
						e.consume();
						break;
				}
			}
		};
		
		quitButton.height = 28;
		quitButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		quitButton.textBlock.fontSize = 22;
		quitButton.textBlock.color.set(Color.LtGrey);
		quitButton.backgroundImage = new Image("white");
		quitButton.backgroundImage.color.set(backgroundColor);
		
		
		
		
		stackPanel = new StackPanel();
		stackPanel.horizontalAlignment = HorizontalAlignment.Left;
		stackPanel.verticalAlignment = VerticalAlignment.Bottom;
		stackPanel.margin.bottom = 100;
		stackPanel.margin.left = 30;
		
		stackPanel.addChild(label);
		stackPanel.addChild(singleplayerButton);
		stackPanel.addChild(optionsButton);
		stackPanel.addChild(extrasButton);
		stackPanel.addChild(quitButton);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		order.add(singleplayerButton);
		order.add(optionsButton);
		order.add(extrasButton);
		order.add(quitButton);
		setCursorOrder(order);
		
		setCursor(singleplayerButton);
	}

	@Override
	public void render()
	{
		//String message = "Use the console with Shift + Escape";
		
		//centerLabel.text = message;
		
		//fontVera.paintText(UserInterface.SCREEN_WIDTH / 2, UserInterface.SCREEN_HEIGHT / 2, 16, Color.White, message, 0, 0, Font.CENTER);
		
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
		//fontVera.paintText(320, 470, 12, Color.White, "XreaL(c) 2005-2009, XreaL Team - http://xreal.sourceforge.net", 0, 0,
		//		  Font.CENTER | Font.DROPSHADOW);
		
		//renderViewTest(320, UserInterface.SCREEN_HEIGHT / 2, 200, 300, UserInterface.getRealTime());
		
		super.render();
	}
	
	
	
	@Override
	public void keyPressed(KeyEvent e)
	{
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
		super.keyPressed(e);
	}
	
	
	
	
	
	
	/*
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
		
//		legsAngles.x = Angle3f.normalize360(((float)(uis.realtime / 30.0)); //180 - 30;
//		legsAngles[PITCH] = 0;
//		legsAngles[ROLL] = 0;
//
//		AnglesToAxis(legsAngles, body.axis);
//
//		renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
//
//		// add the body
//		VectorCopy(origin, body.origin);
//		VectorCopy(body.origin, body.oldorigin);
//
//		body.hModel = pi->bodyModel;
//		body.customSkin = pi->bodySkin;
//		body.shaderTime = 1.0f;
//
//		body.renderfx = renderfx;
//		VectorCopy(origin, body.lightingOrigin);
//		body.lightingOrigin[0] -= 150;			// + = behind, - = in front
//		body.lightingOrigin[1] += 150;			// + = left, - = right
//		body.lightingOrigin[2] += 3000;			// + = above, - = below
//
//		body.backlerp = 1.0f;
//		body.frame = 1;
//		body.oldframe = 0;
//
//		// modify bones and set proper local bounds for culling
//		if(!trap_R_BuildSkeleton(&body.skeleton, pi->animations[LEGS_IDLE].handle, body.oldframe, body.frame, 1.0 - body.backlerp, qfalse))
//		{
//			Com_Printf("Can't build animation\n");
//			return;
//		}
//
//		if(body.skeleton.type == SK_RELATIVE)
//		{
//			// transform relative bones to absolute ones required for vertex skinning
//			UI_XPPM_TransformSkeleton(&body.skeleton, NULL);
//		}
//
//
//		//UI_PlayerFloatSprite(pi, origin, trap_R_RegisterShaderNoMip("sprites/balloon3"));
//		trap_R_AddRefEntityToScene(&body);
		

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
	*/
}
