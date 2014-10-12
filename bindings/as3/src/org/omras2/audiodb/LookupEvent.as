package org.omras2.audiodb 
{
	import org.omras2.audiodb.model.Track;

	import flash.events.Event;

	/**
	 * @author mikej
	 */
	public class LookupEvent extends Event 
	{
		public static const COMPLETE : String = "lookupComplete";
		
		private var _track : Track;

		public function LookupEvent(type : String, track : Track, bubbles : Boolean = false, cancelable : Boolean = false)
		{
			this._track = track;
			super(type, bubbles, cancelable);
		}
		
		public function get track() : Track
		{
			return _track;
		}
		
		public override function clone() : Event
		{
			return new LookupEvent(type, _track, bubbles, cancelable);
		}
	}
}
