// audioDBws.h -- web services interface to audioDB
//
//FIXME: this hard-coding of the service location might be right for
//its internal use at Goldsmiths (for now) but really isn't in
//general.  Find a way to bind this later (at install time?  Or maybe
//just require that the installer edit the resulting wsdl file?)
//
//gsoap adb service location: http://gibbons.doc.gold.ac.uk:20703/

typedef int xsd__int;
typedef double xsd__double;
typedef char* xsd__string;

// Supports result lists of arbitrary length
struct adb__queryResult{
  int __sizeRlist;
  char **Rlist; // Maximum size of result list
  int __sizeDist;  
  double *Dist;
  int __sizeQpos;  
  unsigned int *Qpos;
  int __sizeSpos;
  unsigned int *Spos;
};

struct adb__statusResult {
  unsigned numFiles;
  unsigned dim;
  unsigned length;
  unsigned dudCount;
  unsigned nullCount;
  unsigned flags;
};

struct adb__statusResponse {
  struct adb__statusResult result;
};

struct adb__queryResponse {
  struct adb__queryResult result;
};

struct adb__lisztResult {
  int __sizeRkey;
  char **Rkey;
  int __sizeRlen;
  unsigned int *Rlen;
};

struct adb__queryVector {
  int dim; // dimensionality of the feature (d)
  int __sizev; // l x d :
  double *v;   // pointer to query data
  int __sizep;
  double *p;
};

struct adb__lisztResponse {
  struct adb__lisztResult result;
};

// Print the status of an existing adb database
int adb__status(xsd__string dbName, struct adb__statusResponse &adbStatusResponse);

// Print the status of an existing adb database
int adb__liszt(xsd__string dbName, xsd__int lisztOffset, xsd__int lisztLength, struct adb__lisztResponse &adbLisztResponse);

// Query an existing adb database
int adb__query(xsd__string dbName, xsd__string qKey, xsd__string keyList, xsd__string timesFileName, xsd__string powerFileName, xsd__int qType, xsd__int qPos, xsd__int pointNN, xsd__int segNN, xsd__int segLen, xsd__double radius, xsd__double absolute_threshold, xsd__double relative_threshold, xsd__int exhaustive, xsd__int lsh_exact, xsd__int no_unit_norming, struct adb__queryResponse &adbQueryResponse);

int adb__sequenceQueryByKey(xsd__string dbName,xsd__string trackKey, xsd__string featureFileName, xsd__int queryType,xsd__string trackFileName,xsd__string timesFileName,xsd__int queryPoint,xsd__int pointNN,xsd__int trackNN,xsd__int sequenceLength,xsd__double radius,xsd__double absolute_threshold,xsd__int usingQueryPoint,xsd__int lsh_exact, xsd__int no_unit_norming, struct adb__queryResponse &adbQueryResponse);

// Query an audioDB database by vector (serialized), queryKey/featureFileName is here replaced with qVector
int adb__shingleQuery(xsd__string dbName, struct adb__queryVector qVector, xsd__string keyList, xsd__string timesFileName, xsd__int queryType, xsd__int queryPos, xsd__int pointNN, xsd__int trackNN, xsd__int sequenceLength, xsd__double radius, xsd__double absolute_threshold, xsd__double relative_threshold, xsd__int exhaustive, xsd__int lsh_exact, xsd__int no_unit_norming, struct adb__queryResponse &adbQueryResponse);

