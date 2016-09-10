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
	
	public FullSignalType[] getInputTypes()
	{
		return LogxControll.getGateInputTypes(this);
	}
	
	public FullSignalType[] getOutputTypes()
	{
		return LogxControll.getGateOutputTypes(this);
	}
	
	public String getInputLabel(Signal sig, int subtype, int index)
	{
		return LogxControll.getInputLabel(this, sig, subtype, index);
	}

	public String getOutputLabel(Signal sig, int subtype, int index)
	{
		return LogxControll.getOutputLabel(this, sig, subtype, index);
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
		LogxControll.wireGateInput(null, w.getSubtype(), w, this, index);
	}
	
	public void unwireInput(Signal sig, int index) throws LogxControllException
	{
		LogxControll.wireGateInput(sig, 0, null, this, index);
	}
	
	public void wireOutput(Wire w, int index) throws LogxControllException
	{
		LogxControll.wireGateOutput(null, w.getSubtype(), w, this, index);
	}
	
	public void unwireOutput(Signal sig, int subtype, int index) throws LogxControllException
	{
		LogxControll.wireGateOutput(sig, subtype, null,  this, index);
	}
	
	public Tokenport getInputWire(Signal type, int subtype, int index)
	{
		return LogxControll.getInputWire(this, type, subtype, index);
	}
	
	public Wire getOutputWire(Signal type, int subtype, int index)
	{
		return LogxControll.getOutputWire(this, type, subtype, index);
	}
	
	public int getInputMaxIndex(Signal s, int subtype)
	{
		return LogxControll.getGateInputMaxIndex(this, s, subtype);
	}
	public int getOutputMaxIndex(Signal s, int subtype)
	{
		return LogxControll.getGateOutputMaxIndex(this, s, subtype);
	}
	
	public String toString()
	{
		return "struct lxc_gate_instance `Gate` ("+getName()+") ["+Long.toHexString(ptr)+"]";
	}
	
	protected void printPorts(boolean direction)
	{
		System.out.println(direction?"\tInput ports:":"\tOutput ports:");
		FullSignalType[] types = 	direction?
								getInputTypes()
							:
								getOutputTypes();
		for(FullSignalType s:types)
		{
			int max = 	direction?
							getInputMaxIndex(s.getSignal(), s.getSubtype())
						:
							getOutputMaxIndex(s.getSignal(), s.getSubtype());
			if(max > 0)
			{
				System.out.println("\t\t"+s);
				for(int i=0;i<max;++i)
				{
					String label = 	direction?
									getInputLabel(s.getSignal(), s.getSubtype(), i)
								:
									getOutputLabel(s.getSignal(), s.getSubtype(), i);
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