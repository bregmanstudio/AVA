package  
{
	
	import org.omras2.audiodb.Bridge;
	import org.omras2.audiodb.SearchEvent;
	import org.omras2.audiodb.SoundEvent;
	import org.omras2.audiodb.model.SearchResult;

	import flash.display.Sprite;
	import flash.media.Sound;
	/**
	 * @author mikej
	 */
	public class Example extends Sprite 
	{
		private var _bridge : Bridge;
		private var _sounds : Array;

		public function Example()
		{
			this._bridge = new Bridge("http://0.0.0.0:8080");
			this._sounds = [];
			
			// Find the tracks similar to AWAL1000
			_bridge.addEventListener(SearchEvent.COMPLETE, this.handleSearchComplete);
			_bridge.search("AWAL1000");
		}

		private function handleSearchComplete(event : SearchEvent) : void 
		{
			// Now grab the first 20s of the first result
			_bridge.addEventListener(SoundEvent.COMPLETE, this.handleSoundLoaded);
			
			var query : SearchResult = (event.results[0] as SearchResult);
			_bridge.getSound(query.uid, query.ipos, 5);
			
			var closestMatch : SearchResult = event.results[1] as SearchResult;
			
			_bridge.getSound(closestMatch.uid, closestMatch.ipos , 5);
		}

		private function handleSoundLoaded(event : SoundEvent) : void 
		{
			_sounds.push(event.sound);
			if(_sounds.length == 2)
			{
				for each(var sound : Sound in _sounds)
				{
					sound.play();		
				}
			}
		}
	}
}
