package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import xreal.Color;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.VerticalAlignment;

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
		
		Map<String, String> navButtons = new HashMap<String, String>();
		navButtons.put("RESTORE DEFAULT SETTINGS", "ui/keyboard_keys/standard_104/f2.png");
		navButtons.put("BACK", "ui/keyboard_keys/standard_104/esc.png");
		navButtons.put("ACCEPT/SAVE", "ui/keyboard_keys/standard_104/enter.png");
		navigationBar.set(navButtons);
		
		fullscreen = true;
		
		title = new MenuTitle("AUDIO");
		
		effectsSlider = new MenuSlider("EFFECTS");
		musicSlider = new MenuSlider("MUSIC");
		{
			/*
			public void keyPressed(KeyEvent e)
			{
				if(!e.isDown())
					return;
			
				KeyCode key = e.getKey();
				
				Engine.println("keyboardSetupButton.keyPressed(event = " + e + ")");
				
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						UserInterface.pushMenu(new QuitMenu());
						e.consume();
						break;
				}
			}
			*/
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
}
