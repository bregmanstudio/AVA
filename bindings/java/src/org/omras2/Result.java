package org.omras2;
import lombok.Getter;

public class Result
{
	@Getter String key;
	@Getter int ipos;
	@Getter int qpos;
	@Getter double distance;

	public Result(String key, double distance, int ipos, int qpos)
	{
		this.key = key;
		this.distance = distance;
		this.ipos = ipos;
		this.qpos = qpos;
	}

	public String toString()
	{
		return "{key="+key+" distance="+distance+" ipos="+ipos+" qpos="+qpos+"}";
	}
}
