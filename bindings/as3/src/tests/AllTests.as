package  
tests
{
	import asunit.framework.TestSuite;

	/**
	 * @author mikej
	 */
	public class AllTests extends TestSuite 
	{
		public function AllTests()
		{
			super();
			
			addTest(new TestBridge("testLookupSuccess"));
			addTest(new TestBridge("testLookupFail"));
			addTest(new TestBridge("testSearchSuccess"));
			addTest(new TestBridge("testPlay"));
		}
	}
}
