package tests 
{
	import asunit.framework.TestCase;

	import org.omras2.audiodb.Bridge;
	import org.omras2.audiodb.LookupEvent;
	import org.omras2.audiodb.SearchEvent;
	import org.omras2.audiodb.SoundEvent;
	import org.omras2.audiodb.model.SearchResult;

	import flash.events.ErrorEvent;
	import flash.media.Sound;

	/**
	 * @author mikej
	 */
	public class TestBridge extends TestCase
	{
		private var _bridge : Bridge;

		public function TestBridge(testMethod : String)
		{
			super(testMethod);
		}

		override protected function setUp() : void 
		{
			super.setUp();
			this._bridge = new Bridge("http://127.0.0.1:8080");
		}

		public function testLookupSuccess() : void
		{
			var handler : Function = addAsync(handleLookupSuccessComplete, 2000);
			_bridge.addEventListener(LookupEvent.COMPLETE, handler);	
			_bridge.lookup("AWAL1000");
		}

		public function testLookupFail() : void
		{
			var handler : Function = addAsync(handleLookupFailComplete, 2000);
			_bridge.addEventListener(ErrorEvent.ERROR, handler);	
			_bridge.lookup("AWAL10000");
		}

		private function handleLookupFailComplete(event : ErrorEvent) : void 
		{
			assertEquals(event.text, 'Invalid key');
		}

		protected function handleLookupSuccessComplete(event : LookupEvent) : void 
		{
			assertEquals(event.track.uid, 'AWAL1000');
			assertEquals(event.track.artist, 'Moscow Drive');
			assertEquals(event.track.seconds, '221000');
		}
		
		public function testSearchSuccess() : void
		{
			var handler : Function = addAsync(handleSearchSuccessComplete, 50000);
			_bridge.addEventListener(SearchEvent.COMPLETE, handler);
			_bridge.search("AWAL1000");
		}
		
		protected function handleSearchSuccessComplete(event : SearchEvent) : void 
		{
			assertEquals(20, event.results.length);
			var firstMatch : SearchResult = (event.results[0] as SearchResult);
			assertEquals("AWAL1000", firstMatch.uid);
		}
		
		public function testPlay() : void
		{
			
			var handler : Function = addAsync(handleSoundLoaded, 10000);
			_bridge.addEventListener(SoundEvent.COMPLETE, handler);
			_bridge.getSound("AWAL1000", 5, 5);
			
		}
		
		
		protected function handleSoundLoaded(event : SoundEvent) : void 
		{
			var sound : Sound = event.sound;
			assertEquals(81083, sound.bytesLoaded);
		}
	}
}
