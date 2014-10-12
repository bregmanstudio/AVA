package org.omras2.audiodb 
{
	import flash.events.Event;
	import flash.media.Sound;

	/**
	 * @author mikej
	 */
	public class SoundEvent extends Event 
	{
		public static const COMPLETE : String = "soundLoaded";
		private var _sound : Sound;
		
		public function SoundEvent(type : String, sound : Sound, bubbles : Boolean = false, cancelable : Boolean = false)
		{
			this._sound = sound;
			super(type, bubbles, cancelable);
		}
		
				
		public override function clone() : Event
		{
			return new SoundEvent(type, _sound, bubbles, cancelable);
		}
		
		public function get sound() : Sound
		{
			return _sound;
		}
	}
}
