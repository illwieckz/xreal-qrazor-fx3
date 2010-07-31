package xreal.client.ui;


/**
 * @author Robert Beckebans
 */
public interface FocusTraversalPolicy
{
	Component getComponentAfter(Component container, Component component);
	
	Component getComponentBefore(Component container, Component component);
	
	Component getDefaultComponent(Component container);
	
	Component getFirstComponent(Component container);
	
	Component getLastComponent(Component container);
}
