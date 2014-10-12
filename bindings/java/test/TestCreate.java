import junit.framework.*;
import java.io.File;
import org.omras2.*;

public class TestCreate extends TestCase
{
	File testDBFile;

	protected void setUp()
	{
		testDBFile = new File("testfiles/test.adb");
		if(testDBFile.exists())
			testDBFile.delete();
	}

	public void testCreateNew()
	{
		AudioDB testDB = new AudioDB(testDBFile);
		assertTrue("DB created", testDB.create(5, 5, 12));
		assertTrue("Test DB created on FS", testDBFile.exists());
		assertTrue("Test DB has length > 0", testDBFile.length() > 0);
	}

	public void testReplaceExisting()
	{
		AudioDB testDB = new AudioDB(testDBFile);
		assertTrue("DB created", testDB.create(5, 5, 12));

		// Try to create again
		testDB = new AudioDB(testDBFile);
		assertFalse("DB not created", testDB.create(5, 5, 12));
		assertTrue("Test DB still exists on FS", testDBFile.exists());
		assertTrue("Test DB still has length > 0", testDBFile.length() > 0);
	}
}
