package eu.logxcontroll;

import java.util.Arrays;

import eu.javaexperience.asserts.AssertArgument;
import eu.javaexperience.gnu.GccTools;
import eu.javaexperience.nativ.posix.Posix;
import eu.javaexperience.semantic.references.MayNull;
import eu.logxcontroll.java.LogxControllCallback;
import eu.logxcontroll.java.LogxControllDebugCallback;

public class LogxControll
{

/*********************** Initializing native envirnment ************************
 * System initialized from java side on ClassLoading time.
 * The loaded shared library symbols loaded globally (see: dlopen RTLD_GLOBAL)
 * 	so loading different version of LXC in the same java process is currently
 * 	not available (even if you use custom separated ClassLoaders) 
 * 
 * TODO
 * 
 * 
 */
	static
	{
		System.out.println("logxcontroll library: "+System.getProperty("eu.logxcontroll.lib"));
		long ptr = Posix.dlopen(System.getProperty("eu.logxcontroll.lib"), Posix.RTLD_NOW | Posix.RTLD_GLOBAL);

		if(0 == ptr)
		{
			System.out.println("dlerror(): "+Posix.dlerror());
		}
		else
		{
			System.out.println("dlopen(): "+Long.toHexString(ptr));
		}
		
		
		boolean loaded =
			GccTools.loadArchDependSharedJavaLibraryOrCompileAndLoad
			(
				"eu/logxcontroll",
				"LogxControll",
				new String[]{"util"},
				null,
				new String[]
					{
						"-fpermissive",
						"-pthread",
						//"-std=c99",
						"-I/home/szupervigyor/projektek/LogxKontroll/WS/repository/"
					},
				System.out,
				System.err
			);
		
		if(!loaded)
			throw new UnsatisfiedLinkError("LogxControll shared library not found!");
		
		LogxControllCallback.class.toString();
		NativeObject.class.toString();
		
		initNative();
	}

	protected static native void initNative();

	protected static native int loadLogxControllSharedLibrary(String so, String[] errors);

	public static String[] loadSharedLibrary(String soPath)
	{
		String[] errors = new String[30];
		loadLogxControllSharedLibrary(soPath, errors);
		LxcLibraryTreeNode.refresh();
		
		int maxindex = 0;
		for(int i=0;i<errors.length;++i)
		{
			if(null == errors[i])
			{
				maxindex = i;
				break;
			}
		}

		return Arrays.copyOf(errors, maxindex);
	}
	
	/**
	 * Used to specify port direction (input/output) respectively to the native
	 * lxc-core. 
	 * */
	public static final boolean DIRECTION_IN = true;
	public static final boolean DIRECTION_OUT = false;
	
	protected static native int lxcValRefDiff(long value, int ref_diff);
	
	public static int referenceValue(LxcValue value)
	{
		if(null == value || 0 == value.ptr)
		{
			return 0;
		}
		
		return lxcValRefDiff(value.ptr, 1);
	}
	
	public static int unreferenceValue(LxcValue value)
	{
		if(null == value || 0 == value.ptr)
		{
			return 0;
		}
		
		return lxcValRefDiff(value.ptr, -1);
	}
	
/*	protected static native long lxcGetWireValue(long wire);

	public static LxcValue getWireValue(Wire wire)
	{
		if(null == wire || 0 == wire.ptr)
		{
			return null;
		}
		
		return LxcValue.valueFromPointer(lxcGetWireValue(wire.ptr));
	}
*/
	protected static native long lxcGetValue(long value);
	
	public static long getValue(LxcValue val)
	{
		if(null == val || 0 == val.ptr)
		{
			return 0;
		}
		
		return lxcGetValue(val.ptr);
	}

	protected static native long lxcCreateGenericValue(long signal, int size);
	
	public static LxcValue createGenericValue(Signal sig, int size)
	{
		if(null == sig || 0 == sig.ptr || size <= 0)
		{
			return null;
		}
		
		return LxcValue.valueFromPointer(lxcCreateGenericValue(sig.ptr, size));
	}


	protected static native long lxcGetConstantByName(String name);
	
	public static LxcValue getConstantByName(String name)
	{
		if(null == name)
		{
			return null;
		}
		
		return LxcValue.valueFromPointer(lxcGetConstantByName(name));
	}

	protected static native long lxcGetValueType(long value);
	
	public static Signal getLxcValueSignalType(LxcValue v)
	{
		LxcValue.assertValid(v);
		return Signal.signalFromPointer(lxcGetValueType(v.ptr));
	}
	
	
/******************** Signal ASSOCIATED FACET FUNCTION ************************/
	protected static native long lxcGetSignalByName(String name);
	
	public static Signal getSignalByName(String name)
	{
		if(null == name)
		{
			return null;
		}
		
		return Signal.signalFromPointer(lxcGetSignalByName(name));
	}
	
/******************* Wire/Wiring ASSOCIATED FACET FUNCTION ********************/

	protected static native void lxcDriveWireValue(long instance, int outIndex, long wire, long value);
	
	public static void driveWireValue(Gate driver,int out_index, Wire wire, LxcValue value)
	{
		if(null == driver || 0 == driver.ptr || out_index < 0 || null == wire ||0 ==  wire.ptr)
		{
			return;
		}
		
		lxcDriveWireValue(driver.ptr, out_index, wire.ptr, LxcValue.pointerFromValue(value));
	}

	protected static native int lxcWireGateInput(long signal, int subtype, long wire, long gate, int index);
	
	//TODO wrap  to more specific type
	public static void wireGateInput(Signal type, int subtype, Wire wire, Gate g, int index) throws LogxControllException
	{
		int errno =	lxcWireGateInput
					(
						Signal.pointerFromSignal(type),
						subtype,
						Wire.pointerFromWire(wire),
						Gate.pointerFromGate(g),
						index
					);
		
		if(0 != errno)
		{
			throw new LogxControllException(errno);
		}
	}

	protected static native int lxcWireGateOutput(long type, int subtype, long wire, long g, int index);
	
	protected static void wireGateOutput(Signal type, int subtype, Wire wire, Gate g, int index) throws LogxControllException
	{
		int errno =	lxcWireGateOutput
				(
					Signal.pointerFromSignal(type),
					subtype,
					Wire.pointerFromWire(wire),
					Gate.pointerFromGate(g),
					index
				);
	
		if(0 != errno)
			throw new LogxControllException(errno);
	}

	protected static native long lxcCreateWire(long type);
	
	public static Wire createWire(Signal type)
	{
		return Wire.wireFromPointer(lxcCreateWire(Signal.pointerFromSignal(type)));
	}

/*************************** Gate basic functions *****************************/

	protected static native long lxcNewInstanceByName(String name);
	
	public static Gate newInstanceByName(String name)
	{
		if(null == name)
		{
			return null;
		}
		
		return Gate.gateFromPointer(lxcNewInstanceByName(name));
	}

	static native boolean lxcGateEnable(long gate, boolean set, boolean enable);
	
	protected static native String lxcGetGatename(long gate);
	
	public static String getGatename(Gate gate)
	{
		Gate.assertValid(gate);
		return lxcGetGatename(gate.ptr);
	}
	
	protected static native String lxcGetGateReferenceDesignator(long gate);
	
	protected static native void lxcSetGateReferenceDesignator(long gate, String name);
	
	public static String getGateReferenceDesignator(Gate gate)
	{
		Gate.assertValid(gate);
		long ptr = Gate.pointerFromGate(gate);
		return lxcGetGateReferenceDesignator(ptr);
	}
	
	public static void setGateReferenceDesignator(Gate gate, @MayNull String name)
	{
		Gate.assertValid(gate);
		long ptr = Gate.pointerFromGate(gate);
		lxcSetGateReferenceDesignator(ptr, name);
	}
	
	protected static native long[] lxcGetGateIOTypes(long gate, boolean direction);
	
	protected static FullSignalType[] getGateIOTypes(Gate g, boolean direction)
	{
		Gate.assertValid(g);
		
		long[] arr = lxcGetGateIOTypes(g.ptr, direction);
		
		return FullSignalType.fetch(arr);
	}

	public static FullSignalType[] getGateInputTypes(Gate g)
	{
		return getGateIOTypes(g, DIRECTION_IN);
	}

	public static FullSignalType[] getGateOutputTypes(Gate g)
	{
		return getGateIOTypes(g, DIRECTION_OUT);
	}


	private static native int lxcGetPortMaxIndex(long gate, long singal, int subtype, boolean direction);
	
	public static int getGateInputMaxIndex(Gate gate, Signal s, int subtype)
	{
		Gate.assertValid(gate);
		Signal.assertValid(s);
		
		return lxcGetPortMaxIndex(gate.ptr, s.ptr, subtype, DIRECTION_IN);
	}

	public static int getGateOutputMaxIndex(Gate gate, Signal s, int subtype)
	{
		Gate.assertValid(gate);
		Signal.assertValid(s);
		
		return lxcGetPortMaxIndex(gate.ptr, s.ptr, subtype, DIRECTION_OUT);
	}
	
	protected static native String lxcGetIOLabel(long gate, boolean direction, long sig, int subtype, int index);
	
	public static String getInputLabel(Gate g, Signal sig, int subtype, int index)
	{
		Gate.assertValid(g);
		Signal.assertValid(sig);
		return lxcGetIOLabel(g.ptr, DIRECTION_IN, sig.ptr, subtype, index);
	}

	public static String getOutputLabel(Gate g, Signal sig, int subtype, int index)
	{
		Gate.assertValid(g);
		Signal.assertValid(sig);
		return lxcGetIOLabel(g.ptr, DIRECTION_OUT, sig.ptr, subtype, index);
	}

	protected static final native long lxcGetWire(long gate, boolean direction, long signal, int subtype,  int index);

	protected static Object getWire(Gate gate, boolean direction, Signal type, int subtype, int index)
	{
		Gate.assertValid(gate);
		Signal.assertValid(type);
		long ptr = lxcGetWire(gate.ptr, direction, type.ptr, subtype, index);
		if(direction == DIRECTION_IN)
		{
			return Tokenport.tokenPortFromPointer(ptr);
		}
		else
		{
			return Wire.wireFromPointer(ptr);
		}
	}
	
	public static Tokenport getInputWire(Gate gate, Signal type, int subtype, int index)
	{
		return (Tokenport) getWire(gate, DIRECTION_IN, type, subtype, index);
	}

	public static Wire getOutputWire(Gate gate, Signal type, int subtype, int index)
	{
		return (Wire) getWire(gate, DIRECTION_OUT, type, subtype, index);
	}
	
	protected static native String[] lxcEnumerateProperties(long gate);
	
	public static String[] enumerateProperties(Gate g)
	{
		Gate.assertValid(g);
		return lxcEnumerateProperties(g.ptr);
	}

	protected static native String lxcGetPropertyLabel(long gate, String property);
	
	public static String getPropertyLabel(Gate gate, String property)
	{
		Gate.assertValid(gate);
		
		if(null == property)
		{
			return null;
		}
		
		return lxcGetPropertyLabel(gate.ptr, property);
	}

	protected static native String lxcGetPropertyDescription(long gate, String property);
	
	public static String getPropertyDescription(Gate gate, String property)
	{
		Gate.assertValid(gate);
		
		if(null == property)
		{
			return null;
		}
		
		return lxcGetPropertyDescription(gate.ptr, property);
	}
	
	
	protected static native String lxcGetPropertyValue(long gate, String name);
	
	public static String getPropertyValue(Gate g, String property)
	{
		Gate.assertValid(g);
		if(null== property)
		{
			return null;
		}
		
		return lxcGetPropertyValue(g.ptr, property);
	}

	protected static native String lxcSetPropertyValue(long gate, String propetry, String value);
	
	public static void setPropertyValue(Gate gate, String property, String value) throws LogxControllException
	{
		Gate.assertValid(gate);
		if(null == property)
		{
			throw new RuntimeException("Property may not null!");
		}
		
		String ret = lxcSetPropertyValue(gate.ptr, property, value);
		if(null != ret)
			throw new LogxControllException(ret);
	}
	
	protected static native String lxcGetSignalName(long sig);
	
	public static String getSignalName(Signal sig)
	{
		Signal.assertValid(sig);
		return lxcGetSignalName(sig.ptr);
	}

	protected static native long lxcGetWireSignal(long wire);
	
	public static Signal getWireSignal(Wire w)
	{
		Wire.assertValid(w);
		return Signal.signalFromPointer(lxcGetWireSignal(w.ptr));
	}
	
	protected static native String lxcGetWireReferenceDesignator(long wire);
	
	public static String getWireReferenceDesignator(Wire wire)
	{
		Wire.assertValid(wire);
		long ptr = Wire.pointerFromWire(wire);
		return lxcGetWireReferenceDesignator(ptr);
	}
	
	protected static native void lxcSetWireReferenceDesignator(long wire, String name);
	
	public static void setWireReferenceDesignator(Wire wire, @MayNull String name)
	{
		Wire.assertValid(wire);
		long ptr = Wire.pointerFromWire(wire);
		lxcSetWireReferenceDesignator(ptr, name);
	}

	
	static final native boolean lxcCheckPortiInsance(long gate);
	
	static final native long lxcPortiGetPortManager(long porti_instance, boolean direction);
	
	static final native long lxcPortiGetWire(long gate, boolean direction, int abs);
	
	static final native boolean lxcPortCheckPortnameInUse(long port_mngr, String name);

	//static final native void lxcPortUncheckedAddNewPort(long pm, String name, long type, boolean senstitive);
	
	static final native void lxcPortUncheckedAddNewPort1(long pm, long name, long type, int subtype);
	
	static final native int lxcPortGetAbsIndex(long pm, long signal, int subtype, int index);
	
	static final native int lxcPortRemovePort(long pm, long signal, int subtype, int index);
	
	static final native int lxcPortGetAbsindexByName(long pm, String name);
	
	static native String lxcLibTreeNodeName(long tree_node);
	
	static native long[] lxcLibTreeSubNode(long tree_node);
	
	static native long[] lxcLibTreeBehaviors(long tree_node);
	
	static native String lxcBehaviorGetName(long lxc_gate_behavior);
	
	static native long lxcBehaviorNewInstance(long lxc_gate_behavior);
	
	
/**************************** JavaBridge functions ****************************/
	
	static final native LogxControllCallback jbCb(long gate, boolean set, int index, LogxControllCallback cb);
	
	public static LogxControllCallback javaBridgeCallback(JavaBridgeGate gate, boolean set, int index, LogxControllCallback cb)
	{
		Gate.assertValid(gate);
		return jbCb(gate.ptr, set, index, cb);
	}
	
	static native long lxcPortiReallocIOPort(long gate, boolean direction, int from, int to);
	
	static native int lxcPortManagerMaxIndex(long pm);

	public static native String gnuLibcBacktraceSymbol(long addr);

	static native void lxcAbsorbToken(long ptr);
	
	public static void lxc_absorb_token(Tokenport tokenport)
	{
		if(null != tokenport)
		{
			lxcAbsorbToken(tokenport.ptr);
		}
	}

	static native long lxcGetTokenValue(long ptr);

	
	public static LxcValue lxc_get_token_value(Tokenport tokenport)
	{
		if(null != tokenport)
		{
			return LxcValue.valueFromPointer(lxcGetTokenValue(tokenport.ptr));
		}
		
		return null;
	}
	
	static native int lxcGetWireSubtype(long ptr);

	public static int getWireSubtype(Wire wire)
	{
		if(null != wire)
		{
			return lxcGetWireSubtype(Wire.pointerFromWire(wire));
		}
		throw new NullPointerException("wire may not null");
	}

	static native int lxcGetSignalTypeOrdinal(long signal);
	
	public static int getSignalTypeOrdinal(Signal signal)
	{
		return lxcGetSignalTypeOrdinal(Signal.pointerFromSignal(signal));
	}
	
	static native long lxcGetSignalByOrdinal(int ordinal);
	
	public static Signal getSignalByOrdinal(int ordinal)
	{
		return Signal.signalFromPointer(lxcGetSignalByOrdinal(ordinal));
	}
	
	static native int wireDebugHook(long wire, LogxControllDebugCallback dbg);
	
	public static void wireSetDebugHook(Wire w, LogxControllDebugCallback dbg) throws LogxControllException
	{
		Wire.assertValid(w);
		AssertArgument.assertNotNull(dbg, "dbg");
		int ret = wireDebugHook(Wire.pointerFromWire(w), dbg);
		if(0 != ret)
		{
			throw new LogxControllException(ret);
		}
	}
	
	public static void wireRemoveDebugHook(Wire w, LogxControllDebugCallback dbg) throws LogxControllException
	{
		Wire.assertValid(w);
		AssertArgument.assertNotNull(dbg, "dbg");
		int ret = wireDebugHook(Wire.pointerFromWire(w), null);
		if(0 != ret)
		{
			throw new LogxControllException(ret);
		}
	}
	
	static native LogxControllDebugCallback wireGetDebugHook(long wire);
	
	public static LogxControllDebugCallback wireGetDebugHook(Wire w)
	{
		Wire.assertValid(w);
		return wireGetDebugHook(Wire.pointerFromWire(w));
	}
	
}
