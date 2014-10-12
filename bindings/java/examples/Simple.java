package examples;

import org.omras2.*;
import java.io.File;
import java.util.Vector;

public class Simple
{
	public static void main(String args[])
	{
		AudioDB testDB = new AudioDB(new File("simple.adb"));
		testDB.create(5, 5, 5);
		testDB.open(AudioDB.Mode.O_RDWR);
		System.out.println("Inserting 3 features");
		testDB.insert("feature1", 2, 5, new double[] { 6, 7, 8, 9, 10, 1, 2, 3, 4, 5 });
		testDB.insert("feature2", 1, 5, new double[] { 6, 7, 8, 9, 10 });
		testDB.insert("feature3", 5, 5, new double[] { 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 4, 4, 4, 4, 4, 5, 5, 5, 9, 10, 1, 2, 3, 4, 5 , 4, 4, 4, 4, 4});
		
		System.out.println("Performing query");
		Query query = new Query();
		query.setSeqLength(1);
		query.setSeqStart(0);
		Vector<Result> results = testDB.query("feature1", query);

		System.out.println("Results:");
		for(Result result: results)
		{
			System.out.println(result.getKey()+" "+result.getDistance()+" "+result.getIpos()+" "+result.getQpos());
		}
	}
}
