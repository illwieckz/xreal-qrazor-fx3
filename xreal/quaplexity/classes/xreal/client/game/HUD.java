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
	private Lagometer		lagometer;
	private BulletStatsView	bulletStatsView;

	public HUD(Lagometer lagometer) throws Exception
	{
		setX(0);
		setY(0);
		setWidth(UserInterface.SCREEN_WIDTH);
		setHeight(UserInterface.SCREEN_HEIGHT);
		
		this.lagometer = lagometer;
		fpsCounter = new FPSCounter();
		bulletStatsView = new BulletStatsView();
		
		//addChild(this.lagometer);
		addChild(fpsCounter);
		addChild(bulletStatsView);
	}

	public void render()
	{
		if(!CVars.cg_draw2D.getBoolean())
			return;
		
		alignChildrenAndUpdateBounds();

		super.render();
		
		// TODO bulletStatsView.render();
	}
}
