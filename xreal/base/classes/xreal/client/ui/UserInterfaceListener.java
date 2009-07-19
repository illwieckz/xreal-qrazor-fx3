package xreal.client.ui;

/**
 * @author Robert Beckebans
 */
public interface UserInterfaceListener {
	
	public void initUserInterface();

	public void shutdownUserInterface();

	public void keyEvent(int time, int key, boolean down);

	public void mouseEvent(int time, int dx, int dy);

	public void refresh(int time);

	public boolean isFullscreen();

	public void setActiveMenu(int menu);

	public void drawConnectScreen(boolean overlay);
	
	public boolean consoleCommand(int realTime);
}
