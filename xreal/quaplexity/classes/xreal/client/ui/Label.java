package xreal.client.ui;

import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

/**
 * @author Robert Beckebans
 */
public class Label extends Component
{
	public Font		font;
	public String	text;

	public Label()
	{
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
		text = "Label";
		margin = new Thickness(2);
	}

	public Label(String text)
	{
		this();
		this.text = text;
	}

	@Override
	public Rectangle getBounds() throws Exception
	{
		if(text == null || text.isEmpty())
		{
			throw new Exception("empty label text");
		}

		bounds = font.getTextBounds(text, 10, 0);

		return super.getBounds();
	}

	@Override
	public void render()
	{
		if(text != null)
		{
			font.paintText(getX(), getY(), 10, foregroundColor, text, 0, 0, Font.LEFT | Font.DROPSHADOW);
		}

		super.render();
	}
}
