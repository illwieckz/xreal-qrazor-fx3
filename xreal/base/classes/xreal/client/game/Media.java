package xreal.client.game;

import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

public class Media {

	public final Font fontVera;
	public final Font fontVeraSe;
	public final Font fontVeraBold;
	public final Font fontVeraSerifBold;
	
	public Media() {
		// register fonts
		fontVera = Renderer.registerFont("fonts/Vera.ttf", 48);
		fontVeraSe = Renderer.registerFont("fonts/VeraSe.ttf", 48);
		fontVeraBold = Renderer.registerFont("fonts/VeraBd.ttf", 48);
		fontVeraSerifBold = Renderer.registerFont("fonts/VeraSeBd.ttf", 48);
	}
}
