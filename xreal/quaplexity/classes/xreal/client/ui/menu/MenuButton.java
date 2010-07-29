package xreal.client.ui.menu;


import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Button;
import xreal.client.ui.Image;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;
import xreal.client.ui.border.LineBorder;

/**
 * @author Robert Beckebans
 */
public class MenuButton extends Button
{
	private Image	selectionImage;
	
	private boolean	savedOldAlpha;
	private float	oldAlpha;

	public MenuButton()
	{
		super();

		text = "<Button>";
		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
	}

	public MenuButton(String text)
	{
		super(text);
		
		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
	}

	public MenuButton(String text, float fontSize, int fontStyle)
	{
		super(text, fontSize, fontStyle);

		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
	}
	
	@Override
	public void render()
	{
		if(hasMouseFocus)
		{
			//border.paintBorder(0, bounds.y, UserInterface.SCREEN_WIDTH, bounds.height);
			
			// draw selection
			selectionImage.bounds.x = 0;
			selectionImage.bounds.y = bounds.y;
			selectionImage.bounds.width = UserInterface.SCREEN_WIDTH;
			selectionImage.bounds.height = bounds.height;
			
			selectionImage.render();
		
			if(!savedOldAlpha)
			{
				oldAlpha = backgroundImage.color.alpha;
				savedOldAlpha = true;
				
				textBlock.color.invertRGB();
			}
			
			backgroundImage.color.alpha = 0;
			
		}
		else
		{
			savedOldAlpha = false;
			backgroundImage.color.alpha = oldAlpha;
		}
		
		super.render();
	}
}
