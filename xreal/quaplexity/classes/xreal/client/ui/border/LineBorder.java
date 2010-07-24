package xreal.client.ui.border;

import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;

public class LineBorder extends Border {

	private int whiteMaterial = Renderer.registerMaterialNoMip("white");
	
	protected float thickness;
	protected boolean roundedCorners;
	
	public LineBorder(Color lineColor) {
		super();
		this.borderColor = lineColor;
		this.thickness = 1;
		this.roundedCorners = false;
	}
	
	public LineBorder(Color lineColor, float thickness) {
		super();
		this.borderColor = lineColor;
		this.thickness = thickness;
		this.roundedCorners = false;
	}
	
	public LineBorder(Color lineColor, float thickness, boolean roundedCorners) {
		super();
		this.borderColor = lineColor;
		this.thickness = thickness;
		this.roundedCorners = roundedCorners;
	}

	public void paintBorder(float x, float y, float width, float height) {
		
		Renderer.setColor(borderColor);
		
		Rectangle rect = new Rectangle(x, y, width, height);
		UserInterface.adjustFrom640(rect);

		Renderer.drawStretchPic(rect.x, rect.y, rect.width, 1, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x, rect.y, 1, rect.height, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x, rect.y + rect.height - 1, rect.width, 1, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x + rect.width - 1, rect.y, 1, rect.height, 0, 0, 0, 0, whiteMaterial);

		Renderer.setColor(Color.White);
	}
	
	public void paintBorder(Component c, float x, float y, float width, float height) {
		
		Renderer.setColor(borderColor);
		
		Rectangle rect = new Rectangle(x, y, width, height);
		rect.setLocation(c.getX(), c.getY());
		UserInterface.adjustFrom640(rect);

		Renderer.drawStretchPic(rect.x, rect.y, rect.width, 1, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x, rect.y, 1, rect.height, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x, rect.y + rect.height - 1, rect.width, 1, 0, 0, 0, 0, whiteMaterial);
		Renderer.drawStretchPic(rect.x + rect.width - 1, rect.y, 1, rect.height, 0, 0, 0, 0, whiteMaterial);

		Renderer.setColor(Color.White);
	}
}
