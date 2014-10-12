#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>
#include <librdf.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <audioDB_API.h>

const char * qstring = 
" PREFIX af: <http://purl.org/ontology/af/>"
" PREFIX dc: <http://purl.org/dc/elements/1.1/>"
" PREFIX mo: <http://purl.org/ontology/mo/>"
" PREFIX tl: <http://purl.org/NET/c4dm/timeline.owl#>"

" SELECT ?key ?value ?sample_rate ?window_length ?hop_size ?dimensions"

" WHERE { "
"   ?key a mo:AudioFile . "
"   ?signal a mo:Signal . "
"   ?key mo:encodes ?signal ."
"   ?signal mo:time [ tl:onTimeLine ?signal_timeline ] . "

"   ?timeline_map a tl:UniformSamplingWindowingMap ; "
"     tl:rangeTimeLine ?feature_timeline ; "
"     tl:domainTimeLine ?signal_timeline ; "
"     tl:sampleRate ?sample_rate ; "
"     tl:windowLength ?window_length ; "
"     tl:hopSize ?hop_size . "

"   ?signal af:signal_feature ?feature . "

"   ?feature a ?feature_signal_type ; "
"     mo:time [ tl:onTimeLine ?feature_timeline ] ; "
"     af:value ?value . "
"   ?feature af:dimensions ?dimensions ."

" } "
;

double *parse_value_string(const char *value_string, size_t *nelements) {
  /* What error checking? */


  *nelements = 0;

  const char *current = value_string;
  char *next = 0;

  size_t size = 1;
  double *buf = (double *) malloc(size * sizeof(double));
  double value = strtod(current, &next);
  while(next != current) {
    buf[(*nelements)++] = value;
    if((*nelements) == size) {
      size *= 2;
      buf = (double *) realloc(buf, 2 * size * sizeof(double));
    }
    current = next;
    value = strtod(current, &next);
  }
  return buf;
}

int main(int argc, char* argv[]) {
  
  if(argc != 3)
  {
    fprintf(stderr, "Usage: %s <n3file> <dbfile>\n", argv[0]);
    return 2;
  }
  
  char* n3file = argv[1];
  char* dbfile = argv[2];
  
  librdf_world *world;
  if (!(world = librdf_new_world())) 
    goto librdf_error;

  librdf_storage *storage;
  if (!(storage = librdf_new_storage(world, "memory", NULL, NULL)))
    goto librdf_error;

  librdf_model *model;
  if (!(model = librdf_new_model(world, storage, NULL)))
    goto librdf_error;
  
  librdf_uri *uri;
  if (!(uri = librdf_new_uri_from_filename(world, n3file)))
    goto librdf_error;

  librdf_parser *parser;
  if (!(parser = librdf_new_parser(world, "guess", NULL, NULL)))
    goto librdf_error;

  if(librdf_parser_parse_into_model(parser, uri, NULL, model))
    goto librdf_error;
  
  librdf_query *query;
  if (!(query = librdf_new_query(world, "sparql", NULL, qstring, NULL)))
    goto librdf_error;

  librdf_query_results *results;
  if (!(results = librdf_query_execute(query, model)))
    goto librdf_error;

  if(!librdf_query_results_is_bindings(results)) 
    goto librdf_error;

  adb_t *adb; 
  if(!(adb = audiodb_open(dbfile, O_RDWR))) {
    
    struct stat st;
    if(!(stat(dbfile, &st))) {
      fprintf(stderr, "%s not opened.\n", dbfile);
      return 1;
    } else {
      /* FIXME: if we are doing a proper SPARQL query over a
       * potentially unbounded number of results, we could use
       * librdf_query_results_get_count() to estimate how much space
       * our database will need.

       * If we're doing a SPARQL query over a number of RDF files,
       * with an expected number of datasets per file of 1 (as might
       * be produced by runner(?)) then obviously we can use that as
       * an estimate of number of tracks instead.

       * Specifying the data dimensionality should be easy from the
       * semantics of the feature being inserted; it's not immediately
       * obvious to me how to estimate the data size required, unless
       * we maybe do a preliminary query to find out the total time
       * (or similar) of all the tracks we're about to insert.

       * (also NOTE: this audiodb_create() interface is scheduled for
       * being made less inelegant.) */
      if(!(adb = audiodb_create(dbfile, 0, 0, 0))) {
        fprintf(stderr, "failed to create %s.\n", dbfile);
        return 1;
      }
    }
  }

  if(librdf_query_results_finished(results))
  {
    fprintf(stderr, "No features found!\n");
    return 3;
  }
  

  while(!librdf_query_results_finished(results)) {
    
    adb_datum_t datum = {0};
    
    // Pull out the dimension
    
    librdf_node *node = librdf_query_results_get_binding_value_by_name(results, "dimensions");
    char* dimensions = librdf_node_get_literal_value(node);
   
    int dimension = 0;
    sscanf(dimensions, "%d", &dimension);
    datum.dim = dimension;

    // Grab key and value
    
    node = librdf_query_results_get_binding_value_by_name(results, "key");
    char *key = strdup(librdf_uri_to_filename(librdf_node_get_uri(node)));
    datum.key = basename(key);
    
    size_t nelements = 0;
    node = librdf_query_results_get_binding_value_by_name(results, "value");
    datum.data = parse_value_string(librdf_node_get_literal_value(node), &nelements);
    
    // Validate that we have the correct number of elements
    
    if(nelements % dimension) return 4;
    datum.nvectors = nelements / dimension;
    
    printf("Dimension: %d\n", dimension);
    printf("Key: %s\n", datum.key);
    printf("Vectors: %d\n", datum.nvectors);
    
    librdf_free_node(node);
    
    if(audiodb_insert_datum(adb, &datum)) {
      fprintf(stderr, "failed to insert datum with key %s.\n", datum.key);
      return 1;
    }
    free(datum.data);
    librdf_query_results_next(results);
  }

  audiodb_close(adb);
  librdf_free_query_results(results);
  librdf_free_query(query);
  librdf_free_parser(parser);
  librdf_free_uri(uri);
  librdf_free_model(model);
  librdf_free_storage(storage);
  librdf_free_world(world);

  return 0;

 librdf_error:
  fprintf(stderr, "Something went wrong in librdf.\n");
  return 1;
}
