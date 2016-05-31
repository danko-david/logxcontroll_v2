package eu.logxcontroll.java;

import eu.logxcontroll.Gate;
import eu.logxcontroll.LxcValue;
import eu.logxcontroll.Signal;

public abstract class LogxControllCallback
{
	protected final void callback(long gate, long signal, int input, long value)
	{
		Gate g = Gate.gateFromPointer(gate);
		Signal s = Signal.signalFromPointer(signal);
		LxcValue val = LxcValue.valueFromPointer(value);
		
		execute(g, s, input, val);
	}
	
	public abstract void execute(Gate gate, Signal type, int input, LxcValue value);
}
