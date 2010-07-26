package xreal.client.ui;

import xreal.client.renderer.Renderer;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.event.MouseMotionListener;

public class Cursor extends Component implements MouseMotionListener
{
	private int	x;
	private int	y;

	private int	material;

	private int	frame	= 0;
	private int time;

	Cursor()
	{
		material = Renderer.registerMaterialNoMip("cursors/Azenis/Wait");

		width = 24;
		height = 24;
	}

	public void render()
	{
		// Engine.println("Cursor.render()");

		time += UserInterface.getFrameTime();
		int numFrames = time / (1000 / 24);
		time -= numFrames * (1000 / 24);
		
		frame -= numFrames;
		frame %= 10;

		Rectangle rect;
		try
		{
			rect = new Rectangle(getBounds());
			UserInterface.adjustFrom640(rect);

			Renderer.setColor(1, 1, 1, 1);
			Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, (frame * 0.1f) + 0.02f, 0, ((frame + 1) * 0.1f) + 0.02f, 1, material);
		}
		catch(Exception e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

	@Override
	public void mouseMoved(MouseEvent e)
	{
		// Engine.println("Cursor.mouseMoved()");

		x += e.getX();
		y += e.getY();

		if(x < 0)
			x = 0;
		else if(x > UserInterface.SCREEN_WIDTH)
			x = UserInterface.SCREEN_WIDTH;

		if(y < 0)
			y = 0;
		else if(y > UserInterface.SCREEN_HEIGHT)
			y = UserInterface.SCREEN_HEIGHT;

		setX(x);
		setY(y);
	}

	@Override
	public void processEvent(Event e)
	{
		if(e instanceof MouseEvent)
		{
			mouseMoved((MouseEvent) e);
		}

		super.processEvent(e);
	}
}
