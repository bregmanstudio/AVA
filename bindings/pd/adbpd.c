#include "m_pd.h"
#include <math.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <audioDB_API.h>

#define ADB_MAXSTR (512U)
#define MAXSTR ADB_MAXSTR

// Query types
#define O2_POINT_QUERY (0x4U)
#define O2_SEQUENCE_QUERY (0x8U)
#define O2_TRACK_QUERY (0x10U)
#define O2_N_SEQUENCE_QUERY (0x20U)
#define O2_ONE_TO_ONE_N_SEQUENCE_QUERY (0x40U)

static t_class *adbpd_class; /* so this bit is different */

typedef struct _adbpd {
  t_object x_obj;
  t_outlet *x_key;
  t_outlet *x_dist;
  t_outlet *x_qpos;
  t_outlet *x_spos;
  t_symbol *x_dbname; //database name
  
  t_int x_dbquerytype; //sequence, track, etc.
  t_symbol *x_dbfeature; 
//    t_symbol *x_dbpower;
  t_symbol *x_dbkey;
  t_float x_dbqpoint;
  t_float x_dbnumpoints;
  t_float x_dbradius;
  t_float x_dbresultlength;
  t_float x_dbsequencelength;
  adb_t *db; 
} t_adbpd;


static void adbpd_create(t_adbpd *, t_floatarg, t_floatarg, t_floatarg);
static void adbpd_open(t_adbpd *x);
static void adbpd_close(t_adbpd *x);
static void adbpd_setname(t_adbpd *x, t_symbol *s, int argc, t_atom *argv);
static void adbpd_status(t_adbpd *x);
static void adbpd_l2norm(t_adbpd *x);
static void adbpd_power(t_adbpd *x);
static void adbpd_setfeatures(t_adbpd *x,t_symbol *s,int argc, t_atom *argv);
static void adbpd_setquerytype(t_adbpd *x,t_symbol *s,int argc, t_atom *argv);
static void adbpd_setqpoint(t_adbpd *x,t_floatarg f);
static void adbpd_setresultlength(t_adbpd *x,t_floatarg f);
static void adbpd_setradius(t_adbpd *x,t_floatarg f);
static void adbpd_setnumpoints(t_adbpd *x,t_floatarg f);
static void adbpd_setsequencelength(t_adbpd *x,t_floatarg f);
static void adbpd_doquery(t_adbpd *x);
static void adbpd_parameters(t_adbpd *x);
static void adbpd_getname(t_adbpd *x);



/* input arguments are only used for startup vals */
static void *adbpd_new(t_symbol *s) {

  /* some sort of standard declaration line */
  t_adbpd *x = (t_adbpd *) pd_new(adbpd_class);
  x->x_dbname = s;
  
  /* setup some inlets */
  floatinlet_new(&x->x_obj,&x->x_dbqpoint); /* second inlet */
  floatinlet_new(&x->x_obj,&x->x_dbnumpoints); /* third inlet */
  floatinlet_new(&x->x_obj,&x->x_dbradius); /* fourth inlet */
  floatinlet_new(&x->x_obj,&x->x_dbresultlength); /* fifth inlet */
  floatinlet_new(&x->x_obj,&x->x_dbsequencelength); /* sixth inlet */
  
  /* outlets */
  outlet_new(&x->x_obj, &s_float);
  
  x->x_key = outlet_new(&x->x_obj, &s_symbol);
  x->x_dist = outlet_new(&x->x_obj, &s_float);
  x->x_qpos = outlet_new(&x->x_obj, &s_float);
  x->x_spos = outlet_new(&x->x_obj, &s_float);

  /* adb defaults. These are the same but we need to init them */
  x->x_dbquerytype=O2_POINT_QUERY;
  x->x_dbfeature=gensym("No feature file yet specified");
  x->x_dbqpoint=1;
  x->x_dbnumpoints=10;
  x->x_dbradius=1;
  x->x_dbresultlength=10;
  x->x_dbsequencelength=12;
  return(x);
}

static void adbpd_free(t_adbpd *x) {
  adbpd_close(x);
}

/* bang executes the defined query */
static void adbpd_bang(t_adbpd *x) {
  adbpd_doquery(x);
}

/* create a database */
static void adbpd_create(t_adbpd *x, t_floatarg datasize, t_floatarg ntracks, t_floatarg dim) {
  adbpd_close(x);

  post("Creating db '%s'", x->x_dbname->s_name);
  x->db = audiodb_create(x->x_dbname->s_name, datasize, ntracks, dim);

  if (x->db) {
    post("Created and opened");
  } else {
    error("Could not create db '%s'.", x->x_dbname->s_name);
  }
}

/* open a database, closing an existing one if necessary. */
static void adbpd_open(t_adbpd *x) {
  adbpd_close(x);

  post("Opening db '%s'", x->x_dbname->s_name);
  if((x->db = audiodb_open(x->x_dbname->s_name, O_RDWR))) {
    post("Opened");
  } else {
    error("Could not open db '%s'.", x->x_dbname->s_name);
  }
}

static void adbpd_close(t_adbpd *x) {
  if(x->db) {
    post("Closing db");
    audiodb_close(x->db);
    x->db = NULL;
  }
}

/* This is accessed via the 'set' message. It sets the name and opens
   the database. */
static void adbpd_setname(t_adbpd *x, t_symbol *s, int argc, t_atom *argv) {
  /* if we have a properly formed instruction */
  if (argc == 1 && argv->a_type == A_SYMBOL) {
    /* make the internal database reference name the same as the name
       of the database we want.  This is stupid. There must be a
       better way of doing this. */
    x->x_dbname = gensym(argv->a_w.w_symbol->s_name); 
    post("Name set to '%s'.", x->x_dbname->s_name);
  }
  /* now we can open the database as we did with the audiodb_open call
     above */
  /* FIXME: well, we _can_, but why is that a good idea? */
  adbpd_open(x);
}


/* this is a status call to the audioDB API */
static void adbpd_status(t_adbpd *x){

  adb_status_t mystatus; 

  post("Getting Status");
  
  if (x->db && !(audiodb_status(x->db,&mystatus))){
    post("numFiles %d",mystatus.numFiles);
    post("dim %d",mystatus.dim);
    post("length %d",mystatus.length);
    post("dudCount %d",mystatus.dudCount);
    post("numCount %d",mystatus.nullCount);
    post("flags %d",mystatus.flags);
    post("Bytes Available %d",mystatus.data_region_size);
  } else {
    error("Can't get Status. Have you selected a db?");
  }
}


static void adbpd_doquery(t_adbpd *x){

  adb_datum_t datum = {0};
  adb_query_id_t qid = {0};
  adb_query_parameters_t params = {0};
  adb_query_refine_t refine = {0};
  adb_query_spec_t spec;
  
  // Configure the start+length of the query, and zero
  // the flags initially.
  qid.datum = &datum;
  qid.sequence_length = x->x_dbsequencelength;
  qid.sequence_start = x->x_dbqpoint;
  qid.flags = 0;

  refine.flags |= ADB_REFINE_RADIUS;
  refine.radius = x->x_dbradius;

  spec.qid = qid;
  spec.params = params;
  spec.refine = refine;
  
  int fd;
  struct stat st;

  // Read in the feature file - note that this code may contain leaks
  // (the same chunk also exists in audioDB.cpp).
  fd = open(x->x_dbfeature->s_name, O_RDONLY);
  if(fd < 0) {
    error("failed to open feature file", x->x_dbfeature->s_name);
    return;
  }
  fstat(fd, &st);
  read(fd, &datum.dim, sizeof(uint32_t));
  datum.nvectors = (st.st_size - sizeof(uint32_t)) / (datum.dim * sizeof(double));
  datum.data = (double *) malloc(st.st_size - sizeof(uint32_t));
  read(fd, datum.data, st.st_size - sizeof(uint32_t));
  close(fd);
  
  // Set up query spec params depending on the query type.
  switch(x->x_dbquerytype)
    {
    case O2_POINT_QUERY:
      spec.qid.sequence_length = 1;
      spec.params.accumulation = ADB_ACCUMULATION_DB;
      spec.params.distance = ADB_DISTANCE_DOT_PRODUCT;
      spec.params.npoints = x->x_dbnumpoints;
      spec.params.ntracks = x->x_dbresultlength;
      break;
    case O2_TRACK_QUERY:
      spec.qid.sequence_length = 1;
      spec.params.accumulation = ADB_ACCUMULATION_PER_TRACK;
      spec.params.distance = ADB_DISTANCE_DOT_PRODUCT;
      spec.params.npoints = x->x_dbnumpoints;
      spec.params.ntracks = x->x_dbresultlength;
    case O2_SEQUENCE_QUERY:
    case O2_N_SEQUENCE_QUERY:
      spec.params.accumulation = ADB_ACCUMULATION_PER_TRACK;
      // TODO : Add unit norming param. Defaults to false.
      //
      // spec.params.distance = no_unit_norming ?
      // ADB_DISTANCE_EUCLIDEAN : ADB_DISTANCE_EUCLIDEAN_NORMED;
      spec.params.distance = ADB_DISTANCE_EUCLIDEAN_NORMED;
      spec.params.npoints = x->x_dbnumpoints;
      spec.params.ntracks = x->x_dbresultlength;
      break;
    case O2_ONE_TO_ONE_N_SEQUENCE_QUERY:
      spec.params.accumulation = ADB_ACCUMULATION_ONE_TO_ONE;
      // TODO : Add unit norming param. Defaults to false.
      //
      // spec.params.distance = no_unit_norming ?
      // ADB_DISTANCE_EUCLIDEAN : ADB_DISTANCE_EUCLIDEAN_NORMED;
      spec.params.distance =ADB_DISTANCE_EUCLIDEAN_NORMED;
      spec.params.npoints = 0;
      spec.params.ntracks = 0;
      break;
    default:
      post("Unsupported query type");
  }
 
  adb_query_results_t *rs = audiodb_query_spec(x->db, &spec);
  
  if(datum.data) {
    free(datum.data);
    datum.data = NULL;
  }
  if(datum.power) {
    free(datum.power);
    datum.power = NULL;
  }
  if(datum.times) {
    free(datum.times);
    datum.times = NULL;
  }
  
  if(rs == NULL)
  {
    error("Query failed");
    return;
  }

  int size = rs->nresults;
  post("result size:[%d]",(int)size);
  int i = 0;
  for(i=0; i<size; i++){
    adb_result_t r = rs->results[i];
    
    outlet_float(x->x_dist,r.dist);
    outlet_float(x->x_qpos,r.qpos);
    outlet_float(x->x_spos,r.ipos);
    
    post("in obj ikey:%s",r.ikey);
    post("in obj Dist:%f", r.dist);
    post("in obj qpos:%d", r.qpos);
    post("in obj ipos:%d", r.ipos);
  }
  audiodb_query_free_results(x->db, &spec, rs);
}

/* Do I need to set a the power file for this flag to work ?
   Hmmm. Dunno. Also, should be on/off*/
static void adbpd_power(t_adbpd *x){
  post("power");

  if (x->db && !(audiodb_power(x->db))){
    post("power successfully set on db");
  } else {
    error("power flag not working");
  }   
}

/* works fine but would be better if it took an argument to switch it
   on and off */
static void adbpd_l2norm(t_adbpd *x){
  post("l2norm");

  if (x->db && !(audiodb_l2norm(x->db))){
    post("l2norm successfully set on db");
  } else {
    error("l2norm flag not working");
  }   
}

/* reports the name of the current db */
static void adbpd_getname(t_adbpd *x){
  post("db name is '%s'",  x->x_dbname->s_name);	
}

/* this sets the qpoint for the current query*/
static void adbpd_setqpoint(t_adbpd *x,t_floatarg f){
  post("qpoint %.3f",f);
  x->x_dbqpoint=f; 
} 

/* This sets the number of points for current query */
static void adbpd_setnumpoints(t_adbpd *x,t_floatarg f){
  post("numpoints %.3f",f);
  x->x_dbnumpoints=(int)f;
}

/* This sets the radius */
static void adbpd_setradius(t_adbpd *x,t_floatarg f){
  post("radius %.3f",f);
  x->x_dbradius=f;
} 

/* This sets the result length */ 
static void adbpd_setresultlength(t_adbpd *x,t_floatarg f){
  post("resultlength %.3f",f);
  x->x_dbresultlength=(int)f;
}

/* This sets the sequence length */
static void adbpd_setsequencelength(t_adbpd *x,t_floatarg f){
  post("sequencelength %.3f",f);
  x->x_dbsequencelength=(int)f;
} 

/* This sets the feature file to use for the query */
static void adbpd_setfeatures(t_adbpd *x,t_symbol *s,int argc, t_atom *argv){

  if (argc == 1 && argv->a_type == A_SYMBOL){
    x->x_dbfeature=gensym(argv->a_w.w_symbol->s_name);
    post("Features file has been set to '%s'",x->x_dbfeature->s_name);
  }
}

/* This sets the query type */
static void adbpd_setquerytype(t_adbpd *x,t_symbol *s,int argc, t_atom *argv){ 
  if (argc == 1 && argv->a_type == A_SYMBOL) {
    t_symbol *type = atom_getsymbol(argv);
    if(type == gensym("track")) {
      x->x_dbquerytype=O2_TRACK_QUERY;
    } else if(type == gensym("point")) {
      x->x_dbquerytype=O2_POINT_QUERY;
    } else if(type == gensym("sequence")) {
      x->x_dbquerytype=O2_SEQUENCE_QUERY;
    } else if(type == gensym("nsequence")) {
      x->x_dbquerytype=O2_N_SEQUENCE_QUERY;
    } else if(type == gensym("onetoonensequence")) {
      x->x_dbquerytype=O2_ONE_TO_ONE_N_SEQUENCE_QUERY;
    } else {
      error("unsupported query type: '%s'", type->s_name);
      return;
    }
    post("Query type has been set to '%s'", type->s_name);
  }
}

/* This lists all the parameters */ 

static void adbpd_parameters(t_adbpd *x){
	
  post("dbname %s",x->x_dbname->s_name);
  post("querytype %d",x->x_dbquerytype);
  post("features %s",x->x_dbfeature->s_name);
  post("qpoint %.3f",x->x_dbqpoint);
  post("numpoints %.3f",x->x_dbnumpoints);
  post("radius %.3f",x->x_dbradius);
  post("resultlength %.3f",x->x_dbresultlength);
  post("sequencelength %.3f",x->x_dbsequencelength);
}

/* THAR SHE BLOWS. This sets up the object, takes the messages with
   args and maps them to methods. */
void adbpd_setup(void) {
  /* the arguments in this line refer to INPUT ARGUMENTS and not
     OUTPUTS */
  adbpd_class = 
    class_new(gensym("adbpd"), (t_newmethod) adbpd_new, (t_method) adbpd_free,
	      sizeof(t_adbpd), CLASS_DEFAULT, A_DEFSYMBOL, 0);

  /* all methods that respond to input must be defined here */
  class_addbang(adbpd_class, adbpd_bang);
  class_addfloat(adbpd_class, adbpd_setqpoint);
  class_addfloat(adbpd_class, adbpd_setnumpoints);
  class_addfloat(adbpd_class, adbpd_setradius);
  class_addfloat(adbpd_class, adbpd_setresultlength);
  class_addfloat(adbpd_class, adbpd_setsequencelength);
  class_addmethod(adbpd_class, (t_method)adbpd_setname, gensym("set"), A_GIMME, 0);
  class_addmethod(adbpd_class, (t_method)adbpd_getname, gensym("get"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_create, gensym("create"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(adbpd_class, (t_method)adbpd_open, gensym("open"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_close, gensym("close"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_status, gensym("status"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_l2norm, gensym("l2norm"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_power, gensym("power"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_doquery, gensym("doquery"), A_NULL);
  class_addmethod(adbpd_class, (t_method)adbpd_setquerytype, gensym("querytype"), A_GIMME, 0);
  class_addmethod(adbpd_class, (t_method)adbpd_setfeatures, gensym("features"), A_GIMME, 0);
  class_addmethod(adbpd_class, (t_method)adbpd_parameters, gensym("parameters"), A_NULL);
}

