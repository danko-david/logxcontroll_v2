package eu.logxcontroll;
import eu.logxcontroll.Signal;


public class FullSignalType
{
	protected Signal signal;
	protected int subtype;
	
	public static final FullSignalType[] emptyFullSignalTypeArray = new FullSignalType[0];
	
	public static FullSignalType[] fetch(long[] data)
	{
		if(null == data || data.length == 0 || data.length % 2 != 0)
		{
			return  emptyFullSignalTypeArray;
		}
		
		FullSignalType[] ret = new FullSignalType[data.length/2];
		for(int i=0;i<ret.length;++i)
		{
			FullSignalType fst = new FullSignalType();
			fst.signal = Signal.signalFromPointer(data[i*2]);
			fst.subtype = i*2+1;
			ret[i] = fst;
		}
		
		return ret;
	}

	public Signal getSignal()
	{
		return signal;
	}
	
	public int getSubtype()
	{
		return subtype;
	}
}