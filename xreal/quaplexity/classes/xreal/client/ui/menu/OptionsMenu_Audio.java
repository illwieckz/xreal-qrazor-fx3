package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import xreal.CVars;
import xreal.Color;
import xreal.Engine;
import xreal.client.KeyCode;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.KeyEvent;

/**
 * @author Robert Beckebans
 */
public class OptionsMenu_Audio extends MenuFrame
{
	Label						title;
	StackPanel					stackPanel;
	MenuSlider					effectsSlider;
	MenuSlider					musicSlider;
	
	public OptionsMenu_Audio() 
	{
		super("menuback");
		
		backgroundImage.color.set(Color.LtGrey);
		
		fullscreen = true;
		
		title = new MenuTitle("AUDIO");
		
		effectsSlider = new MenuSlider("EFFECTS", 0, 1, CVars.s_volume.getValue(), 0.1f)
		{
			public void keyPressed(KeyEvent e)
			{
				Engine.println("effectsSlider.keyPressed(event = " + e + ")");
				
				KeyCode key = e.getKey();
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						e.consume();
						break;
				}
				
				if(!e.isConsumed())
				{
					super.keyPressed(e);
				}
			}
		};
		
		
		musicSlider = new MenuSlider("MUSIC", 0, 1, CVars.s_musicvolume.getValue(), 0.1f)
		{
			public void keyPressed(KeyEvent e)
			{
				Engine.println("musicSlider.keyPressed(event = " + e + ")");
				
				KeyCode key = e.getKey();
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						//UserInterface.pushMenu(new QuitMenu());
						e.consume();
						break;
				}
			}
		};
		
		stackPanel = new StackPanel();
		stackPanel.horizontalAlignment = HorizontalAlignment.Left;
		stackPanel.verticalAlignment = VerticalAlignment.Bottom;
		stackPanel.margin.bottom = 100;
		stackPanel.margin.left = 43;
		
		stackPanel.addChild(title);
		stackPanel.addChild(effectsSlider);
		stackPanel.addChild(musicSlider);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		order.add(effectsSlider);
		order.add(musicSlider);
		setCursorOrder(order);
		
		setCursor(effectsSlider);
	}
	
	@Override
	protected void updateNavigationBarPC()
	{
		navigationBar.clear();
		navigationBar.add("ACCEPT/SAVE", "ui/keyboard_keys/standard_104/enter.png");
		navigationBar.add("BACK", "ui/keyboard_keys/standard_104/esc.png");
		navigationBar.add("RESTORE DEFAULT SETTINGS", "ui/keyboard_keys/standard_104/f2.png");
		
		super.updateNavigationBarPC();
	}
	
	@Override
	protected void updateNavigationBar360()
	{
		navigationBar.clear();
		navigationBar.add("ACCEPT/SAVE", "ui/xbox360/xna/buttons/xboxControllerButtonA.png");
		navigationBar.add("BACK", "ui/xbox360/xna/buttons/xboxControllerButtonB.png");
		navigationBar.add("RESTORE DEFAULT SETTINGS", "ui/xbox360/xna/buttons/xboxControllerButtonY.png");
		
		super.updateNavigationBar360();
	}
}
