#!/usr/bin/python

import sys
import psycopg2
import psycopg2.extras

from rdflib.Graph import ConjunctiveGraph as Graph
from rdflib import Namespace, Literal, URIRef, BNode, RDF

catalogueID = sys.argv[1]

foaf = Namespace("http://xmlns.com/foaf/0.1/")
mo = Namespace("http://purl.org/ontology/mo/")
mb_artist = Namespace("http://dbtune.org/musicbrainz/resource/artist/")
dc = Namespace("http://purl.org/dc/elements/1.1/")
graph_uri = "http://omras2.gold.ac.uk/catalogue/"+catalogueID.lower()
audiodb = Namespace("http://omras2.gold.ac.uk/ontology/audiodb#")
doap = Namespace("http://usefulinc.com/ns/doap#")

username = "USERNAME"
host = "HOSTNAME"
database = "DATABASE"


try:
	conn = psycopg2.connect("dbname='"+database+"' user='"+username+"' host='"+host+"'");
except:
	print "Unable to connect to the database"

cursor = conn.cursor(cursor_factory=psycopg2.extras.DictCursor)

counters = {}
namespaces = {}
extractors = {}
projects = {}
databases = {}

def loadFeatures(catalogueID):
	cursor.execute("""SELECT * from features WHERE catalogue LIKE '"""+catalogueID+"""'""")
	rows = cursor.fetchall()
	return rows

def loadCatalogue(catalogueID):
	cursor.execute("""SELECT * from media WHERE catalogue LIKE '"""+catalogueID+"""'""")
	rows = cursor.fetchall()
	return rows

def createFeatureGraphs(rows):
	albums = {}

	graph = Graph(identifier = URIRef(graph_uri))
	counter = 1
	databases[catalogueID] = []

	for row in rows:
	
		# Create all the relevant nodes (with the correct IDs)
		database = getNewNode('database')
		databases[catalogueID].append(database)
		feature = getNewNode('feature')
		segmentation = getNewNode('segmentation')
		window = getNewNode('window')

		if row['feature'] == "cqt":
			graph.add((feature, RDF.type, audiodb['CQTFeature']))
		elif row['feature'] == "chr":
			graph.add((feature, RDF.type, audiodb['ChromagramFeature']))
		elif row['feature'] == "mfcc":
			graph.add((feature, RDF.type, audiodb['MFCCFeature']))

		if row['segtype'] == "frames":
			graph.add((segmentation, RDF.type, audiodb['FrameSegmentation']))
		elif row['segtype'] == "beats":
			graph.add((segmentation, RDF.type, audiodb['BeatSegmentation']))
		elif row['segtype'] == "segs":
			graph.add((segmentation, RDF.type, audiodb['StructuralSegmentation']))
		
		if row['windowtype'] == "hamming":
			graph.add((window, RDF.type, audiodb['HammingWindow']))

		graph.add((feature, audiodb["window"], window))
		graph.add((feature, audiodb["segmentation"], segmentation))

		graph.add((feature, audiodb["dimension"], Literal(row['dim'])))
		graph.add((feature, audiodb["hop-size"], Literal(row['hopsize'])))
		graph.add((feature, audiodb["window-length"], Literal(row['winlen'])))
		graph.add((feature, audiodb["nfft"], Literal(row['nfft'])))
		graph.add((feature, audiodb["segn"], Literal(row['segn'])))
		graph.add((feature, audiodb["channel"], Literal(row['channel'])))
		graph.add((feature, audiodb["loedge"], Literal(row['loedge'])))
		graph.add((feature, audiodb["hiedge"], Literal(row['hiedge'])))
		graph.add((feature, audiodb["octaveres"], Literal(row['octaveres'])))

		version = buildNewExtractor(graph, row['software'], row['version'], row['platform'])

		project = buildNewProject(graph, row['software'])
		graph.add((project, doap['release'], version))

		graph.add((database, RDF.type, audiodb["Database"]))
		graph.add((database, audiodb["feature"], feature))
		graph.add((database, audiodb["extractor"], version))
		
		counter += 1
	graph.serialize(format='xml',destination="output/"+catalogueID.lower()+"/"+"features.rdf")
 
def buildNewExtractor(graph, software, version, platform):
	key = software+"_"+version+"_"+platform
	try:
		extractor = extractors[key]
	except KeyError:
		extractor = getNewNode('extractor')
		graph.add((extractor, RDF.type, doap["Version"]))
		graph.add((extractor, doap['version'], Literal(version)))
		graph.add((extractor, doap['name'], Literal(software)))
		graph.add((extractor, doap['os'], Literal(platform)))
		extractors[key] = extractor
	return extractor

def buildNewProject(graph, software):
	key = software
	try:
		project = projects[key]
	except KeyError:
		project = getNewNode('project')
		graph.add((project, RDF.type, doap["Project"]))
		graph.add((project, doap['name'], Literal(software)))
		projects[key] = project
	return project

def createMediaGraphs(rows):
	albums = {}

	artists = {
		'Madonna': mb_artist['79239441-bfd5-4981-a70c-55c3f15c1287'], 
		'John Coltrane': mb_artist['b625448e-bf4a-41c3-a421-72ad46cdb831'], 
		'Miles Davis' : mb_artist['561d854a-6a28-4aa7-8c99-323e6ce46c2a']}

	counter = 1
	for row in rows:
		graph = Graph(identifier = URIRef(graph_uri))
		# Create all the relevant nodes (with the correct IDs)

		work = getNewNode('work')
		composition = getNewNode('composition')
		track = getNewNode('track')
		record = getNewNode('record')
		performance = getNewNode('performance')
		signal = Namespace(graph_uri+"/"+row['uid'])

		# If we don't have an artist url, make a foaf Agent instead.
		if row['artist']:
			try:
				artist = artists[row['artist']]
			except KeyError:
				artist = getNewNode('artist')
				graph.add((artist, RDF.type, foaf['Agent']))
				graph.add((artist, foaf['name'], Literal(row['artist'].strip())))
				artists[row['artist']] = artist;	

		if row['composer']:
			try:
				composer = artists[row['composer']]
			except KeyError:
				composer = getNewNode('artist')
				graph.add((composer, RDF.type, foaf['Agent']))
				graph.add((composer, foaf['name'], Literal(row['composer'].strip())))
				artists[row['composer']] = composer;	
		else:
			composer = artist


		# Work
		graph.add((work, RDF.type, mo['MusicalWork']))
		
		# Composition
		graph.add((composition, RDF.type, mo['Composition']))
		if composer:
			graph.add((composition, mo['composer'], composer)) 
		graph.add((composition, mo['produced_work'], work))

		# Track
		graph.add((track, RDF.type, mo['Track']))
		if row['artist']:
			graph.add((track, foaf['maker'], artist))
		if row['tracknum']:
			graph.add((track, mo['track_number'], Literal(row['tracknum'])))

		if row['album']:
			# Album
			try:
				album = albums[row['album']]
			except KeyError:
				album = getNewNode('album')
				graph.add((album, RDF.type, mo['Record']))
				graph.add((album, dc['title'], Literal(row['album'].strip())))
				graph.add((album, mo['release_type'], mo['album']))
				albums[row['album']] = album
			graph.add((album, mo['track'], track))

		# Signal
		graph.add((signal, RDF.type, mo['Signal']))
		graph.add((signal, mo['published_as'], record))
		
		if row['track']:
			graph.add((signal, dc['title'], Literal(row['track'].strip())))
		if row['isrc']:
			graph.add((signal, mo['isrc'], Literal(row['isrc'].strip())))

		# Add to the various databases
		dbs = databases[catalogueID]
		for db in dbs:
			graph.add((db, audiodb["has-signal"], signal))

		# Record
		graph.add((record, RDF.type, mo['Record']))
		graph.add((record, mo['publication_of'], signal))
		graph.add((record, mo['track'], track))

		# Performance
		graph.add((performance, RDF.type, mo['Performance']))
		graph.add((performance, mo['performance_of'], work))
		if row['artist']:
			graph.add((performance, mo['performer'], artist))
		graph.add((performance, mo['recorded_as'], signal))
		
		graph.close()
		graph.serialize(format='xml',destination="output/"+catalogueID.lower()+"/media_"+str(counter)+".rdf")
		counter += 1
 
def getNewNode(type):
	try:
		count = counters[type]
	except KeyError:
		counters[type] = 1
		count = counters[type]

	try:
		namespace = namespaces[type]
	except KeyError:
		namespaces[type] = Namespace(graph_uri+"/"+type+"/")
		namespace = namespaces[type]

	node = namespace[str(count)]
	counters[type] += 1
	return node

features = loadFeatures(catalogueID)
catalogue = loadCatalogue(catalogueID)

createFeatureGraphs(features)
createMediaGraphs(catalogue)
