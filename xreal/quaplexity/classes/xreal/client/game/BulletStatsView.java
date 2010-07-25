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

/**
 * @author Robert Beckebans
 */
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
		
		width = 200;
		
		whiteMaterial = Renderer.registerMaterialNoMip("white");
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
		
		backgroundImage = new Image("lagometer2");
		
		numDeepPenetrationChecksLabel = new Label();
		numDeepPenetrationChecksLabel.textBlock.foregroundColor = Color.Green;
		numDeepPenetrationChecksLabel.textBlock.fontSize = 8;
		
		numGjkChecksLabel = new Label();
		numGjkChecksLabel.textBlock.foregroundColor = Color.Green;
		numGjkChecksLabel.textBlock.fontSize = 8;
		
		numSplitImpulseRecoveries = new Label();
		numSplitImpulseRecoveries.textBlock.foregroundColor = Color.Green;
		numSplitImpulseRecoveries.textBlock.fontSize = 8;
		
		
		//addChild(backgroundImage);
		addChild(numDeepPenetrationChecksLabel);
		addChild(numGjkChecksLabel);
		addChild(numSplitImpulseRecoveries);
	}
	
	@Override
	public Rectangle getSize() throws Exception
	{
		//width = 200;
		/*
		horizontalAlignment = HorizontalAlignment.Left;
		verticalAlignment = VerticalAlignment.Center;
		orientation = Orientation.Vertical;
		
		numDeepPenetrationChecksLabel.height = 20;
		numDeepPenetrationChecksLabel.textBlock.fontSize = 10;
		numGjkChecksLabel.height = 20;
		numSplitImpulseRecoveries.height = 20;
		*/
		
		return super.getSize();
	}
	
	@Override
	public void render()
	{
		if(!CVars.cg_drawBulletStats.getBoolean())
		{
			return;
		}
		
		backgroundImage.setBounds(bounds);
		backgroundImage.render();
		
		numDeepPenetrationChecksLabel.text = "gNumDeepPenetrationChecks = " + BulletStats.gNumDeepPenetrationChecks;
		numGjkChecksLabel.text = "gNumGjkChecks = " + BulletStats.gNumGjkChecks;
		numSplitImpulseRecoveries.text = "gNumSplitImpulseRecoveries = " + BulletStats.gNumSplitImpulseRecoveries;

		super.render();
	}
}
