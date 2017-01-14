package eu.logxcontroll;

import java.util.ArrayList;
import java.util.Iterator;

import eu.javaexperience.collection.iterator.ReadOnlyIterator;

public class LxcLibraryTreeNode extends NativeObject implements Iterable<LxcLibraryTreeNode>
{
	protected LxcLibraryTreeNode(long ptr)
	{
		super(ptr);
		nodeName = LogxControll.lxcLibTreeNodeName(ptr);
		
		long[] beh = LogxControll.lxcLibTreeBehaviors(ptr);
		if(null != beh)
		{
			for(int i=0;i<beh.length;++i)
			{
				behaviors.add(new GateBehavior(beh[i]));
			}
		}
		
		long[] sub = LogxControll.lxcLibTreeSubNode(ptr);
		if(null != sub)
		{
			for(int i=0;i<sub.length;++i)
			{
				LxcLibraryTreeNode node = new LxcLibraryTreeNode(sub[i]);
				node.setUpper(this);
				subNodes.add(node);
			}
		}
	}
	
	void setUpper(LxcLibraryTreeNode node)
	{
		upper = node;
	}
	
	protected LxcLibraryTreeNode upper;
	
	protected ArrayList<GateBehavior> behaviors = new ArrayList<>(); 
	
	protected ArrayList<LxcLibraryTreeNode> subNodes = new ArrayList<>();
	
	protected final String nodeName;

	public String getNodeName()
	{
		return nodeName;
	}
	
	@Override
	public Iterator<LxcLibraryTreeNode> iterator()
	{
		return new ReadOnlyIterator<>(subNodes.iterator());
	}
	
	public Iterable<GateBehavior> iterateBehaviors()
	{
		return new ReadOnlyIterator<>(behaviors.iterator());
	}
	
	public String getPath()
	{
		return recursivePath(null);
	}
	
	protected String recursivePath(String str)
	{
		if(null == nodeName)
		{
			return str;
		}
		
		if(null == str)
		{
			str = nodeName;
		}
		else
		{
			str = nodeName+"/"+str;
		}
		
		if(null != upper)
		{
			return upper.recursivePath(str);
		}
		
		return str;
	}
	
	protected static LxcLibraryTreeNode ROOT_NODE = new LxcLibraryTreeNode(0);
	
	public static LxcLibraryTreeNode getRootNode()
	{
		return ROOT_NODE;
	}
	
	public static void refresh()
	{
		ROOT_NODE =  new LxcLibraryTreeNode(0);
	}

	public String[] list()
	{
		String[] ret = new String[subNodes.size()];
		for(int i=0;i<ret.length;++i)
		{
			ret[i] = subNodes.get(i).nodeName;
		}
		return ret;
	}
	
	public String toString()
	{
		return "LxcLibraryNode "+nodeName;
	}
	
	
}