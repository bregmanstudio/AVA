package org.omras2.audiodb 
{
	import flash.events.Event;
	
	/**
	 * @author mikej
	 */
	public class SearchEvent extends Event 
	{
		public static const COMPLETE : String = "searchComplete";
		private var _results : Array;
		
		public function SearchEvent(type : String, results : Array, bubbles : Boolean = false, cancelable : Boolean = false)
		{
			this._results = results;
			super(type, bubbles, cancelable);
		}
		
				
		public override function clone() : Event
		{
			return new SearchEvent(type, _results, bubbles, cancelable);
		}
		
		public function get results() : Array
		{
			return _results;
		}
	}
}
