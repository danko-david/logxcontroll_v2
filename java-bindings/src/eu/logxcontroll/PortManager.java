package eu.logxcontroll;

public class PortManager extends NativeObject
{
	protected PortManager(long ptr)
	{
		super(ptr);
	}
	
	public static PortManager getFromPortManagerInInstance(Gate g, boolean direction)
	{
		assertInstanceOfPortInInstance(g);
		return new PortManager(LogxControll.lxcPortiGetPortManager(g.ptr, direction));
	}
	
	public static void assertInstanceOfPortInInstance(Gate g)
	{
		Gate.assertValid(g);
		if(!LogxControll.lxcCheckPortiInsance(g.ptr))
		{
			throw new RuntimeException("Gate is looks like not an `port manager in instance` type.");
		}
	}
	
	public static void assertPortName(String name)
	{
		if(null == name)
		{
			throw new RuntimeException("Port name may not null");
		}
	}
	
	public void assertPortnameNotInUse(String portname)
	{
		assertPortName(portname);
		if(LogxControll.lxcPortCheckPortnameInUse(ptr, portname))
		{
			throw new RuntimeException("Port `"+portname+"` is already in use");
		}
	}
	
	public int getAbsIndex(Signal sig, int subtype, int index)
	{
		Signal.assertValid(sig);
		return LogxControll.lxcPortGetAbsIndex(ptr, sig.ptr, subtype, index);
	}
	
	public int getAbsIndexByPortName(String name)
	{
		if(null == name)
		{
			throw new NullPointerException("Port name may not null");
		}
		
		return LogxControll.lxcPortGetAbsindexByName(ptr, name);
	}

	public int assertSuccessfullyRegistered(String name)
	{
		assertPortName(name);
		int index = getAbsIndexByPortName(name);
		if(index < 0)
		{
			throw new RuntimeException("Native error: newly registered bort abs index is negative `"+index+"`.");
		}
		
		return index;
	}

	public int getMaxIndex()
	{
		return LogxControll.lxcPortManagerMaxIndex(ptr);
	}
	
/*	public int addNewPort(Signal sig, String name, boolean sensitive)
	{
		assertPortName(name);
		Signal.assertValid(sig);
		LogxControll.lxcPortUncheckedAddNewPort1(ptr, name, sig.ptr, sensitive);
		return assertSuccessfullyRegistered(name);
	}*/
}