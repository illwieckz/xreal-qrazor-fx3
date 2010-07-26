package xreal.client.ui;

import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

public class TextBlock extends Component
{
	public Font		font;
	public float	fontSize;
	public String	text;

	public TextBlock()
	{
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
		fontSize = 10;
		
		text = "TextBlock";
		margin.set(3);
	}

	public TextBlock(String text)
	{
		this();
		this.text = text;
	}

	@Override
	public Rectangle getSize() throws Exception
	{
		if(text == null || text.isEmpty())
		{
			throw new Exception("empty TextBlock text");
		}

		
		Rectangle rect = font.getTextBounds(text, fontSize, 0);
		
		//rect.width += margin.left + margin.right;
		//rect.height += margin.top + margin.bottom;
		
		if(width != 0)
		{
			rect.width = width;
		}
		
		if(height != 0)
		{
			rect.height = height;
		}

		return rect;
	}

	@Override
	public void render()
	{
		if(text != null)
		{
			font.paintText(bounds.x, bounds.y, fontSize, color, text, 0, 0, Font.LEFT | Font.DROPSHADOW);
		}

		super.render();
	}
}
