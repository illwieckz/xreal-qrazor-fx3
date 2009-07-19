package xreal.client.ui;

import xreal.client.renderer.Renderer;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.event.MouseMotionListener;

public class Cursor extends Component implements MouseMotionListener {

	private int x;
	private int y;
	
	private int material;
	
	private int frame = 0;
	
	Cursor() {
		material = Renderer.registerMaterialNoMip("cursors/Azenis/Wait");
		
		bounds.x = 0;
		bounds.y = 0;
		bounds.width = 32;
		bounds.height = 32;
		
		UserInterface.adjustFrom640(bounds);
	}
	
	public void render() {
		//Engine.println("Cursor.render()");
		
		frame--;
		frame %= 10;
		
		Renderer.setColor(1, 1, 1, 1);
		Renderer.drawStretchPic(bounds.x, bounds.y, bounds.width, bounds.height, (frame * 0.1f) + 0.02f, 0, ((frame + 1) * 0.1f) + 0.02f, 1, material);
	}

	@Override
	public void mouseMoved(MouseEvent e) {
		//Engine.println("Cursor.mouseMoved()");
		
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
		
		bounds.x = x;
		bounds.y = y;
		bounds.width = 32;
		bounds.height = 32;
		
		UserInterface.adjustFrom640(bounds);
	}

	@Override
	public boolean consumeEvent(Event e) {
		if (e instanceof MouseEvent) {
			mouseMoved((MouseEvent) e);
			
			return true;
		}
		
		return super.consumeEvent(e);
	}
}
