package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.CVars;
import xreal.Color;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.KeyListener;

/**
 * @author Robert Beckebans
 */
public class OptionsMenu_Audio extends MenuFrame
{
	Label						title;
	StackPanel					stackPanel;
	MenuSlider					effectsSlider;
	MenuSlider					musicSlider;
//	MenuSpinControl				soundQuality;
	MenuSpinControl				useOpenAL;
	
	int							radioSignalSound;
	
	public OptionsMenu_Audio() 
	{
		super("menuback");
		
		backgroundImage.color.set(Color.LtGrey);
		
		fullscreen = true;

		title = new MenuTitle("AUDIO");
		
		radioSignalSound = Client.registerSound("sound/misc/69310__uair01__LS100421_radio_signal_beep_859Hz_5sec_7340Khz_AM.ogg");
		
		effectsSlider = new MenuSlider("EFFECTS", 0, 1, CVars.s_volume.getValue(), 0.01f)
		{
			public void keyPressed(KeyEvent e)
			{
				super.keyPressed(e);
				
				CVars.s_volume.set(Float.toString(effectsSlider.getCurValue()));
			}
			
			@Override
			public void focusGained(FocusEvent e)
			{
				//Client.startLocalSound(radioSignalSound, SoundChannel.LOCAL);
				
				Client.stopBackgroundTrack();
				Client.addLoopingSound(Engine.ENTITYNUM_WORLD, 0, 0, 0, 0, 0, 0, radioSignalSound);
				Client.respatialize(Engine.ENTITYNUM_NONE, new Vector3f(), new Quat4f(), false);
				
				super.focusGained(e);
			}
			
			@Override
			public void focusLost(FocusEvent e)
			{
				//Client.stopLoopingSound(Engine.ENTITYNUM_NONE);
				Client.clearLoopingSounds(true);
				
				//Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/02-Parabellum.ogg", "");
				
				super.focusLost(e);
			}
		};
		
		
		musicSlider = new MenuSlider("MUSIC", 0, 1, CVars.s_musicvolume.getValue(), 0.01f)
		{
			@Override
			public void keyPressed(KeyEvent e)
			{
				super.keyPressed(e);
				
				CVars.s_musicvolume.set(Float.toString(musicSlider.getCurValue()));
			}
			
			@Override
			public void focusGained(FocusEvent e)
			{
				//Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/05-Motor.ogg", "");
				Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/02-Parabellum.ogg", "");
				
				super.focusGained(e);
			}
			
			@Override
			public void focusLost(FocusEvent e)
			{
				Client.stopBackgroundTrack();
				//Client.startBackgroundTrack("music/jamendo.com/Vate/Motor/02-Parabellum.ogg", "");
				
				super.focusLost(e);
			}
		};
		
		/*
		String[] qualityItems = {"LOW", "HIGH"};
		soundQuality = new MenuSpinControl("SOUND QUALITY", qualityItems, CVars.s_khz.getString().equals("22") ? 1 : 0)
		{
			@Override
			public void keyPressed(KeyEvent e)
			{
				super.keyPressed(e);
				
				KeyCode key = e.getKey();
				switch(key)
				{
					case ENTER:
					case XBOX360_A:
						if(soundQuality.getCurValue().equals("HIGH"))
						{
							CVars.s_khz.set("22");
						}
						else
						{
							CVars.s_khz.set("11");
						}
						
						UserInterface.forceMenuOff();
						Engine.sendConsoleCommand(Engine.EXEC_APPEND, "snd_restart\n");
						break;
				}
			}
		};
		*/
		
		String[] yesNo = {"NO", "YES"};
		useOpenAL = new MenuSpinControl("USE OPENAL", yesNo, CVars.s_useOpenAL.getBoolean() ? 1 : 0)
		{
			@Override
			public void keyPressed(KeyEvent e)
			{
				super.keyPressed(e);
				
				KeyCode key = e.getKey();
				switch(key)
				{
					case ENTER:
					case XBOX360_A:
						if(useOpenAL.getCurValue().equals("YES"))
						{
							CVars.s_useOpenAL.set("1");
						}
						else
						{
							CVars.s_useOpenAL.set("0");
						}
						
						UserInterface.forceMenuOff();
						Engine.sendConsoleCommand(Engine.EXEC_APPEND, "snd_restart\n");
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
		stackPanel.addChild(musicSlider);
		stackPanel.addChild(effectsSlider);
		//stackPanel.addChild(soundQuality);
		stackPanel.addChild(useOpenAL);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		order.add(musicSlider);
		order.add(effectsSlider);
		//order.add(soundQuality);
		order.add(useOpenAL);
		setCursorOrder(order);
		
		setCursor(musicSlider);
	}
	
	@Override
	public void keyPressed(KeyEvent e)
	{
		KeyCode key = e.getKey();
		
		switch(key)
		{
			case ESCAPE:
			case MOUSE2:
			case XBOX360_B:
				Client.stopBackgroundTrack();
				Client.clearLoopingSounds(true);
				break;
				
			case F2:
			case XBOX360_Y:
				CVars.s_volume.reset();
				effectsSlider.setCurValue(CVars.s_volume.getValue());
				
				CVars.s_musicvolume.reset();
				musicSlider.setCurValue(CVars.s_musicvolume.getValue());
				break;
		}
				
		super.keyPressed(e);
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
