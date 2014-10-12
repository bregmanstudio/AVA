package org.omras2.audiodb 
{
	import flash.net.URLVariables;
	import com.adobe.serialization.json.JSON;

	import org.omras2.audiodb.model.SearchResult;
	import org.omras2.audiodb.model.Track;

	import flash.events.ErrorEvent;
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.IOErrorEvent;
	import flash.media.Sound;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	import flash.utils.Dictionary;

	/**
	 * @author mikej
	 */
	public class Bridge extends EventDispatcher
	{
		private var _endpoint : String;

		private var _trackCache : Dictionary;

		public function Bridge(endpoint : String)
		{
			this._endpoint = endpoint;
			this._trackCache = new Dictionary();
		}

		/**
		 * Metadata handling
		 */

		public function lookup(uid : String) : void
		{
			if(_trackCache[uid])
			{
				this.dispatchEvent(new LookupEvent(LookupEvent.COMPLETE, _trackCache[uid]));
			}
			else
			{
				var url : String = _endpoint + "/track/" + uid;
				var req : URLRequest = new URLRequest(url);
				var loader : URLLoader = new URLLoader();
				
				try
				{
					loader.load(req);	
				}
				catch(error : SecurityError)
				{
					var err : ErrorEvent = new ErrorEvent(ErrorEvent.ERROR);
					err.text = error.message;
					this.dispatchEvent(err);
				}
				
				loader.addEventListener(IOErrorEvent.IO_ERROR, this.handleQueryError);
				loader.addEventListener(Event.COMPLETE, this.handleLookupComplete);
			}
		}

		private function handleLookupComplete(event : Event) : void 
		{
			var response : Object = getResponse(event);
			if(validResponse(response))
			{
				var track : Track = Track.createFromResponse(response['data']);
				_trackCache[track.uid] = track;
				this.dispatchEvent(new LookupEvent(LookupEvent.COMPLETE, track));
			}
		}

		/**
		 * Search handling
		 */

		public function search(uid : String) : void
		{
			var url : String = _endpoint + "/search/" + uid;
			var req : URLRequest = new URLRequest(url);
			var loader : URLLoader = new URLLoader();
				
			try
			{
				loader.load(req);	
			}
			catch(error : SecurityError)
			{
				var err : ErrorEvent = new ErrorEvent(ErrorEvent.ERROR);
				err.text = error.message;
				this.dispatchEvent(err);
			}
				
			loader.addEventListener(IOErrorEvent.IO_ERROR, this.handleQueryError);
			loader.addEventListener(Event.COMPLETE, this.handleSearchComplete);
		}

		private function handleSearchComplete(event : Event) : void 
		{
			var response : Object = getResponse(event);
			if(validResponse(response))
			{
				var results : Array = [];
				for each(var result : Array in response['data'])
				{
					results.push(SearchResult.createFromResponse(result));
				}
				this.dispatchEvent(new SearchEvent(SearchEvent.COMPLETE, results));
			}
		}
		
		/**
		 * Playback functions
		 */
		 
		public function getSound(uid : String, start : uint, length : uint) : void
		{
			var url : String = _endpoint + "/audio/" + uid;
			var req : URLRequest = new URLRequest(url);
			var vars : URLVariables = new URLVariables();
			vars['start'] = start;
			vars['length'] = length;
			req.data = vars;
			var sound : Sound = new Sound();
			sound.addEventListener(Event.COMPLETE, this.handleSoundLoaded);
			sound.load(req);
		}

		private function handleSoundLoaded(event : Event) : void 
		{
			trace("Sound loaded");
			this.dispatchEvent(new SoundEvent(SoundEvent.COMPLETE, (event.target as Sound)));
		}

		/**
		 * Reusable parsing / error functions
		 */

		private function getResponse(event : Event) : Object 
		{
			var data : String = (event.target as URLLoader).data as String;
			var response : Object = (JSON.decode(data) as Object);
			return response;
		}

		private function validResponse(response : Object) : Boolean 
		{
			if(response['status'] != 'ok')
			{
				var err : ErrorEvent = new ErrorEvent(ErrorEvent.ERROR);
				err.text = response['message'];
				this.dispatchEvent(err);
				return false;
			}
			return true;
		}

		private function handleQueryError(event : IOErrorEvent) : void 
		{
			this.dispatchEvent(event);
		}
	}
}
