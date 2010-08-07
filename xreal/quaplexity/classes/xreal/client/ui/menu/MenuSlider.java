package xreal.client.ui.menu;

import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.Slider;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.StackPanel.Orientation;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.VerticalAlignment;


/**
 * @author Robert Beckebans
 */
public class MenuSlider extends Component
{
	private MenuButton	textLabel;
	
	private StackPanel	stackPanel;
	private Slider		slider;
	private Label		valueLabel;

	private float		oldTextLabelAlpha;
	
	private Image		selectionImage;
	
	public MenuSlider(String labelText)
	{
		super();
		
		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
		
		//backgroundImage = new Image("white");
		//backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		textLabel = new MenuButton(labelText);
		//label.backgroundImage = null;
		
		width = 320;
		
		slider = new Slider();
		slider.verticalAlignment = VerticalAlignment.Stretch;
		slider.width = 140;
		//slider.height = 26;
		
		valueLabel = new Label("100");
		valueLabel.height = 26;
		valueLabel.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		valueLabel.textBlock.color.set(Color.LtGrey);
		valueLabel.textBlock.fontSize = 20;
		
		stackPanel = new StackPanel();
		stackPanel.orientation = Orientation.Horizontal;
		//stackPanel.width = 120;
		stackPanel.horizontalAlignment = HorizontalAlignment.Right;
		stackPanel.addChild(slider);
		stackPanel.addChild(valueLabel);
		
		stackPanel.backgroundImage = new Image("white");
		stackPanel.backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		addChild(textLabel);
		addChild(stackPanel);
	}
	
	@Override
	public void focusGained(FocusEvent e)
	{
		oldTextLabelAlpha = textLabel.backgroundImage.color.alpha;
		textLabel.backgroundImage.color.alpha = 0;
		textLabel.textBlock.color.invertRGB();
		
		stackPanel.backgroundImage.color.alpha = 0;
		valueLabel.textBlock.color.invertRGB();
		
		slider.backgroundImage.color.set(0, 0, 0, 0.5f);
		
		stackPanel.backgroundImage.color.alpha = 0;
		
		if(!silent)
		{
			//Client.startLocalSound(focusSound, SoundChannel.LOCAL);
		}
		
		super.focusGained(e);
	}
	
	@Override
	public void focusLost(FocusEvent e)
	{
		textLabel.textBlock.color.invertRGB();
		textLabel.backgroundImage.color.alpha = oldTextLabelAlpha;
		
		valueLabel.textBlock.color.invertRGB();
		
		stackPanel.backgroundImage.color.alpha = 0.5f;
		
		slider.backgroundImage.color.set(0, 0, 0, 0);
		
		super.focusLost(e);
	}
	
	
	@Override
	public void render()
	{
		if(isFocusOwner())
		{
			//border.paintBorder(0, bounds.y, UserInterface.SCREEN_WIDTH, bounds.height);
		
			// draw selection until stack panel
			selectionImage.bounds.x = 0;
			selectionImage.bounds.y = bounds.y;
			selectionImage.bounds.width = stackPanel.bounds.x;
			selectionImage.bounds.height = bounds.height;
			selectionImage.render();
			
			// draw selection behind stack panel to end of screen
			selectionImage.bounds.x = valueLabel.bounds.x;
			selectionImage.bounds.y = bounds.y;
			selectionImage.bounds.width = UserInterface.SCREEN_WIDTH - valueLabel.bounds.x;
			selectionImage.bounds.height = bounds.height;
			selectionImage.render();
		}
		
		super.render();
	}
}
