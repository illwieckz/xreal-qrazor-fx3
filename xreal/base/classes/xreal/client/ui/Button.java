package xreal.client.ui;

import xreal.Color;
import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

public class Button extends Component {

	private Color color = Color.White;

	private Font font = Renderer.registerFont("fonts/GOODTIME.ttf", 48);
	// private Font font = Renderer.registerFont("fonts/VeraBd.ttf", 48);
	private float fontSize = 24;
	private int fontStyle;

	private String text = "<Button>";

	Button() {
		super();
		
		calcBounds();
	}

	Button(String text) {
		super();

		this.text = text;

		calcBounds();
	}
	
	Button(String text, float fontSize, int fontStyle) {
		super();

		this.text = text;
		this.fontStyle = fontStyle;
		this.fontSize = fontSize;

		calcBounds();
	}

	@Override
	public void render() {
		//Rectangle rect = new Rectangle(bounds);
		//UserInterface.adjustFrom640(rect);
		
		//if(hasFlags(QMF_HASMOUSEFOCUS))
		//	Engine.println("button has focus");

		font.paintText(bounds.x, bounds.y, fontSize, color, text, 0, 0, fontStyle | (hasFlags(QMF_HASMOUSEFOCUS) ? Font.PULSE : 0));

		if(border != null) {
			border.paintBorder(this, bounds.x, bounds.y, bounds.width, bounds.height);
		}
	}

	private void calcBounds() {
		bounds.width = font.getTextWidth(text, fontSize, 0);
		bounds.height = font.getTextHeight(text, fontSize, 0);
	}
}
