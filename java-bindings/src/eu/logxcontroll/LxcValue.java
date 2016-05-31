package eu.logxcontroll;

public class LxcValue extends NativeObject
{
	protected LxcValue(long ptr)
	{
		super(ptr);
		LogxControll.referenceValue(this);
	}
	
	@Override
	public void finalize()
	{
		LogxControll.unreferenceValue(this);
	}
	
	public static void assertValid(LxcValue v)
	{
		//if v is null, NullPointerException is automatically thrown.
		if(0 == v.ptr) 
		{
			throw new NullPointerException("LxcValue's native pointer is NULL!");
		}
	}
	
	public static LxcValue valueFromPointer(long ptr)
	{
		if(0 == ptr)
		{
			return null;
		}
		
		return new LxcValue(ptr);
	}
	
	public static long pointerFromValue(LxcValue val)
	{
		if(null == val)
		{
			return 0;
		}
		
		return val.ptr;
	}
	
	public long getData()
	{
		return LogxControll.getValue(this);
	}
	
	public Signal getType()
	{
		return LogxControll.getLxcValueSignalType(this);
	}
	
	public static LxcValue createGenericValue(Signal type, int size_in_bytes)
	{
		return LogxControll.createGenericValue(type, size_in_bytes);
	}
	
	public String toString()
	{
		Signal sig = getType();
		String type = sig == null? "NULL":sig.getName();
		return "struct lxc_value `LxcValue` {"+type+"} ["+Long.toHexString(ptr)+"]";
	}
	
}
