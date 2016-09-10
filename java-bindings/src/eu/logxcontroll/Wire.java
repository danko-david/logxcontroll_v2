package eu.logxcontroll;

public class Wire extends NativeObject
{
	protected Wire(long ptr)
	{
		super(ptr);
	}
	
	public static void assertValid(Wire w)
	{
		//if w is null, NullPointerException is automatically thrown.
		if(0 == w.ptr) 
		{
			throw new NullPointerException("Wire's native pointer is NULL!");
		}
	}
	
	public static Wire wireFromPointer(long ptr)
	{
		if(0 == ptr)
		{
			return null;
		}
		
		return new Wire(ptr);
	}
	
	public static long pointerFromWire(Wire w)
	{
		if(null == w)
		{
			return 0;
		}
		
		return w.ptr;
	}

	public Signal getType()
	{
		return LogxControll.getWireSignal(this);
	}
	
/*	So it's now a complex thing, wire can be token managed	
	public LxcValue getValue() 
	{
		return LogxControll.getWireValue(this);
	}
*/	
	public void drive(Gate g, int out_index, LxcValue value)
	{
		LogxControll.driveWireValue(g, out_index, this, value);
	}
	
	
	public String toString()
	{
		Signal sig = getType();
		String type = sig == null? "NULL":sig.getName();
		return "struct lxc_wire `Wire` {"+type+"} ["+Long.toHexString(ptr)+"]";
	}

	public int getSubtype()
	{
		return LogxControll.getWireSubtype(this);
	}
}