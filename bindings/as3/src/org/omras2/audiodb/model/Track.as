package org.omras2.audiodb.model 
{
	import flash.utils.Dictionary;

	/**
	 * @author mikej
	 */
	public class Track 
	{
		private var _uid : String;
		private var _album : String;
		private var _artist : String;
		private var _seconds : uint;
		private var _tracknum : uint;
		private var _track : String;
		private var _filename : String;
		
		public static function createFromResponse(response : Object) : Track
		{
			var track : Track = new Track();
			track.uid = response['uid'];
			track.album = response['album'];
			track.artist = response['artist'];
			track.seconds = response['seconds'];
			track.tracknum = response['tracknum'];
			track.track = response['track'];
			track.filename = response['filename'];
			return track;
		}
		
		public function get uid() : String
		{
			return _uid;
		}
		
		public function set uid(uid : String) : void
		{
			_uid = uid;
		}
		
		public function get album() : String
		{
			return _album;
		}
		
		public function set album(album : String) : void
		{
			_album = album;
		}
		
		public function get artist() : String
		{
			return _artist;
		}
		
		public function set artist(artist : String) : void
		{
			_artist = artist;
		}
		
		public function get seconds() : uint
		{
			return _seconds;
		}
		
		public function set seconds(seconds : uint) : void
		{
			_seconds = seconds;
		}
		
		public function get tracknum() : uint
		{
			return _tracknum;
		}
		
		public function set tracknum(tracknum : uint) : void
		{
			_tracknum = tracknum;
		}
		
		public function get track() : String
		{
			return _track;
		}
		
		public function set track(track : String) : void
		{
			_track = track;
		}
		
		public function get filename() : String
		{
			return _filename;
		}
		
		public function set filename(filename : String) : void
		{
			_filename = filename;
		}
	}
}
