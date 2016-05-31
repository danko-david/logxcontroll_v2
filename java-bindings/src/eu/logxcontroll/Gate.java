package eu.logxcontroll;

public class Gate extends NativeObject
{
	protected Gate(long ptr)
	{
		super(ptr);
	}
	
	public static void assertValid(Gate g)
	{
		//if g is null, NullPointerException is automatically thrown.
		if(0 == g.ptr) 
		{
			throw new NullPointerException("Gate's native pointer is NULL!");
		}
	}
	
	public static Gate gateFromPointer(long ptr)
	{
		if(0 == ptr)
		{
			return null;
		}
		
		return new Gate(ptr);
	}

	public static long pointerFromGate(Gate g)
	{
		if(null == g)
		{
			return 0;
		}
		
		return g.ptr;
	}
	
	public static Gate newGateByName(String name)
	{
		return LogxControll.newInstanceByName(name);
	}
	
	public String getName()
	{
		return LogxControll.getGatename(this);
	}
	
	public boolean isEnabled()
	{
		return LogxControll.lxcGateEnable(ptr, false, false);
	}
	
	public void setEnabled(boolean enabled)
	{
		LogxControll.lxcGateEnable(ptr, true, enabled);
	}
	
	public Signal[] getInputTypes()
	{
		return LogxControll.getGateInputTypes(this);
	}
	
	public Signal[] getOutputTypes()
	{
		return LogxControll.getGateOutputTypes(this);
	}
	
	public String getInputLabel(Signal sig, int index)
	{
		return LogxControll.getInputLabel(this, sig, index);
	}

	public String getOutputLabel(Signal sig, int index)
	{
		return LogxControll.getOutputLabel(this, sig, index);
	}

	public String[] getProperties()
	{
		return LogxControll.enumerateProperties(this);
	}
	
	public String getPropertyLabel(String propname)
	{
		return LogxControll.getPropertyLabel(this, propname);
	}

	public String getPropertyDescription(String propname)
	{
		return LogxControll.getPropertyDescription(this, propname);
	}
	
	public String getPropertyValue(String propname)
	{
		return LogxControll.getPropertyValue(this, propname);
	}
	
	public void setPropertyValue(String propname, String value) throws LogxControllException
	{
		LogxControll.setPropertyValue(this, propname, value);
	}
	
	public void wireInput(Wire w, int index) throws LogxControllException
	{
		LogxControll.wireGateInput(null, w, this, index);
	}
	
	public void unwireInput(Signal sig, int index) throws LogxControllException
	{
		LogxControll.wireGateInput(sig, null, this, index);
	}
	
	public void wireOutput(Wire w, int index) throws LogxControllException
	{
		LogxControll.wireGateOutput(null, w, this, index);
	}
	
	public void unwireOutput(Signal sig, int index) throws LogxControllException
	{
		LogxControll.wireGateOutput(sig, null, this, index);
	}
	
	public Tokenport getInputWire(Signal type, int index)
	{
		return LogxControll.getInputWire(this, type, index);
	}
	
	public Wire getOutputWire(Signal type, int index)
	{
		return LogxControll.getOutputWire(this, type, index);
	}
	
	public int getInputMaxIndex(Signal s)
	{
		return LogxControll.getGateInputMaxIndex(this, s);
	}
	public int getOutputMaxIndex(Signal s)
	{
		return LogxControll.getGateOutputMaxIndex(this, s);
	}
	
	public String toString()
	{
		return "struct lxc_gate_instance `Gate` ("+getName()+") ["+Long.toHexString(ptr)+"]";
	}
	
	protected void printPorts(boolean direction)
	{
		System.out.println(direction?"\tInput ports:":"\tOutput ports:");
		Signal[] types = 	direction?
								getInputTypes()
							:
								getOutputTypes();
		for(Signal s:types)
		{
			int max = 	direction?
							getInputMaxIndex(s)
						:
							getOutputMaxIndex(s);
			if(max > 0)
			{
				System.out.println("\t\t"+s);
				for(int i=0;i<max;++i)
				{
					String label = 	direction?
									getInputLabel(s, i)
								:
									getOutputLabel(s, i);
					System.out.println("\t\t\t"+label);
				}
			}
		}
	}
	
	protected void printProps()
	{
		String[] props = getProperties();
		for(String prop:props)
		{
			System.out.println("\t"+prop+":\t"+getPropertyValue(prop));
		}
	}
	
	public void dbgPrintAll()
	{
		System.out.println(getName()+" {enabled: "+isEnabled()+"}");
		printPorts(true);
		printPorts(false);
		printProps();
	}
}