package  
{
	import asunit.textui.TestRunner;
	import asunit.textui.XMLResultPrinter;

	import tests.AllTests;

	import flash.display.Sprite;

	/**
	 * @author mikej
	 */
	public class Main extends Sprite 
	{
		public function Main()
		{
			var unittests : TestRunner = new TestRunner();
			stage.addChild(unittests);
			unittests.start(tests.AllTests, null, TestRunner.SHOW_TRACE);
		}
	}
}
