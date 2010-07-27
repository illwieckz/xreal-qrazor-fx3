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
		float w = 0;
		float h = 0;
		
		/*
		if(width == 0 || height == 0)
		{
			// we need to calculate the size of this component
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
				
				// add margin
				rect.width += c.margin.left + c.margin.right;
				rect.height += c.margin.top + c.margin.bottom;
				
				if(rect.width > w)
				{
					w = rect.width;
				}
				
				if(rect.height > h)
				{
					h = rect.height;
				}
			}
			
			if(width == 0 && children.size() > 0)
			{
				bounds.width = w;
			}
			
			if(height == 0 && children.size() > 0)
			{
				bounds.height = h;
			}
		}
		
		if(width != 0)
		{
			bounds.width = width;
		}
		
		if(height != 0)
		{
			bounds.height = height;
		}
		*/
		
		for(Component c : children)
		{
			Rectangle rect;
			try
			{
				rect = c.getSizeWithMargin();
			}
			catch(Exception e)
			{
				c.active = false;
				//e.printStackTrace();
				continue;
			}
			
			switch(orientation)
			{
				case Vertical:
					c.bounds.x = bounds.x + c.margin.left;
					c.bounds.y = y + c.margin.top;
					
					c.bounds.width = rect.width;
					c.bounds.height = rect.height;
					
					y += rect.height;
					h += rect.height;
					
					if(rect.width > w)
					{
						w = rect.width;
					}
					break;
					
				case Horizontal:
					c.bounds.x = x + c.margin.left;
					c.bounds.y = bounds.y + c.margin.top;
					
					c.bounds.width = rect.width;
					c.bounds.height = rect.height;
					
					x += rect.width;
					w += rect.width;
					
					if(rect.height > h)
					{
						h = rect.height;
					}
					break;
			}
			
			c.alignChildrenAndUpdateBounds();
		}
		
		if(width != 0)
		{
			bounds.width = width;
		}
		/*
		else if(horizontalAlignment == HorizontalAlignment.Stretch && parent != null)
		{
			// stretch to parent width
			bounds.width = parent.bounds.width;
		}
		*/
		else if(children.size() > 0)
		{
			// only as big as children require it
			bounds.width = w;
		}
		
		if(height != 0)
		{
			bounds.height = height;
		}
		else if(children.size() > 0)
		{
			bounds.height = h;
		}
	}
}
