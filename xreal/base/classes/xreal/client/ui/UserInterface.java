package xreal.client.ui;

import xreal.Color;
import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCatchers;
import xreal.client.KeyCode;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.MouseEvent;

/**
 * 
 * @author Robert Beckebans
 */
public class UserInterface extends Container implements UserInterfaceListener {
	
	// all drawing is done to a 640*480 virtual screen size
	// and will be automatically scaled to the real resolution
	public static final int SCREEN_WIDTH = 640;
	public static final int SCREEN_HEIGHT = 480;

	private int vidWidth;
	private int vidHeight;
	private float windowAspect;

	private static float screenScale;
	private static float screenXBias;
	private static float screenYBias;
	private static float screenXScale;
	private static float screenYScale;

	private int backgroundMaterial;
	
	private Cursor cursor;
	private Font textFont;

	public UserInterface(int vidWidth, int vidHeight, float windowAspect) {
		super();
		this.vidWidth = vidWidth;
		this.vidHeight = vidHeight;
		this.windowAspect = windowAspect;
		
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
		
		
		backgroundMaterial = Renderer.registerMaterialNoMip("menuback");
		
		cursor = new Cursor();
		children.add(cursor);
		
		textFont = Renderer.registerFont("fonts/Vera.ttf", 48);
	}

	@Override
	public boolean consoleCommand(int realTime) {
		// Engine.println("UserInterface.consoleCommand(realTime = " + realTime
		// + ")");

		// TODO

		return false;
	}

	@Override
	public void drawConnectScreen(boolean overlay) {
		Engine.println("UserInterface.drawConnectScreen(overlay = " + overlay + ")");

		// TODO
	}

	@Override
	public void initUserInterface() {
		Engine.println("UserInterface.initUserInterface()");

		// TODO ?
		
	}

	@Override
	public boolean isFullscreen() {
		// Engine.println("UserInterface.isFullscreen()");

		// TODO

		return false;
	}

	@Override
	public void keyEvent(int time, int key, boolean down) {
		Engine.println("UserInterface.keyEvent(time = " + time + ", key = " + key + ", down = " + down + ")");

		// TODO
		KeyCode keyCode = KeyCode.findKeyCode(key);
		Engine.println("KeyCode = " + keyCode + ", text = '" + (keyCode != null ? keyCode.getText() : "") + "'");
		
		if(keyCode != null) {
			fireEvent(new KeyEvent(this, time, keyCode, down));
		}
	}

	@Override
	public void mouseEvent(int time, int dx, int dy) {
		//Engine.println("UserInterface.mouseEvent(time = " + time + ", dx = " + dx + ", dy = " + dy + ")");
		
		fireEvent(new MouseEvent(this, time, 0, dx, dy));
	}

	@Override
	public void refresh(int time) {
		// Engine.println("UserInterface.refresh(time = " + time + ")");
		
		render();
	}
	
	@Override
	public void render() {
		
		// render background
		Rectangle rect = new Rectangle(0, 0, 640, 480);
		adjustFrom640(rect);
		
		Renderer.setColor(1, 1, 1, 1);
		Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, backgroundMaterial);
		
		String message = ConsoleColorStrings.YELLOW + "Java " + ConsoleColorStrings.WHITE + "is sooo easy " + ConsoleColorStrings.RED + "!";
		
		/*
		rect = textFont.getStringBounds(message, 0.5f, 0);
		rect.x = SCREEN_WIDTH / 2;
		rect.y = SCREEN_HEIGHT / 2;
		adjustFrom640(rect);
		*/
		
		Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, backgroundMaterial);
		
		textFont.paintText(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.5f, Color.White, message, 0, 0, Font.CENTER);
		
		super.render();
	}

	@Override
	public void setActiveMenu(int menu) {
		// Engine.println("UserInterface.setActiveMenu(menu = " + menu + ")");

		// TODO
		Client.setKeyCatchers(KeyCatchers.UI);
	}

	@Override
	public void shutdownUserInterface() {
		Engine.println("UserInterface.shutdownUserInterface()");

		// TODO
	}

	/**
	 * Adjusted for resolution and screen aspect ratio
	 */
	public static void adjustFrom640(Rectangle r) {
		r.x *= screenXScale;
		r.y *= screenYScale;
		r.width *= screenXScale;
		r.height *= screenYScale;
		
		/*
		 * original code
		 *x = *x * uis.screenScale + uis.screenXBias;
		 *y = *y * uis.screenScale + uis.screenYBias;
		 *w *= uis.screenScale;
		 *h *= uis.screenScale;
		 */
	}

}
