package xreal.client.ui;

import xreal.Color;
import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.ui.border.LineBorder;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.menu.MenuFrame;

public class MainMenu extends MenuFrame {

	private static final int MAIN_MENU_VERTICAL_SPACING = 34;
	
	Button singlePlayer;
	
	MainMenu() 
	{
		super();
		
		fullscreen = true;
		wrapAround = true;
		showlogo = true;
		
		int y = 134;
		singlePlayer = new Button("SINGLEPLAYER", 27, Font.DROPSHADOW) {
			@Override
			public void keyPressed(KeyEvent e) {
				Engine.println("clicked single player button");
				
				//super.keyPressed(e);
			}
		};
		singlePlayer.setXCenter(UserInterface.SCREEN_WIDTH / 2);
		singlePlayer.setYCenter(y);
		//singlePlayer.setCenter(0, 0);
		singlePlayer.setBorder(new LineBorder(Color.Red));
		
		children.add(singlePlayer);

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
		
		String message = ConsoleColorStrings.YELLOW + "Java " + ConsoleColorStrings.WHITE + "is sooo easy " + ConsoleColorStrings.RED + "!";
		
		fontVera.paintText(UserInterface.SCREEN_WIDTH / 2, UserInterface.SCREEN_HEIGHT / 2, 24, Color.White, message, 0, 0, Font.CENTER);
		
		/*
		rect = textFont.getStringBounds(message, 0.5f, 0);
		rect.x = SCREEN_WIDTH / 2;
		rect.y = SCREEN_HEIGHT / 2;
		adjustFrom640(rect);
		*/
		
		// stress test
		/*
		float textWidth = textFont.getTextWidth(message, 0.125f, 0);
		float textHeight = textFont.getTextHeight(message, 0.125f, 0);
		
		int count = 0;
		for (float x = 0; x < SCREEN_WIDTH; x += textWidth) {
			for (float y = 0; y < SCREEN_HEIGHT; y += textHeight) {
				textFont.paintText(x, y, 0.125f, Color.White, message, 0, 0, Font.LEFT);
			}
		}
		*/
		
		fontVera.paintText(320, 470, 12, Color.White, "XreaL(c) 2005-2009, XreaL Team - http://xreal.sourceforge.net", 0, 0,
				  Font.CENTER | Font.DROPSHADOW);
		
		super.render();
	}
}
