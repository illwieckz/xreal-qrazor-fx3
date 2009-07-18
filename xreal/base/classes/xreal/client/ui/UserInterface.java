package xreal.client.ui;

import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCatchers;
import xreal.client.renderer.Renderer;

/**
 * 
 * @author Robert Beckebans
 */
public class UserInterface implements UserInterfaceListener {

	private int vidWidth;
	private int vidHeight;
	private float windowAspect;

	private float screenScale;
	private float screenXBias;
	private float screenYBias;
	private float screenXScale;
	private float screenYScale;

	private int backgroundMaterial;

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

		// TODO
		backgroundMaterial = Renderer.registerMaterialNoMip("menuback");
	}

	@Override
	public boolean isFullscreen() {
		// Engine.println("UserInterface.isFullscreen()");

		// TODO

		return false;
	}

	@Override
	public void keyEvent(int key, boolean down) {
		//Engine.println("UserInterface.keyEvent(key = " + key + ", down = " + down + ")");

		// TODO
	}

	@Override
	public void mouseEvent(int dx, int dy) {
		//Engine.println("UserInterface.mouseEvent(dx = " + dx + ", dy = " + dy + ")");

		// TODO
	}

	@Override
	public void refresh(int time) {
		// Engine.println("UserInterface.refresh(time = " + time + ")");

		// TODO
		
		// render background
		Rectangle rect = new Rectangle(0, 0, 640, 480);
		adjustFrom640(rect);
		
		Renderer.setColor(1, 1, 1, 1);
		Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, backgroundMaterial);
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
	void adjustFrom640(Rectangle r) {
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
