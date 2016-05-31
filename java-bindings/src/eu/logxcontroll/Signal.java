package eu.logxcontroll;

public class Signal extends NativeObject
{
	protected Signal(long ptr)
	{
		super(ptr);
	}
	
	public static void assertValid(Signal s)
	{
		//if s is null, NullPointerException is automatically thrown.
		if(0 == s.ptr) 
		{
			throw new NullPointerException("Signal's native pointer is NULL!");
		}
	}
	
	public static Signal signalFromPointer(long ptr)
	{
		if(0 == ptr)
		{
			return null;
		}
		
		return new Signal(ptr);
	}
	
	public static long pointerFromSignal(Signal sig)
	{
		if(null == sig)
		{
			return 0;
		}
		
		return sig.ptr;
	}
	
	public String getName()
	{
		return LogxControll.getSignalName(this);
	}
	
	//TODO other attributes
	
	public String toString()
	{
		return "struct lxc_signal_type `Signal` ("+getName()+") ["+Long.toHexString(ptr)+"]";
	}
	
}
