package xreal.client.game;

import xreal.Color;
import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

public class FPSCounter {

	private final int	FPS_FRAMES = 40;	
	private int			previousTimes[];
	private int			index;
	private int			previous;
	
	private Font		font;
	
	public FPSCounter() {
		
		previousTimes = new int[FPS_FRAMES];
		
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
	}
	
	public float render(float y)
	{
		int             i, total;
		int             fps;
		int             t, frameTime;

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

			String s = fps + "fps";

			font.paintText(635, y, 10, Color.White, s, 0, 0, Font.RIGHT | Font.DROPSHADOW);
		}
		
		// test
		//ClientGame.getMedia().fontVera = null;

		return y + 16;
	}
}
