package xreal.client.game;

import com.bulletphysics.BulletStats;

import xreal.CVars;
import xreal.Color;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.Rectangle;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;

public class BulletStatsView extends StackPanel
{
	private int		whiteMaterial;
	private Font	font;
	private Image	backgroundImage;
	
	private Label	numDeepPenetrationChecksLabel;
	private Label	numGjkChecksLabel;
	private Label	numSplitImpulseRecoveries;
	
	public BulletStatsView() throws Exception
	{
		horizontalAlignment = HorizontalAlignment.Left;
		verticalAlignment = VerticalAlignment.Center;
		
		width = 100;
		
		whiteMaterial = Renderer.registerMaterialNoMip("white");
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
		
		backgroundImage = new Image("lagometer2");
		
		numDeepPenetrationChecksLabel = new Label();
		numDeepPenetrationChecksLabel.foregroundColor = Color.Green;
		
		numGjkChecksLabel = new Label();
		numGjkChecksLabel.foregroundColor = Color.Green;
		
		numSplitImpulseRecoveries = new Label();
		numSplitImpulseRecoveries.foregroundColor = Color.Green;
		
		//addChild(backgroundImage);
		addChild(numDeepPenetrationChecksLabel);
		addChild(numGjkChecksLabel);
		addChild(numSplitImpulseRecoveries);
	}
	
	@Override
	public Rectangle getBounds() throws Exception
	{
		width = 200;
		
		Rectangle rect = super.getBounds();
		
		try
		{
			backgroundImage.setBounds(rect);
		}
		catch(Exception e)
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return rect;
	}
	
	@Override
	public void render()
	{
		if(!CVars.cg_drawBulletStats.getBoolean())
		{
			return;
		}
		
		backgroundImage.render();
		
		numDeepPenetrationChecksLabel.text = "gNumDeepPenetrationChecks = " + BulletStats.gNumDeepPenetrationChecks;
		numGjkChecksLabel.text = "gNumGjkChecks = " + BulletStats.gNumGjkChecks;
		numSplitImpulseRecoveries.text = "gNumSplitImpulseRecoveries = " + BulletStats.gNumSplitImpulseRecoveries;

		super.render();
	}
}
