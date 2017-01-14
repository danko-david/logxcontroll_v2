package eu.logxcontroll.java;

import eu.logxcontroll.Gate;
import eu.logxcontroll.LxcValue;
import eu.logxcontroll.Signal;
import eu.logxcontroll.Wire;

public abstract class LogxControllDebugCallback
{
	public static enum LxcWireOperationPhase
	{
		lxc_before_wire_driven,

		lxc_before_gate_notified,
		lxc_after_gate_notified,

		lxc_after_wire_driven,
	}
	
	protected final void callback(int op_phase, long wire, long gate, int index, long value, long signal, int subtype)
	{
		/*void (*wire_debug_hook)
		(
			struct lxc_wire_debug_hook_data* data,
			enum lxc_wire_operation_phase,
			Wire this_wire,
			Gate subject_gate,
			uint subject_port_index,
			LxcValue value,
			Signal type,
			int subtype
		);*/
		
		try
		{
			LxcWireOperationPhase phase = LxcWireOperationPhase.values()[op_phase];
			Wire w = Wire.wireFromPointer(wire);
			Gate g = Gate.gateFromPointer(gate);
			Signal s = Signal.signalFromPointer(signal);
			LxcValue val = LxcValue.valueFromPointer(value);
		
			debug(phase, w, g, index, val, s, subtype);
		}
		catch(Throwable t)
		{
			t.printStackTrace();
		}
	}
	
	public abstract void debug
	(
		LxcWireOperationPhase phase,
		Wire wire,
		Gate subjectGate,
		int subjectPort,
		LxcValue value,
		Signal type,
		int subtype
	);
}
