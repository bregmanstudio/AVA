var sparqler = new SPARQL.Service("http://harrison/sparql/");

sparqler.setPrefix("mo", "http://purl.org/ontology/mo/");
sparqler.setPrefix("foaf", "http://xmlns.com/foaf/0.1/");
sparqler.setPrefix("dc", "http://purl.org/dc/elements/1.1/");

sparqler.setRequestHeader("Accept", "application/json");

var resultsTable;

$(document).ready(function() {
	$("#search").click(search)
	$("#spinner").hide();
	resultsTable = $('#results').dataTable({"bFilter":false,"bLengthChange":false,"bPaginate":true, 
		"fnRowCallback": function(nRow, aData, iDisplayIndex)
		{
			$(nRow).attr("typeof", "mo:Track");
			return nRow;
		},
		"fnDrawCallback": function ()
		{
			$(".artist_name").click(function(event) { searchArtist($(this).attr("href")); return false; });
			$(".album_name").click(function(event) { searchAlbum($(this).attr("href")); return false; });
		}
	});

	$("#results tbody").click(function(event) {
		$(resultsTable.fnSettings().aoData).each(function (){
			$(this.nTr).removeClass('row_selected');
		});
		$(event.target.parentNode).addClass('row_selected');
	});

});

function search(event) {

	var trackSearchString = $("#tracksearch").val();
	var artistSearchString = $("#artistsearch").val();

	if(trackSearchString.length == 0 && artistSearchString.length == 0)
	{
		resultsTable.fnClearTable();
		return;
	}

	var queryString = "SELECT ?maker ?album ?album_title ?tracknum ?artist_name ?track_title WHERE {";

	queryString += " ?track a mo:Track; mo:track_number ?tracknum; foaf:maker ?maker. ?album mo:track ?track; dc:title ?album_title. ?maker foaf:name ?artist_name";
	
	if(artistSearchString.length > 0)
	{
		queryString += ' FILTER regex(?artist_name, "^'+artistSearchString+'", "i")';
	}
	else
	{
		queryString += ".";
	}
	
	queryString += " ?record mo:track ?track; mo:publication_of ?signal. ?signal dc:title ?track_title";

	if(trackSearchString.length > 0)
	{
		queryString += ' FILTER regex(?track_title, "^'+trackSearchString+'", "i")';
	}
	else
	{
		queryString += ".";
	}

	queryString += " }";

	performSearch(queryString);
}

function searchArtist(id) {
	var queryString = "SELECT ?maker ?album ?album_title ?tracknum ?artist_name ?track_title WHERE {";
	queryString += " ?track a mo:Track; mo:track_number ?tracknum; foaf:maker ?maker. ?album mo:track ?track; dc:title ?album_title. ?maker foaf:name ?artist_name";
	queryString += " ?record mo:track ?track; mo:publication_of ?signal. ?signal dc:title ?track_title.";
	queryString += " FILTER(sameTerm(?maker, <"+id+">))";
	queryString += " }";
	performSearch(queryString);
}

function searchAlbum(id) {
	var queryString = "SELECT ?maker ?album ?album_title ?tracknum ?artist_name ?track_title WHERE {";
	queryString += " ?track a mo:Track; mo:track_number ?tracknum; foaf:maker ?maker. ?album mo:track ?track; dc:title ?album_title. ?maker foaf:name ?artist_name";
	queryString += " ?record mo:track ?track; mo:publication_of ?signal. ?signal dc:title ?track_title.";
	queryString += " FILTER(sameTerm(?album, <"+id+">))";
	queryString += " }";
	performSearch(queryString);
}


function performSearch(queryString) {
	$("#spinner").show();
	$("#query").text(queryString);
	var query = sparqler.createQuery();
	query.query(queryString, {failure: function(xhr) { alert("Bad response! "+xhr.responseText) }, success: displayResults});
}

function displayResults(json) {
	resultsTable.fnClearTable();
	if(json) {
		
		var bindings = json.results.bindings;
		for(var i=0; i<bindings.length; i++)
		{
			var artistEl = $('<div />');
			var artistLink = $('<a/>');
			artistEl.append(artistLink);
			artistLink.attr("href", bindings[i].maker.value);
			artistLink.attr("rel", "foaf:maker");
			artistLink.addClass("artist_name");
			artistLink.append(bindings[i].artist_name.value);
			
			var albumEl = $('<div />');
			var albumLink = $('<a/>');
			albumEl.append(albumLink);
			albumLink.attr("href", bindings[i].album.value);
			albumLink.attr("rel", "dc:title");
			albumLink.addClass("album_name");
			albumLink.append(bindings[i].album_title.value);
			
			resultsTable.fnAddData([artistEl.html(), bindings[i].track_title.value, bindings[i].tracknum.value, albumEl.html()]);
		}

	}
	$("#spinner").hide();
}
