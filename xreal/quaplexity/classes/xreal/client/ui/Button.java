package xreal.client.ui;

import xreal.Color;
import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.menu.NavigationBar;

/**
 * @author Robert Beckebans
 */
public class Button extends AbstractButton
{
	public Button()
	{
		super();

		textBlock.text = "<Button>";
	}

	public Button(String text)
	{
		super();

		textBlock.text = text;
	}

	public Button(String text, float fontSize, int fontStyle)
	{
		super();

		textBlock.text = text;
		textBlock.fontSize = fontSize;
		textBlock.fontStyle = fontStyle;
	}

	@Override
	public void render()
	{
		//if(hasMouseFocus)
		//fontStyle | ( ? Font.PULSE : 0)
		
		if(border != null)
		{
			border.paintBorder(this, getX(), getY(), getWidth(), getHeight());
		}
		
		super.render();
	}
}
