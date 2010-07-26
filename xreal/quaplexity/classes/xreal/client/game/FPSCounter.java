package xreal.client.game;

import xreal.CVars;
import xreal.Color;
import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Label;
import xreal.client.ui.Rectangle;
import xreal.client.ui.VerticalAlignment;

public class FPSCounter extends Label
{
	private final int	FPS_FRAMES	= 40;
	private int			previousTimes[];
	private int			index;
	private int			previous;

	public FPSCounter()
	{
		super();
		
		color.set(Color.White);
		
		horizontalAlignment = HorizontalAlignment.Right;
		verticalAlignment = VerticalAlignment.Top;
		
		previousTimes = new int[FPS_FRAMES];
	}
	
	private void updateText()
	{
		int i, total;
		int fps;
		int t, frameTime;
		
		if(!CVars.cg_drawFPS.getBoolean())
		{
			text = null;
			return;
		}

		// don't use serverTime, because that will be drifting to
		// correct for internet lag changes, timescales, timedemos, etc
		t = Engine.getTimeInMilliseconds();
		frameTime = t - previous;
		previous = t;

		previousTimes[index % FPS_FRAMES] = frameTime;
		index++;

		if(index > FPS_FRAMES)
		{
			// average multiple frames together to smooth changes out a bit
			total = 0;
			for(i = 0; i < FPS_FRAMES; i++)
			{
				total += previousTimes[i];
			}

			if(total <= 0)
			{
				total = 1;
			}
			fps = 1000 * FPS_FRAMES / total;

			text = fps + "fps";
		}	
	}
	
	@Override
	public Rectangle getSize() throws Exception
	{
		updateText();
		
		return super.getSize();
	}
	
	@Override
	public void render()
	{
		// TODO Auto-generated method stub
		super.render();
	}
}
