package xreal.client.game;

import xreal.CVars;

/**
 * 
 * @author Robert Beckebans
 */
public class HUD {
	
	private FPSCounter fpsCounter = new FPSCounter();
	
	public void draw()
	{
		if(!CVars.cg_draw2D.getBoolean())
			return;
		
		if(CVars.cg_drawFPS.getBoolean()) {
			fpsCounter.drawFPS(10);
		}
	}
	
}
