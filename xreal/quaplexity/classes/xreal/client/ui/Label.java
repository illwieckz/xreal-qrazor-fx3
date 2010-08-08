package xreal.client.ui;

import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;

/**
 * @author Robert Beckebans
 */
public class Label extends Component
{
	public final TextBlock	textBlock;

	public Label()
	{
		textBlock = new TextBlock("Label");
		textBlock.horizontalAlignment = HorizontalAlignment.Center;
		textBlock.verticalAlignment = VerticalAlignment.Center;
		
		addChild(textBlock);
	}

	public Label(String text)
	{
		this();
		textBlock.text = text;
	}

	@Override
	public Rectangle getSize() throws Exception
	{
		/*
		if(text == null || text.isEmpty())
		{
			throw new Exception("empty label text");
		}
		else
		{
			textBlock.text = text;
		}
		*/

		/*
		Rectangle rect = textBlock.getSize();

		if(width != 0)
		{
			rect.width = width;
		}

		if(height != 0)
		{
			rect.height = height;
		}
		*/

		return super.getSize();
	}

	@Override
	public void render()
	{
		/*
		if(text != null)
		{
			textBlock.text = text;
		}
		*/
		
		super.render();
		
	}
}
