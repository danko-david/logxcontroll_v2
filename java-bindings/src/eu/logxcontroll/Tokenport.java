package eu.logxcontroll;

public class Tokenport extends NativeObject
{
	protected Tokenport(long ptr)
	{
		super(ptr);
	}

	public static Object tokenPortFromPointer(long ptr)
	{
		if(0 == ptr)
		{
			return null;
		}
		
		return new Tokenport(ptr);
	}
	
	//get_value
	//absorb_value
	
	public void absorbToken()
	{
		LogxControll.lxc_absorb_token(this);
	}

	public LxcValue getTokenValue()
	{
		return LogxControll.lxc_get_token_value(this);
	}
}
