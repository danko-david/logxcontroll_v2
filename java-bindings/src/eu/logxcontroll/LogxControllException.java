package eu.logxcontroll;

public class LogxControllException extends Exception
{
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	public final int lxc_errno;
	
	public LogxControllException(int lxc_errno)
	{
		super(String.valueOf(lxc_errno));
		this.lxc_errno = lxc_errno;
	}

	public LogxControllException(String ret)
	{
		super(ret);
		this.lxc_errno = 0;
	}
	
	//TODO get description from native environment
}