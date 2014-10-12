package org.omras2;

import java.io.File;
import java.util.Vector;

public class AudioDB
{
	public native boolean audiodb_create(String path, int datasize, int ntracks, int datadim);
	public native boolean audiodb_open(String path, Mode mode);
	public native void audiodb_close();
	public native Status audiodb_status();
	public native boolean audiodb_insert_path(String key, String features, String power, String times);
	public native boolean audiodb_insert_data(String key, int nvectors, int dim, double[] features, double[] power, double[] times);
	public native Vector<Result> audiodb_query(String key, Query config);

	public enum Mode { O_RDONLY, O_RDWR }

	private File path;
	private long adbHandle;

	public AudioDB(File path)
	{
		this.path = path;
	}

	public void close()
	{
		audiodb_close();
	}

	public boolean insert(File features)
	{
		return insert(null, features, null, null);
	}
	
	public boolean insert(String key, File features)
	{
		return insert(key, features, null, null);
	}
	
	public boolean insert(String key, File features, File power)
	{
		return insert(key, features, power, null);
	}
	
	public boolean insert(String key, File features, File power, File times)
	{
		return audiodb_insert_path(key, features.getPath(), (power == null ? null : power.getPath()), (times == null ? null : times.getPath()));
	}

	public boolean insert(String key, int nvectors, int dim, double[] features)
	{
		return insert(key, nvectors, dim, features, null, null);
	}
	
	public boolean insert(String key, int nvectors, int dim, double[] features, double[] power)
	{
		return insert(key, nvectors, dim, features, power, null);
	}

	public boolean insert(String key, int nvectors, int dim, double[] features, double[] power, double[] times)
	{
		return audiodb_insert_data(key, nvectors, dim, features, power, times);
	}	

	public boolean create(int datasize, int ntracks, int datadim)
	{
		return audiodb_create(path.toString(), datasize, ntracks, datadim);
	}

	public boolean open(Mode mode)
	{
		return audiodb_open(path.toString(), mode);
	}
	
	public Vector<Result> query(Query config)
	{
		return audiodb_query(null, config);	
	}

	public Vector<Result> query(String key, Query config)
	{
		return audiodb_query(key, config);	
	}

	public Status getStatus() 
	{
		return audiodb_status();
	}

	static {
		System.loadLibrary("AudioDB_JNI");
	}


	public static void main(String args[])
	{
		AudioDB testDB = new AudioDB(new File("test.adb"));
		testDB.create(5, 5, 12);
		testDB.open(Mode.O_RDWR);
		testDB.insert(new File("testfiles/testfeature"));
		Status status = testDB.getStatus();
		System.out.println("Num files: "+status.getNumFiles());
		testDB.close();
	}
}


