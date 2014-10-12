package org.omras2.audiodb.model 
{

	/**
	 * @author mikej
	 */
	public class SearchResult 
	{
		private var _uid : String;
		private var _dist : Number;
		private var _qpos : Number;
		private var _ipos : Number;
		
		public static function createFromResponse(response : Array) : SearchResult
		{
			var searchResult : SearchResult = new SearchResult();
			searchResult.uid = response[0];
			searchResult.dist = response[1];
			searchResult.qpos = response[2];
			searchResult.ipos = response[3];
			return searchResult;
		}
		
		public function get uid() : String
		{
			return _uid;
		}
		
		public function set uid(uid : String) : void
		{
			_uid = uid;
		}
		
		public function get dist() : Number
		{
			return _dist;
		}
		
		public function set dist(dist : Number) : void
		{
			_dist = dist;
		}
		
		public function get qpos() : Number
		{
			return _qpos;
		}
		
		public function set qpos(qpos : Number) : void
		{
			_qpos = qpos;
		}
		
		public function get ipos() : Number
		{
			return _ipos;
		}
		
		public function set ipos(ipos : Number) : void
		{
			_ipos = ipos;
		}
		
		public function toString() : String
		{
			return "[SearchResult uid="+uid+" dist="+dist+" qpos="+qpos+" ipos="+ipos+"]";
		}
	}
}
