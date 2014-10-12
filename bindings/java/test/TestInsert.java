import junit.framework.*;
import java.io.File;
import org.omras2.*;

public class TestInsert extends TestCase
{
	File testDBFile;
	File testFeatureFile;
	protected void setUp()
	{
		testDBFile = new File("testfiles/test.adb");
		testFeatureFile = new File("testfiles/testfeature");
		if(testDBFile.exists())
			testDBFile.delete();
	}

	public void testInsertFile()
	{
		AudioDB testDB = new AudioDB(testDBFile);
		assertTrue("DB created", testDB.create(1, 1, 1));
		testDB.open(AudioDB.Mode.O_RDWR);
		assertTrue("Insert feature file", testDB.insert(testFeatureFile));
		Status status = testDB.getStatus();
		assertEquals("One feature in db", 1, status.getNumFiles());
		assertEquals("1D Feature", 1, status.getDim());
	}

	public void testInsertFileReadOnly()
	{
		AudioDB testDB = new AudioDB(testDBFile);
		assertTrue("DB created", testDB.create(1, 1, 1));
		testDB.open(AudioDB.Mode.O_RDONLY);
		assertFalse("Insert feature file", testDB.insert(testFeatureFile));
	}

	public void testInsertData()
	{
		AudioDB testDB = new AudioDB(testDBFile);
		assertTrue("DB created", testDB.create(100, 100, 1));
		testDB.open(AudioDB.Mode.O_RDWR);
		Status status = testDB.getStatus();
		assertTrue("Insert feature 1", testDB.insert("feature1", 5, 1, new double[] { 1, 2, 3, 4, 5 }));
		assertTrue("Insert feature 2", testDB.insert("feature2", 5, 1, new double[] { 1, 2, 3, 4, 5 }));
		assertTrue("Insert feature 3", testDB.insert("feature3", 5, 1, new double[] { 5, 4, 3, 2, 1 }));
		assertFalse("Insert feature 3 again", testDB.insert("feature3", 5, 1, new double[] { 5, 4, 3, 2, 1 }));
		assertFalse("Insert bad feature", testDB.insert("feature4", 1, 3, new double[] { 5, 4, 3 }));
		assertEquals("3 features in db", 3, testDB.getStatus().getNumFiles());
	}
}
