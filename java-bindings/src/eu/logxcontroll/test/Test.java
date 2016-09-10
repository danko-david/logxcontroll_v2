package eu.logxcontroll.test;

import java.io.IOException;

import eu.javaexperience.io.IOTools;
import eu.javaexperience.io.fd.FDIOStream;
import eu.javaexperience.io.fd.FDIOStreamFactory;
import eu.javaexperience.nativ.posix.CString;
import eu.javaexperience.nativ.posix.Posix;
import eu.logxcontroll.Gate;
import eu.logxcontroll.JavaBridgeGate;
import eu.logxcontroll.LogxControll;
import eu.logxcontroll.LogxControllException;
import eu.logxcontroll.LxcValue;
import eu.logxcontroll.Signal;
import eu.logxcontroll.Wire;
import eu.logxcontroll.java.LogxControllCallback;

public class Test
{
	public static void main(String[] args) throws Throwable
	{
		//loadPosixLibrary();
		//showLibraryTree();
		//System.exit(0);
		testPosixSockets();
		
		//debugJB();
	}
	
	public static void debugJB()
	{
		LogxControllCallback cb = new LogxControllCallback()
		{
			@Override
			public void execute(Gate gate, Signal type, int subtype, int input, LxcValue value)
			{
				System.out.println
				(
					gate+"\n"+
					type+"\n"+
					input+"\n"+
					value
				);
			}
		};
		
		JavaBridgeGate bridge = JavaBridgeGate.newInstance();

		Signal sig_int = LogxControll.getSignalByName("int");
		Signal sig_long = LogxControll.getSignalByName("long");
		
		bridge.dbgPrintAll();
		
		bridge.addInputPort(sig_int, 0, "int0", true, cb);
		
		bridge.dbgPrintAll();
		
		bridge.addInputPort(sig_int, 0, "int1", true, cb);
		
		bridge.dbgPrintAll();
		
		bridge.addInputPort(sig_int, 0, "int2", true, cb);
		
		bridge.dbgPrintAll();
		
		bridge.addInputPort(sig_long, 0, "long0", true, cb);
		
		bridge.dbgPrintAll();
	}
	
	public static void testPosixSockets() throws LogxControllException
	{
		final Signal signal_int = LogxControll.getSignalByName("int");
		final Signal signal_sockaddr = LogxControll.getSignalByName("sockaddr");
		final Signal signal_string = LogxControll.getSignalByName("string");
		
		LogxControllCallback cb = new LogxControllCallback()
		{
			@Override
			public void execute(Gate gate, Signal type,  int subtype, int input, LxcValue value)
			{
				System.out.println(gate+" "+type+" "+subtype+" "+input+" "+value);
				if(!type.getName().equals("int"))
				{
					System.out.println("Nem int!");
					return;
				}
				
				if(null == value)
				{
					System.out.println("Null érték");
					return;
				}
				/*System.out.println
				(
					gate+"\t"+
					type+"\t"+
					input+"\t"+
					value
				);*/
				
				int val = Posix.intAt(value.getData());
				System.out.println("Bemenet: "+input+", érték: "+val);

				if(input == 0)
				{
					FDIOStream s = FDIOStreamFactory.fromFD(val);
					try {
						IOTools.copyStream(s.getInputStream(), System.out);
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		};
		
		
		JavaBridgeGate bridge = JavaBridgeGate.newInstance();
		
		Wire fd_connection = LogxControll.createWire(signal_int);
		
		Wire remote = LogxControll.createWire(signal_sockaddr);
		
		Wire in_fd = LogxControll.createWire(signal_int);
		
		Gate socket_create = LogxControll.newInstanceByName("socket create");
		
		Gate socketaddress_create = LogxControll.newInstanceByName("socketaddress create");
		
		Gate socket_bring_up = LogxControll.newInstanceByName("socket bring up");
		
		socketaddress_create.setPropertyValue("family", "INET");
		
		/*
		bridge.dbgPrintAll();
		socket_create.dbgPrintAll();
		socketaddress_create.dbgPrintAll();
		*/
		
		//socket_bring_up.dbgPrintAll();
		
		Wire addr = LogxControll.createWire(signal_string);
		Wire port = LogxControll.createWire(signal_int);
		
		socketaddress_create.wireInput(addr, 0);
		socketaddress_create.wireInput(port, 0);
		socketaddress_create.wireOutput(remote, 0);
		
		Wire af = LogxControll.createWire(signal_int);
		Wire sock = LogxControll.createWire(signal_int);
		Wire pf = LogxControll.createWire(signal_int);
		
		socket_create.wireInput(af, 0);
		socket_create.wireInput(sock, 1);
		socket_create.wireInput(pf, 2);
		socket_create.wireOutput(fd_connection, 0);
		
		socket_bring_up.wireInput(fd_connection, 0);
		
		socket_bring_up.wireInput(remote, 1);
		
		Wire error_bring = LogxControll.createWire(signal_int);
		Wire error_cre = LogxControll.createWire(signal_int);
		
		socket_bring_up.wireOutput(in_fd, 0);
		socket_bring_up.wireOutput(error_bring, 1);
		

		//create values:
		
		
		LxcValue AF_INET = createIntValue(Posix.AF_INET);
		LxcValue SOCK_STREAM = createIntValue(Posix.SOCK_STREAM);
		LxcValue PF_UNSPEC = createIntValue(0);

		LxcValue dst_addr = createStringValue("127.0.0.1");
		
		LxcValue dst_port = createIntValue(3000);
		
		//0
		bridge.addInputPort(signal_int, 0, "in_fd", true, cb);
		
		//1
		bridge.addInputPort(signal_int, 0, "errno_in", true, cb);
		
		//2
		bridge.addInputPort(signal_int, 0, "errno_bring", true, cb);
		
		//3
		bridge.addInputPort(signal_int, 0, "port_num", true, cb);
		
		bridge.wireInput(in_fd, 0);
		bridge.wireInput(error_bring, 1);
		bridge.wireInput(error_cre, 2);
		bridge.wireInput(port, 3);
		
		socket_create.wireOutput(error_cre, 1);
		
		bridge.setEnabled(true);
		
		socket_create.setEnabled(true);
		socketaddress_create.setEnabled(true);
		socket_bring_up.setEnabled(true);
		
		
		af.drive(bridge, 0, AF_INET);
		sock.drive(bridge, 0, SOCK_STREAM);
		pf.drive(bridge, 0, PF_UNSPEC);
		
		addr.drive(bridge, 0, dst_addr);
		port.drive(bridge, 0, dst_port);
		

		System.out.println("driven");
		
		Posix.pause();
	}
	
	
	public static LxcValue createIntValue(int num)
	{
		LxcValue ret = LxcValue.createGenericValue(LogxControll.getSignalByName("int"), 8);
		Posix.setIntAt(ret.getData(), num);
		return ret;
	}
	
	public static LxcValue createStringValue(String str)
	{
		if(null == str)
		{
			return null;
		}

		CString cs = new CString(str);
		LxcValue ret = LxcValue.createGenericValue(LogxControll.getSignalByName("string"), str.getBytes().length+1);
		Posix.strcpy(ret.getData(), cs.getPointer());
		return ret;
	}
	
	public static void _loadPosixLibrary()
	{
		String[] ret = LogxControll.loadSharedLibrary("/home/szupervigyor/projektek/LogxKontroll/WS/LogxControllCore/Shared/libLogxControllCore.elf");
		
		System.out.println("Library load start");
		for(String s:ret)
		{
			System.out.println(s);
		}
		System.out.println("Library load end");
	}
	
	public static void showLibraryTree()
	{
		LibraryTree.main(null);
	}
	
	public static void testJavaBridge() throws LogxControllException
	{
		System.out.println("Start");
		LogxControllCallback cb = new LogxControllCallback()
		{
			@Override
			public void execute(Gate gate, Signal type, int subtype, int input, LxcValue value)
			{
				System.out.println
				(
					gate+"\n"+
					type+"\n"+
					input+"\n"+
					value+"\n"+
					"\n"
				);
				
				//throw new RuntimeException();
				//new Exception().printStackTrace();
			}
		};
		
		JavaBridgeGate bridge = JavaBridgeGate.newInstance();
		
		Signal bool = LogxControll.getSignalByName("bool");
		
		Wire in = LogxControll.createWire(bool);
		
		bridge.addInputPort(bool, 0, "bool", true, cb);
		
		bridge.wireInput(in, 0);
		
		LxcValue val_true = LogxControll.getConstantByName("true");
		
		System.out.println(val_true);
		
		bridge.setEnabled(true);
		
		System.out.println("");
		
		in.drive(bridge, 0, val_true);
	}
}