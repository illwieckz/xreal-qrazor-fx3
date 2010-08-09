package xreal.client.ui.menu;

import xreal.Color;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.Rectangle;
import xreal.client.ui.Slider;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.StackPanel.Orientation;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.KeyListener;
import xreal.client.ui.VerticalAlignment;


/**
 * @author Robert Beckebans
 */
public abstract class MenuSlider extends Component implements KeyListener
{
	private MenuButton	textLabel;
	
	private StackPanel	stackPanel;
	private Slider		slider;
	private Label		valueLabel;

	private float		oldTextLabelAlpha;
	
	private Image		selectionImage;
	private Image		thumbImage;
	
	private float		minValue;
	private float		maxValue;
	private float		curValue;
	
	public void setCurValue(float curValue)
	{
		if(curValue < minValue)
		{
			this.curValue = minValue;
		}
		else if(curValue > maxValue)
		{
			this.curValue = maxValue;
		}
		else
		{
			this.curValue = curValue;
		}
		
		updateValueLabel(this.curValue);
	}

	public float getCurValue()
	{
		return curValue;
	}

	private float 		stepSize;
	
	private int			moveSound;
	private int			buzzSound;
	
	public MenuSlider(String labelText, float minValue, float maxValue, float curValue, float stepSize)
	{
		super();
		
		this.minValue = minValue;
		this.maxValue = maxValue;
		this.curValue = curValue;
		this.stepSize = stepSize;
		
		selectionImage = new Image("white");
		selectionImage.color.set(1, 1, 0, 0.7f);
		
		thumbImage = new Image("white");
		thumbImage.color.set(1, 0, 0, 0.9f);
		
		//backgroundImage = new Image("white");
		//backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
		
		textLabel = new MenuButton(labelText);
		//label.backgroundImage = null;
		
		width = 400;
		
		slider = new Slider();
		
		slider.verticalAlignment = VerticalAlignment.Stretch;
		slider.width = 140;
		//slider.height = 26;
		
		valueLabel = new Label("100");
		valueLabel.height = 26;
		valueLabel.width = 40;
		valueLabel.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		valueLabel.textBlock.color.set(Color.LtGrey);
		valueLabel.textBlock.fontSize = 20;
		updateValueLabel(curValue);
		
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
		
		
		moveSound = Client.registerSound("sound/misc/menu2.wav");
		buzzSound = Client.registerSound("sound/misc/menu4.wav");
	}
	
	private void updateValueLabel(float value)
	{
		valueLabel.textBlock.text = Integer.toString((int) ((value / maxValue) * 100));
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
		
		{
			// draw thumb
			thumbImage.bounds.x = slider.bounds.x + slider.bounds.width * (curValue / maxValue);
			thumbImage.bounds.y = bounds.y;
			thumbImage.bounds.width = 4;
			thumbImage.bounds.height = bounds.height;
			thumbImage.render();
		}
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
				if(curValue > minValue)
				{
					curValue -= stepSize;
					curValue = Math.round(curValue * 100.0f) / 100.0f;
					curValue = Math.max(curValue, minValue);
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
				if(curValue < maxValue)
				{
					curValue += stepSize;
					curValue = Math.round(curValue * 100.0f) / 100.0f;
					curValue = Math.min(curValue, maxValue);
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
