package xreal.client.game;

import xreal.CVars;
import xreal.client.ui.Component;
import xreal.client.ui.UserInterface;

/**
 * 
 * @author Robert Beckebans
 */
public class HUD extends Component
{
	private FPSCounter		fpsCounter;
	private BulletStatsView	bulletStatsView;

	public HUD(Lagometer lagometer) throws Exception
	{
		width = UserInterface.SCREEN_WIDTH;
		height = UserInterface.SCREEN_HEIGHT;
		
		fpsCounter = new FPSCounter();
		bulletStatsView = new BulletStatsView();
		
		addChild(lagometer);
		addChild(fpsCounter);
		addChild(bulletStatsView);
	}

	public void render()
	{
		if(!CVars.cg_draw2D.getBoolean())
			return;
		
		alignChildrenAndUpdateBounds();

		super.render();
	}
}
