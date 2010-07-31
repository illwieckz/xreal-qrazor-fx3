package xreal.client.ui;

import java.util.Vector;

/**
 * @author Robert Beckebans
 */
public class LinearFocusTraversalPolicy implements FocusTraversalPolicy
{
	private Vector<Component>	order;
	
	public LinearFocusTraversalPolicy(Vector<Component> order)
	{
		this.order = new Vector<Component>(order.size());
		this.order.addAll(order);
	}
	
	@Override
	public Component getComponentAfter(Component container, Component component)
	{
		int index = (order.indexOf(component) + 1) % order.size();
		
		return order.get(index);
	}

	@Override
	public Component getComponentBefore(Component container, Component component)
	{
		int index = order.indexOf(component) -1;
		if(index < 0)
		{
			index = order.size() - 1;
		}
		
		return order.get(index);
	}

	@Override
	public Component getDefaultComponent(Component container)
	{
		return order.get(0);
	}

	@Override
	public Component getFirstComponent(Component container)
	{
		return order.get(0);
	}

	@Override
	public Component getLastComponent(Component container)
	{
		return order.lastElement();
	}

}
