package xreal.client.ui;

import java.util.Stack;

import xreal.CVars;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCatchers;
import xreal.client.KeyCode;
import xreal.client.renderer.Renderer;
import xreal.client.ui.event.CharEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.menu.MainMenu;
import xreal.client.ui.menu.MenuFrame;

/**
 * @author Robert Beckebans
 */
public class UserInterface implements UserInterfaceListener
{

	// all drawing is done to a 640*480 virtual screen size
	// and will be automatically scaled to the real resolution
	public static final int	SCREEN_WIDTH	= 640;
	public static final int	SCREEN_HEIGHT	= 480;

	private enum MenuCommand
	{
		NONE, MAIN, INGAME, TEAM, POSTGAME
	}

	private static int		vidWidth;
	private static int		vidHeight;
	private static float	windowAspect;

	private static float	screenScale;
	private static float	screenXBias;
	private static float	screenYBias;
	private static float	screenXScale;
	private static float	screenYScale;

	private static int		realTime;
	private static int		frameTime;

	static Stack<MenuFrame>	menuStack;

	MenuFrame				mainMenu;

	private static Cursor	cursor;

	public static Cursor getCursor()
	{
		return cursor;
	}

	public UserInterface(int vidWidth, int vidHeight, float windowAspect)
	{
		super();
		UserInterface.vidWidth = vidWidth;
		UserInterface.vidHeight = vidHeight;
		UserInterface.windowAspect = windowAspect;

		// for 640x480 virtualized screen
		screenXScale = vidWidth / 640.0f;
		screenYScale = vidHeight / 480.0f;
		screenScale = vidHeight * (1.0f / 480.0f);
		if(vidWidth * 480 > vidHeight * 640)
		{
			// wide screen
			screenXBias = 0.5f * (vidWidth - (vidHeight * (640.0f / 480.0f)));
			screenYBias = 0;
		}
		else if(vidWidth * 480 < vidHeight * 640)
		{
			// narrow screen
			screenXBias = 0;
			screenYBias = 0.5f * (vidHeight - (vidWidth * (480.0f / 640.0f)));
			screenScale = vidWidth * (1.0f / 640.0f);
		}
		else
		{
			// no wide screen
			screenXBias = screenYBias = 0;
		}

		menuStack = new Stack<MenuFrame>();
		cursor = new Cursor();

		mainMenu = new MainMenu();
	}

	@Override
	public boolean consoleCommand(int realTime)
	{
		// Engine.println("UserInterface.consoleCommand(realTime = " + realTime
		// + ")");

		// TODO

		return false;
	}

	@Override
	public void drawConnectScreen(boolean overlay)
	{
		Engine.println("UserInterface.drawConnectScreen(overlay = " + overlay + ")");

		// TODO
	}

	@Override
	public void initUserInterface()
	{
		Engine.println("UserInterface.initUserInterface()");
	}

	@Override
	public boolean isFullscreen()
	{
		// Engine.println("UserInterface.isFullscreen()");

		if(!menuStack.isEmpty() && (Client.getKeyCatchers() & KeyCatchers.UI) != 0)
		{
			MenuFrame activeMenu = menuStack.peek();

			return activeMenu.isFullscreen();
		}

		return false;
	}

	@Override
	public void keyEvent(int time, int key, boolean down)
	{
		//Engine.println("UserInterface.keyEvent(time = " + time + ", key = " + key + ", down = " + down + ")");

		if(!menuStack.isEmpty())
		{
			MenuFrame activeMenu = menuStack.peek();

			KeyCode keyCode = KeyCode.findKeyCode(key);
			// Engine.println("KeyCode = " + keyCode + ", text = '" + (keyCode != null ? keyCode.getText() : "") + "'");

			if(keyCode != null)
			{
				// the char events are just distinguished by or'ing in K_CHAR_FLAG 1024 (ugly)
				if(KeyCode.isCharacterKeyCode(key))
				{
					activeMenu.fireEvent(new CharEvent(activeMenu, time, keyCode, down));
				}
				else
				{
					activeMenu.fireEvent(new KeyEvent(activeMenu, time, keyCode, down));
				}
			}
		}
	}

	@Override
	public void mouseEvent(int time, int dx, int dy)
	{
		// Engine.println("UserInterface.mouseEvent(time = " + time + ", dx = " + dx + ", dy = " + dy + ")");

		if(!menuStack.isEmpty())
		{
			MenuFrame activeMenu = menuStack.peek();

			activeMenu.fireEvent(new MouseEvent(activeMenu, time, 0, dx, dy));
		}
	}

	@Override
	public void refresh(int time) throws Exception
	{
		// Engine.println("UserInterface.refresh(time = " + time + ")");

		UserInterface.frameTime = time - realTime;
		UserInterface.realTime = time;

		if(!menuStack.isEmpty())
		{
			MenuFrame activeMenu = menuStack.peek();

			// if(activeMenu.isFullscreen())
			{
				// // draw the background
				// if(uis.activemenu->showlogo)
				// {
				// FIXME: non 4:3 resolutions are causeing black bars

				// render background
				//Rectangle rect = new Rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				//adjustFrom640(rect);
				//Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, backgroundMaterial);
				// }
				// else
				// {
				// UI_DrawHandlePic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, uis.menuBackNoLogoShader);
				// }
			}

			activeMenu.checkNavigationBarControls();
			activeMenu.alignChildrenAndUpdateBounds();
			activeMenu.render();

			/*
			 * if(uis.firstdraw) { UI_MouseEvent(0, 0); uis.firstdraw = qfalse; }
			 */
		}
		
		cursor.render();
	}

	public static void pushMenu(MenuFrame menu)
	{
		// avoid stacking menus invoked by hotkeys
		if(menuStack.search(menu) == -1)
		{
			menuStack.push(menu);
		}

		// Engine.println("pushMenu stack size = " + stack.size());

		// default cursor position
		// menu->cursor = 0;
		// menu->cursor_prev = 0;

		// m_entersound = qtrue;

		Client.setKeyCatchers(KeyCatchers.UI);
	}

	public static void popMenu()
	{
		// trap_S_StartLocalSound(menu_out_sound, CHAN_LOCAL_SOUND);

		menuStack.pop();
		
		if(menuStack.empty())
		{
			forceMenuOff();
		}
	}

	private static void forceMenuOff()
	{
		while(!menuStack.isEmpty())
		{
			menuStack.pop();
		}

		Client.setKeyCatchers(Client.getKeyCatchers() & ~KeyCatchers.UI);
		Client.clearKeyStates();

		CVars.cl_paused.set("0");

		Client.stopBackgroundTrack();
	}

	private void activateMainMenu()
	{
		CVars.sv_killserver.set("1");

		pushMenu(mainMenu);

		//Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/02-Parabellum.ogg", "");
	}

	private void activateInGameMenu()
	{
		Engine.println("UserInterface.activateInGameMenu() TODO");

		// Cvars.cl_paused.set("1")
	}

	@Override
	public void setActiveMenu(int menu)
	{
		// Engine.println("UserInterface.setActiveMenu(menu = " + menu + ")");

		MenuCommand cmd = MenuCommand.values()[menu];
		switch(cmd)
		{
			case NONE:
				forceMenuOff();
				return;

			case MAIN:
				activateMainMenu();
				return;

			case INGAME:
				activateInGameMenu();
				return;
		}
	}

	@Override
	public void shutdownUserInterface()
	{
		Engine.println("UserInterface.shutdownUserInterface()");

		// TODO
	}

	/**
	 * Adjusted for resolution and screen aspect ratio
	 */
	public static void adjustFrom640(Rectangle r)
	{
		r.x *= screenXScale;
		r.y *= screenYScale;
		r.width *= screenXScale;
		r.height *= screenYScale;

		/*
		 * original codex = *x * uis.screenScale + uis.screenXBias;y = *y * uis.screenScale + uis.screenYBias;w *=
		 * uis.screenScale;h *= uis.screenScale;
		 */
	}

	public static int getRealTime()
	{
		return realTime;
	}

	public static int getFrameTime()
	{
		return frameTime;
	}

	public static int getVidWidth()
	{
		return vidWidth;
	}

	public static int getVidHeight()
	{
		return vidHeight;
	}
}
