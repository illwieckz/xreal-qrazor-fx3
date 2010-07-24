package xreal.client.ui;


/**
 * @author Robert Beckebans
 */
public class StackPanel extends Component
{
	public enum Orientation
	{
		Vertical, Horizontal
	}

	public Orientation	orientation = Orientation.Vertical;
	
	@Override
	public Rectangle getBounds() throws Exception
	{
		alignChildrenAndUpdateBounds();
		
		return super.getBounds();
	}
	
	@Override
	protected void alignChildrenAndUpdateBounds()
	{
		float x = bounds.x;
		float y = bounds.y;
		
		for(Component c : children)
		{
			Rectangle rect;
			try
			{
				rect = c.getBounds();
			}
			catch(Exception e)
			{
				c.active = false;
				e.printStackTrace();
				continue;
			}
			
			// TODO use margin
			switch(orientation)
			{
				case Vertical:
					c.bounds.y = y;
					y += rect.height + c.margin.top + c.margin.bottom;
					
					if(rect.width + c.margin.left + c.margin.right > x)
					{
						x = rect.width + c.margin.left + c.margin.right;
					}
					break;
					
				case Horizontal:
					c.bounds.x = x;
					x += rect.width;
					break;
			}
		}
		
		if(width != 0)
		{
			bounds.width = width;
		}
		else
		{
			bounds.width = Math.abs(x - bounds.x);
		}
		
		if(height != 0)
		{
			bounds.height = height;
		}
		else
		{
			bounds.height = Math.abs(y - bounds.y);
		}
	}
}
