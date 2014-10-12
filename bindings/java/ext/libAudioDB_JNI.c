#include <sys/stat.h>
#include "org_omras2_AudioDB.h"
#include "org_omras2_AudioDB_Mode.h"
#include <jni.h>
#include "audioDB_API.h"
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

// Following Ben's lead here!
#define ADB_HEADER_FLAG_L2NORM 0x1
#define ADB_HEADER_FLAG_POWER 0x4
#define ADB_HEADER_FLAG_TIMES 0x20
#define ADB_HEADER_FLAG_REFERENCES 0x40

jdoubleArray get_double_array(JNIEnv *env, jclass classValue, jobject objectValue, char* field)
{
	jfieldID fid = (*env)->GetFieldID(env, classValue, field, "[D");
	if(fid == NULL) return NULL;
	jdoubleArray arr = (*env)->GetObjectField(env, objectValue, fid);
	return arr;
}

int get_int_val(JNIEnv *env, jclass classValue, jobject objectValue, char* field, int defaultValue)
{
	jfieldID fid = (*env)->GetFieldID(env, classValue, field, "I");
	if(fid == NULL) return defaultValue;
	int val = (*env)->GetIntField(env, objectValue, fid);
	return (val ? val : defaultValue);
}

double get_double_val(JNIEnv *env, jclass classValue, jobject objectValue, char* field, double defaultValue)
{
	jfieldID fid = (*env)->GetFieldID(env, classValue, field, "D");
	if(fid == NULL) return defaultValue;
	double val = (*env)->GetDoubleField(env, objectValue, fid);
	return (val ? val : defaultValue);
}

const char* get_enum_val(JNIEnv *env, jclass queryClass, jobject queryObject, char* fieldName, char* enumClassName)
{
	char signature[strlen(enumClassName)+2];
	sprintf(signature, "L%s;", enumClassName);

	// Get the value of the enum field
	jfieldID fid = (*env)->GetFieldID(env, queryClass, fieldName, signature); 
	if(fid == NULL) return;

	jobject field = (*env)->GetObjectField(env, queryObject, fid);
	if(field == NULL) return;

	// Retrieve the string value via name()
	jclass enumClass = (*env)->FindClass(env, enumClassName);
	if(enumClass == NULL) return;

	jmethodID getNameMethod = (*env)->GetMethodID(env, enumClass, "name", "()Ljava/lang/String;");
	if(getNameMethod == NULL) return;

	jstring value = (jstring)((*env)->CallObjectMethod(env, field, getNameMethod));
	if(value == NULL) return;
	
	return (*env)->GetStringUTFChars(env, value, 0);
}

adb_t* get_handle(JNIEnv *env, jobject obj)
{
	// Fetch the adb pointer

	adb_t *handle;
	
	jclass adbClass = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, adbClass, "adbHandle", "J");
	if(fid == NULL) {
		return;
	}
	
	handle = (adb_t*)((*env)->GetLongField(env, obj, fid)); 
	(*env)->DeleteLocalRef(env, adbClass);
	return handle;	
}

JNIEXPORT jboolean JNICALL Java_org_omras2_AudioDB_audiodb_1create (JNIEnv *env, jobject obj, jstring path, jint datasize, jint ntracks, jint datadim)
{
	char buf[256];
	const char *str;
	str = (*env)->GetStringUTFChars(env, path, NULL);
	if (str == NULL)
		return;

	adb_t *handle;
	handle = audiodb_create(str, datasize, ntracks, datadim);
	if(!handle)
		return JNI_FALSE;
	
	(*env)->ReleaseStringUTFChars(env, path, str);
	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_org_omras2_AudioDB_audiodb_1open (JNIEnv *env, jobject obj, jstring path, jobject mode)
{
	// TODO: If we have a handle, close the old one.
	if(get_handle(env, obj))
		return;

	jclass modeClass = (*env)->FindClass(env, "org/omras2/AudioDB$Mode");
	jmethodID getNameMethod = (*env)->GetMethodID(env, modeClass, "name", "()Ljava/lang/String;");
	jstring value = (jstring)(*env)->CallObjectMethod(env, mode, getNameMethod);
	const char* openMode = (*env)->GetStringUTFChars(env, value, 0);
	const char* pathVal = (*env)->GetStringUTFChars(env, path, 0);
	int modeVal = 0;
	if(strcmp(openMode, "O_RDWR") == 0)
	{
		modeVal = O_RDWR;
	}
	else if(strcmp(openMode, "O_RDONLY") == 0)
	{
		modeVal = O_RDONLY;
	}
	else
		return;

	adb_t *handle;
	handle = audiodb_open(pathVal, modeVal);
	jclass adbClass = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, adbClass, "adbHandle", "J");
	if(fid == NULL) {
		return;
	}
	(*env)->SetLongField(env, obj, fid, (long)handle);
	(*env)->DeleteLocalRef(env, adbClass);

	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_org_omras2_AudioDB_audiodb_1close (JNIEnv *env, jobject obj)
{
	adb_t *handle = get_handle(env, obj);
	if(!handle)
		return;

	audiodb_close(handle);
	
	jclass adbClass = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, adbClass, "adbHandle", "J");
	
	if(fid == NULL) {
		return;
	}

	(*env)->SetLongField(env, obj, fid, 0);
	(*env)->DeleteLocalRef(env, adbClass);
}

JNIEXPORT jboolean JNICALL Java_org_omras2_AudioDB_audiodb_1insert_1data(JNIEnv *env, jobject obj, jstring key, int nvectors, int dim, jdoubleArray features, jdoubleArray power, jdoubleArray times)
{
	adb_t *handle = get_handle(env, obj);
	if(!handle)
		return JNI_FALSE;
	
	adb_datum_t* ins = (adb_datum_t *)malloc(sizeof(adb_datum_t));
	if(!features || !key)
		return JNI_FALSE;

	ins->data = (*env)->GetDoubleArrayElements(env, features, NULL); 
	ins->power = NULL;
	ins->times = NULL;

	if(power)
		ins->power = (*env)->GetDoubleArrayElements(env, power, NULL); 
	if(times)
		ins->times = (*env)->GetDoubleArrayElements(env, times, NULL); 

	ins->key = (*env)->GetStringUTFChars(env, key, 0); 
	ins->nvectors = nvectors;
	ins->dim = dim;

	int result =  audiodb_insert_datum(handle, ins);
	if(result)
		return JNI_FALSE;

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_org_omras2_AudioDB_audiodb_1insert_1path(JNIEnv *env, jobject obj, jstring key, jstring features, jstring power, jstring times)
{
	adb_t *handle = get_handle(env, obj);
	if(!handle)
		return JNI_FALSE;
	
	adb_insert_t* ins = (adb_insert_t *)malloc(sizeof(adb_insert_t));
	ins->key = NULL;
	ins->features = NULL;
	ins->power = NULL;
	ins->times = NULL;

	if(key)
		ins->key = (*env)->GetStringUTFChars(env, key, 0);
	if(features)
		ins->features = (*env)->GetStringUTFChars(env, features, 0);
	if(power)
		ins->power = (*env)->GetStringUTFChars(env, power, 0);
	if(times)
		ins->times = (*env)->GetStringUTFChars(env, times, 0);

	int result = audiodb_insert(handle, ins);

	if(result)
		return JNI_FALSE;
	
	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_org_omras2_AudioDB_query(JNIEnv *env, jobject obj)
{
}

JNIEXPORT jobject JNICALL Java_org_omras2_AudioDB_audiodb_1status(JNIEnv *env, jobject obj)
{
	adb_t *handle = get_handle(env, obj);
	if(!handle)
		return NULL;
	adb_status_t *status;
	status = (adb_status_t *)malloc(sizeof(adb_status_t));
	int flags = audiodb_status(handle, status);	
	
	jclass statusClass = (*env)->FindClass(env, "org/omras2/Status");
	if(statusClass == NULL) {
		return NULL;
	}	
	jmethodID cid = (*env)->GetMethodID(env, statusClass, "<init>", "()V");
	if(cid == NULL) {
		return NULL;
	}

	jobject result = (*env)->NewObject(env, statusClass, cid);

	// This needs a macro!
	jfieldID fid = (*env)->GetFieldID(env, statusClass, "numFiles", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->numFiles);
	
	fid = (*env)->GetFieldID(env, statusClass, "dim", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->dim);
	
	fid = (*env)->GetFieldID(env, statusClass, "dudCount", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->dudCount);
	
	fid = (*env)->GetFieldID(env, statusClass, "nullCount", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->nullCount);
	
	fid = (*env)->GetFieldID(env, statusClass, "length", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->length);
	
	fid = (*env)->GetFieldID(env, statusClass, "dataRegionSize", "I");
	if(fid == NULL) return;
	(*env)->SetIntField(env, result, fid, status->data_region_size);

	// Flags
	
	fid = (*env)->GetFieldID(env, statusClass, "isL2Normed", "Z");
	if(fid == NULL) return;
	(*env)->SetBooleanField(env, result, fid, (status->flags & ADB_HEADER_FLAG_L2NORM));
	
	fid = (*env)->GetFieldID(env, statusClass, "hasPower", "Z");
	if(fid == NULL) return;
	(*env)->SetBooleanField(env, result, fid, (status->flags & ADB_HEADER_FLAG_POWER));
	
	fid = (*env)->GetFieldID(env, statusClass, "hasTimes", "Z");
	if(fid == NULL) return;
	(*env)->SetBooleanField(env, result, fid, (status->flags & ADB_HEADER_FLAG_TIMES));
	
	fid = (*env)->GetFieldID(env, statusClass, "hasReferences", "Z");
	if(fid == NULL) return;
	(*env)->SetBooleanField(env, result, fid, (status->flags & ADB_HEADER_FLAG_REFERENCES));

	(*env)->DeleteLocalRef(env, statusClass);

	return result;
}

jobject build_results(JNIEnv *env, adb_query_results_t *result)
{
	int i;
	// Create a vector
	jclass vectorClass = (*env)->FindClass(env, "java/util/Vector");
	if(vectorClass == NULL) return;
	jmethodID vectorCtor = (*env)->GetMethodID(env, vectorClass, "<init>", "()V");
	if(vectorCtor == NULL) return;
	jobject vector = (*env)->NewObject(env, vectorClass, vectorCtor);
	
	jmethodID addElementMethod = (*env)->GetMethodID(env, vectorClass, "addElement", "(Ljava/lang/Object;)V");
	if(addElementMethod == NULL) return;

	jclass resultClass = (*env)->FindClass(env, "org/omras2/Result");
	if(resultClass == NULL) return;
	jmethodID resultCtor = (*env)->GetMethodID(env, resultClass, "<init>", "(Ljava/lang/String;DII)V");
	if(resultCtor == NULL) return;

	for(i=0; i<result->nresults; i++)
	{
		jstring keyStr = (*env)->NewStringUTF(env, result->results[i].ikey);
		jobject resultObj = (*env)->NewObject(env, resultClass, resultCtor, 
			keyStr, 
			(double)(result->results[i].dist),
			(int)(result->results[i].qpos),
			(int)(result->results[i].ipos));
	
		(*env)->CallObjectMethod(env, vector, addElementMethod, resultObj);
	
	}
	(*env)->DeleteLocalRef(env, resultClass);
	(*env)->DeleteLocalRef(env, vectorClass);
	return vector;
}

JNIEXPORT jobject JNICALL Java_org_omras2_AudioDB_audiodb_1query(JNIEnv *env, jobject adbobj, jstring key, jobject queryObject)
{
	adb_t* handle = get_handle(env, adbobj);

	if(!handle)
		return;
	
	jclass queryClass = (*env)->GetObjectClass(env, queryObject);

	jclass datumClass = (*env)->FindClass(env, "org/omras2/Datum");	
	jfieldID datumFid = (*env)->GetFieldID(env, queryClass, "datum", "Lorg/omras2/Datum;"); 
	if(datumFid == NULL) return;

	jobject datumObject = (*env)->GetObjectField(env, queryObject, datumFid);
	if(datumObject == NULL) return;
	
	adb_query_spec_t* spec = (adb_query_spec_t *)malloc(sizeof(adb_query_spec_t));
	spec->qid.datum = (adb_datum_t *)malloc(sizeof(adb_datum_t));
	adb_query_results_t* result = (adb_query_results_t *)malloc(sizeof(adb_query_results_t));

	// As in python bindings
	spec->refine.flags = 0;

	spec->qid.sequence_length = get_int_val(env, queryClass, queryObject, "seqLength", 16);
	spec->qid.sequence_start = get_int_val(env, queryClass, queryObject, "seqStart", 0);
	spec->params.npoints = get_int_val(env, queryClass, queryObject, "npoints", 1);
	spec->params.ntracks = get_int_val(env, queryClass, queryObject, "ntracks", 100);
	
	jfieldID fid = (*env)->GetFieldID(env, queryClass, "exhaustive", "Z");
	if(fid == NULL) return;
	if((*env)->GetBooleanField(env, queryObject, fid)) {
		spec->qid.flags = spec->qid.flags | ADB_QID_FLAG_EXHAUSTIVE;
	}
	
	fid = (*env)->GetFieldID(env, queryClass, "hasFalsePositives", "Z");
	if(fid == NULL) return;
	if((*env)->GetBooleanField(env, queryObject, fid)) {
		spec->qid.flags = spec->qid.flags | ADB_QID_FLAG_ALLOW_FALSE_POSITIVES;
	}

	const char* accType = get_enum_val(env, queryClass, queryObject, "accumulation", "org/omras2/Query$Accumulation"); 
	const char* distType = get_enum_val(env, queryClass, queryObject, "distance", "org/omras2/Query$Distance"); 
	
	if(strcmp(accType, "DB") == 0)
		spec->params.accumulation = ADB_ACCUMULATION_DB;
	else if(strcmp(accType, "PER_TRACK") == 0)
		spec->params.accumulation = ADB_ACCUMULATION_PER_TRACK;
	else if(strcmp(accType, "ONE_TO_ONE") == 0)
		spec->params.accumulation = ADB_ACCUMULATION_ONE_TO_ONE;
	else {
		return;
	}	
	
	if(strcmp(distType, "DOT_PRODUCT") == 0)
		spec->params.distance = ADB_DISTANCE_DOT_PRODUCT;
	else if(strcmp(distType, "EUCLIDEAN_NORMED") == 0)
		spec->params.distance = ADB_DISTANCE_EUCLIDEAN_NORMED;
	else if(strcmp(distType, "EUCLIDEAN") == 0)
		spec->params.distance = ADB_DISTANCE_EUCLIDEAN;
	else {
		return;
	}	
	
	int i;
	
	fid = (*env)->GetFieldID(env, queryClass, "includeKeys", "[Ljava/lang/String;");
	if(fid == NULL) return;
	jobjectArray includeObj = (jobjectArray)((*env)->GetObjectField(env, queryObject, fid));
	jsize len = (*env)->GetArrayLength(env, includeObj);
	
	if(len > 0)
	{
		spec->refine.flags |= ADB_REFINE_INCLUDE_KEYLIST;
		spec->refine.include.nkeys = len;
		spec->refine.include.keys = (const char **)calloc(sizeof(const char *),  spec->refine.include.nkeys);
		for(i=0; i<spec->refine.include.nkeys; i++) {
			jstring key = (*env)->GetObjectArrayElement(env, includeObj, i);
			spec->refine.include.keys[i] = (*env)->GetStringUTFChars(env, key, NULL);
		}
	}
	
	fid = (*env)->GetFieldID(env, queryClass, "excludeKeys", "[Ljava/lang/String;");
	if(fid == NULL) return;
	jobjectArray excludeObj = (jobjectArray)((*env)->GetObjectField(env, queryObject, fid));
	len = (*env)->GetArrayLength(env, excludeObj);
	
	if(len > 0)
	{
		spec->refine.flags |= ADB_REFINE_EXCLUDE_KEYLIST;
		spec->refine.exclude.nkeys = len;
		spec->refine.exclude.keys = (const char **)calloc(sizeof(const char *),  spec->refine.exclude.nkeys);
		for(i=0; i<spec->refine.exclude.nkeys; i++) {
			jstring key = (*env)->GetObjectArrayElement(env, excludeObj, i);
			spec->refine.exclude.keys[i] = (*env)->GetStringUTFChars(env, key, NULL);
		}
	}

	// Rest of refine

	double radius = get_double_val(env, queryClass, queryObject, "radius", 0);
	double absThres = get_double_val(env, queryClass, queryObject, "absThres", 0);
	double relThres = get_double_val(env, queryClass, queryObject, "relThres", 0);
	double durRatio = get_double_val(env, queryClass, queryObject, "durRatio", 0);
	int hopSize = get_int_val(env, queryClass, queryObject, "hopSize", 0);

	if(radius) {
		spec->refine.flags |= ADB_REFINE_RADIUS;
		spec->refine.radius = radius;
	}
	
	if(absThres) {
		spec->refine.flags |= ADB_REFINE_ABSOLUTE_THRESHOLD;
		spec->refine.absolute_threshold = absThres;
	}
	
	if(relThres) {
		spec->refine.flags |= ADB_REFINE_RELATIVE_THRESHOLD;
		spec->refine.relative_threshold = relThres;
	}
	
	if(durRatio) {
		spec->refine.flags |= ADB_REFINE_DURATION_RATIO;
		spec->refine.duration_ratio = durRatio;
	}
	
	if(hopSize) {
		spec->refine.flags |= ADB_REFINE_HOP_SIZE;
		spec->refine.qhopsize = hopSize;
		spec->refine.ihopsize = hopSize;
	}

	spec->qid.datum->data = NULL;
	spec->qid.datum->power = NULL;
	spec->qid.datum->times = NULL;

	if (key != NULL)
	{
		const char* keyStr = (*env)->GetStringUTFChars(env, key, NULL);
		if(keyStr != NULL)
		{
			int ok = audiodb_retrieve_datum(handle, keyStr, spec->qid.datum);
			if(ok != 0) 
				return;
		}
	}
	else
	{
		jdoubleArray data = get_double_array(env, datumClass, datumObject, "data");
		jdouble *dataBody = (*env)->GetDoubleArrayElements(env, data, 0);
		spec->qid.datum->data = dataBody;
		
		jdoubleArray power = get_double_array(env, datumClass, datumObject, "power");
		jdouble *powerBody = (*env)->GetDoubleArrayElements(env, power, 0);
		spec->qid.datum->power = powerBody;
		
		jdoubleArray times = get_double_array(env, datumClass, datumObject, "times");
		jdouble *timesBody = (*env)->GetDoubleArrayElements(env, times, 0);
		spec->qid.datum->times = timesBody;

		spec->qid.datum->nvectors = get_int_val(env, datumClass, datumObject, "nvectors", 0);
		spec->qid.datum->dim = get_int_val(env, datumClass, datumObject, "dim", 0);

		(*env)->ReleaseDoubleArrayElements(env, data, dataBody, 0);
		(*env)->ReleaseDoubleArrayElements(env, power, powerBody, 0);
		(*env)->ReleaseDoubleArrayElements(env, times, timesBody, 0);
	}

	result = audiodb_query_spec(handle, spec);
	
	if(result == NULL) {
		return;
	}
	(*env)->DeleteLocalRef(env, queryClass);
	(*env)->DeleteLocalRef(env, datumClass);


	return build_results(env, result);
}

