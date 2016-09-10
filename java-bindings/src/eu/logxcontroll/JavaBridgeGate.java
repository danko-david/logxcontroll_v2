package eu.logxcontroll;

import java.util.ArrayList;

import eu.javaexperience.nativ.posix.CString;
import eu.logxcontroll.Gate;
import eu.logxcontroll.LogxControll;
import eu.logxcontroll.PortManager;
import eu.logxcontroll.Signal;
import eu.logxcontroll.java.LogxControllCallback;

public class JavaBridgeGate extends Gate
{
	protected final PortManager inputPorts;
	protected final PortManager outputPorts;
	
	protected JavaBridgeGate(long ptr)
	{
		super(ptr);
		inputPorts = PortManager.getFromPortManagerInInstance(this, LogxControll.DIRECTION_IN);
		outputPorts = PortManager.getFromPortManagerInInstance(this, LogxControll.DIRECTION_OUT);
	}
	
	public static JavaBridgeGate newInstance()
	{
		Gate ret = LogxControll.newInstanceByName("java bridge");
		if(null == ret)
		{
			throw new RuntimeException("java bridge gate library not loaded");
		}
		
		return new JavaBridgeGate(ret.ptr);
	}
	
	protected ArrayList<LogxControllCallback> cbs = new ArrayList<>();
	
	protected ArrayList<CString> inputLabels = new ArrayList<>();
	
	protected ArrayList<CString> outputLabels = new ArrayList<>();
	
	protected void setCb(LogxControllCallback cb, int index)
	{
		int diff = index - cbs.size() +1;
		for(int i=0;i<diff;++i)
			cbs.add(null);
		
		cbs.set(index, cb);
	}
	
	public int addInputPort(Signal sig, int subtype, String name, boolean sensitive, LogxControllCallback cb)
	{
		inputPorts.assertPortnameNotInUse(name);
		//int prev = inputPorts.getMaxIndex(); 
		CString nn = new CString(name);
		inputLabels.add(nn);
		//int index = inputPorts.addNewPort(sig, name, sensitive);
		
		
		PortManager.assertPortName(name);
		Signal.assertValid(sig);
		LogxControll.lxcPortUncheckedAddNewPort1(inputPorts.ptr, nn.getPointer(), sig.ptr, subtype);
		int index = inputPorts.assertSuccessfullyRegistered(name);
		
		
		
		//int next = inputPorts.getMaxIndex();
		/*if(prev != next)
		{
			LogxControll.lxcPortiReallocIOPort(ptr, LogxControll.DIRECTION_IN, prev, next);
		}*/
		
		LogxControll.javaBridgeCallback(this, true, index, cb);
		setCb(cb, index);
		return index;
	}
	
	public int addOutputPort(Signal sig, int subtype, String name)
	{
		PortManager.assertPortName(name);
		Signal.assertValid(sig);
		int prev = outputPorts.getMaxIndex(); 
		CString nn = new CString(name);
		outputLabels.add(nn);
		
		
		PortManager.assertPortName(name);
		Signal.assertValid(sig);
		LogxControll.lxcPortUncheckedAddNewPort1(outputPorts.ptr, nn.getPointer(), sig.ptr, subtype);
		int index = outputPorts.assertSuccessfullyRegistered(name);
		
		//int index = outputPorts.addNewPort(sig, name, false);
		/*int next = outputPorts.getMaxIndex();
		if(prev != next)
		{
			LogxControll.lxcPortiReallocIOPort(ptr, LogxControll.DIRECTION_OUT, prev, next);
		}*/
		return index;
	}


}
