package eu.logxcontroll;

public class NativeObject
{
	public final long ptr;
	
	protected NativeObject(long ptr)
	{
		this.ptr = ptr;
	}
}