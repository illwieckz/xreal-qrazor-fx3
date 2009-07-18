package xreal.client.ui;

import xreal.Engine;

/**
 * 
 * @author Robert Beckebans
 */
public class UserInterface implements UserInterfaceListener {

	@Override
	public boolean consoleCommand(int realTime) {
		//Engine.println("UserInterface.consoleCommand(realTime = " + realTime + ")");
		
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
	}

	@Override
	public boolean isFullscreen() {
		//Engine.println("UserInterface.isFullscreen()");
		
		// TODO
		
		return false;
	}

	@Override
	public void keyEvent(int key, boolean down) {
		Engine.println("UserInterface.keyEvent(key = " + key + ", down = " + down + ")");
		
		// TODO
	}

	@Override
	public void mouseEvent(int dx, int dy) {
		Engine.println("UserInterface.mouseEvent(dx = " + dx + ", dy = " + dy + ")");
		
		// TODO
	}

	@Override
	public void refresh(int time) {
		Engine.println("UserInterface.refresh(time = " + time + ")");
		
		// TODO
	}

	@Override
	public void setActiveMenu(int menu) {
		//Engine.println("UserInterface.setActiveMenu(menu = " + menu + ")");
		
		// TODO
	}

	@Override
	public void shutdownUserInterface() {
		Engine.println("UserInterface.shutdownUserInterface()");
		
		// TODO
	}

}
