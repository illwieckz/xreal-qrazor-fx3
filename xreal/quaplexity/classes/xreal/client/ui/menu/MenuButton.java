package xreal.client.ui.menu;


import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Button;
import xreal.client.ui.Image;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;
import xreal.client.ui.border.LineBorder;
import xreal.client.ui.event.FocusEvent;

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
	public void focusGained(FocusEvent e)
	{
		textBlock.color.invertRGB();
		
		oldAlpha = backgroundImage.color.alpha;
		backgroundImage.color.alpha = 0;
		
		super.focusGained(e);
	}
	
	@Override
	public void focusLost(FocusEvent e)
	{
		textBlock.color.invertRGB();
		backgroundImage.color.alpha = oldAlpha;
		
		super.focusLost(e);
	}
	
	
	@Override
	public void render()
	{
		if(isFocusOwner())
		{
			//border.paintBorder(0, bounds.y, UserInterface.SCREEN_WIDTH, bounds.height);
		
			// draw selection
			selectionImage.bounds.x = 0;
			selectionImage.bounds.y = bounds.y;
			selectionImage.bounds.width = UserInterface.SCREEN_WIDTH;
			selectionImage.bounds.height = bounds.height;
		
			selectionImage.render();
		}
		
		super.render();
	}
}
