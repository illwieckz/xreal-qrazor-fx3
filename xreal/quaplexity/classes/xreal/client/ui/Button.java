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

		text = "<Button>";
	}

	public Button(String text)
	{
		super();

		this.text = text;
	}

	public Button(String text, float fontSize, int fontStyle)
	{
		super();

		this.text = text;
		this.textBlock.fontSize = fontSize;
		this.textBlock.fontStyle = fontStyle;
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
