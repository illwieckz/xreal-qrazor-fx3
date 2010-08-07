package xreal.client.ui.menu;


import xreal.Color;
import xreal.client.Client;
import xreal.client.SoundChannel;
import xreal.client.renderer.Font;
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
	
	private float	oldAlpha;
	
	private int		focusSound;

	public MenuButton()
	{
		this("<Button>", 20, 0);
	}

	public MenuButton(String text)
	{
		this(text, 20, Font.DROPSHADOW);
	}

	public MenuButton(String text, float fontSize, int fontStyle)
	{
		super(text, fontSize, fontStyle);

		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
		
		height = 26;
		
		textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		textBlock.color.set(Color.LtGrey);
		
		backgroundImage = new Image("white");
		backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		focusSound = Client.registerSound("sound/misc/menu2.wav");
	}
	
	@Override
	public void focusGained(FocusEvent e)
	{
		textBlock.color.invertRGB();
		
		oldAlpha = backgroundImage.color.alpha;
		backgroundImage.color.alpha = 0;
		
		if(!silent)
		{
			//Client.startLocalSound(focusSound, SoundChannel.LOCAL);
		}
		
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
