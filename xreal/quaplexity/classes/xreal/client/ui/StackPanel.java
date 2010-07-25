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
	public Rectangle getSize() throws Exception
	{
		bounds.x = 0;
		bounds.y = 0;
		
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
				rect = c.getSize();
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
					c.bounds.x = bounds.x;
					c.bounds.y = y + c.margin.top;
					y += rect.height + c.margin.top + c.margin.bottom;
					
					if((rect.width + c.margin.left + c.margin.right) > x)
					{
						x = rect.width + c.margin.left + c.margin.right;
					}
					break;
					
				case Horizontal:
					c.bounds.x = x + c.margin.left;
					x += rect.width + c.margin.left + c.margin.right;
					break;
			}
			
			c.alignChildrenAndUpdateBounds();
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
