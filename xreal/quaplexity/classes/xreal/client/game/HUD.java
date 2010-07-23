package xreal.client.game;

import xreal.CVars;

/**
 * 
 * @author Robert Beckebans
 */
public class HUD
{
	private FPSCounter		fpsCounter;
	private BulletStatsView	bulletStatsView;

	public HUD()
	{
		fpsCounter = new FPSCounter();
		bulletStatsView = new BulletStatsView();
	}

	public void render()
	{
		if(!CVars.cg_draw2D.getBoolean())
			return;

		if(CVars.cg_drawFPS.getBoolean())
		{
			fpsCounter.render(10);
		}
		
		// TODO bulletStatsView.render();
	}
}
