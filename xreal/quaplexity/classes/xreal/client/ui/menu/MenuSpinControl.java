package xreal.client.ui.menu;

import xreal.Color;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.Slider;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.StackPanel.Orientation;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.KeyListener;

/**
 * @author Robert Beckebans
 */
public abstract class MenuSpinControl extends Component implements KeyListener
{
	private MenuButton	textLabel;
	
	private StackPanel	stackPanel;
	private Label		valueLabel;

	private float		oldTextLabelAlpha;
	
	private Image		selectionImage;
	
	private int			curValue;
	private String[]	items;
	
	public void setCurValue(int index)
	{
		if(index < 0)
		{
			curValue = 0;
		}
		else if(index >= items.length)
		{
			curValue = items.length - 1;
		}
		else
		{
			curValue = index;
		}
		
		updateValueLabel(curValue);
	}

	public String getCurValue()
	{
		return items[curValue];
	}

	private float 		stepSize;
	
	private int			moveSound;
	private int			buzzSound;
	
	public MenuSpinControl(String labelText, String[] items, int itemIndex)
	{
		super();
		
		this.items = items;
		
		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
		
		//backgroundImage = new Image("white");
		//backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		textLabel = new MenuButton(labelText);
		//label.backgroundImage = null;
		
		width = 400;
		
		valueLabel = new Label("100");
		valueLabel.height = 26;
		valueLabel.width = 180;
		valueLabel.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		valueLabel.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		valueLabel.textBlock.color.set(Color.LtGrey);
		valueLabel.textBlock.fontSize = 20;
		
		setCurValue(itemIndex);
		updateValueLabel(this.curValue);
		
		stackPanel = new StackPanel();
		stackPanel.orientation = Orientation.Horizontal;
		//stackPanel.width = 120;
		stackPanel.horizontalAlignment = HorizontalAlignment.Right;
		stackPanel.addChild(valueLabel);
		
		stackPanel.backgroundImage = new Image("white");
		stackPanel.backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		addChild(textLabel);
		addChild(stackPanel);
		
		
		moveSound = Client.registerSound("sound/misc/menu2.wav");
		buzzSound = Client.registerSound("sound/misc/menu4.wav");
	}
	
	private void updateValueLabel(int value)
	{
		valueLabel.textBlock.text = items[value];
	}
	
	@Override
	public void focusGained(FocusEvent e)
	{
		oldTextLabelAlpha = textLabel.backgroundImage.color.alpha;
		textLabel.backgroundImage.color.alpha = 0;
		textLabel.textBlock.color.invertRGB();
		
		stackPanel.backgroundImage.color.alpha = 0;
		valueLabel.textBlock.color.invertRGB();
		
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
		
		super.focusLost(e);
	}
	
	
	@Override
	public void render()
	{
		if(isFocusOwner())
		{
			// draw selection
			selectionImage.bounds.x = 0;
			selectionImage.bounds.y = bounds.y;
			selectionImage.bounds.width = UserInterface.SCREEN_WIDTH;
			selectionImage.bounds.height = bounds.height;
			selectionImage.render();
		}
		
		super.render();
	}
	
	@Override
	public void keyPressed(KeyEvent e)
	{
		//int x;
		//int oldvalue;
		//float step;

		//step = this.step;
		//if(step < 1)
		//	step = 1;
		
		//Engine.println("MenuSlider.keyPressed(event = " + e + ")");
		
		KeyCode key = e.getKey();
		switch(key)
		{
			case LEFTARROW:
			case KP_LEFTARROW:
			case XBOX360_DPAD_LEFT:
				if(curValue > 0)
				{
					curValue--;
					updateValueLabel(curValue);
					
					//Client.startLocalSound(moveSound, SoundChannel.LOCAL);
				}
				else
				{
					Client.startLocalSound(buzzSound, SoundChannel.LOCAL);
				}
				break;
				
				
			case RIGHTARROW:
			case KP_RIGHTARROW:
			case XBOX360_DPAD_RIGHT:
				if(curValue < (items.length -1))
				{
					curValue++;
					updateValueLabel(curValue);
					
					//Client.startLocalSound(moveSound, SoundChannel.LOCAL);
				}
				else
				{
					Client.startLocalSound(buzzSound, SoundChannel.LOCAL);
				}
				break;
			
		}
	}
	
	@Override
	public void keyReleased(KeyEvent e)
	{
		// TODO Auto-generated method stub
	}
}
