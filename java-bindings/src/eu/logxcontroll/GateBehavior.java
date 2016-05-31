package eu.logxcontroll;

public class GateBehavior extends NativeObject
{
	GateBehavior(long ptr)
	{
		super(ptr);
	}

	public String getName()
	{
		return LogxControll.lxcBehaviorGetName(ptr);
	}
	
	public Gate newInstance()
	{
		return Gate.gateFromPointer(LogxControll.lxcBehaviorNewInstance(ptr));
	}
}