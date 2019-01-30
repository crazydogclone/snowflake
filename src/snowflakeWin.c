
#include <node_api.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include <stdlib.h>
#include <math.h>

#include "./common.h"

//各个数据所在位偏移
int workerIdBits = 5;
int dataCenterIdBits = 5;
int sequenceBits = 12;

int maxWorkerId = -1;
int maxDataCenterId = -1;
int sequenceMask = -1;

int workerIdShift, dataCenterIdShift, timestampLeftShift;

uint64_t lastTimestamp = (uint64_t)0;

unsigned int sequence = 0;

 void timeGen(uint64_t* time){
	SYSTEMTIME tv;
	GetSystemTime(&tv); 
 	*time = (uint64_t)tv.wSecond*(uint64_t)1000 + (uint64_t)tv.wMilliseconds%(uint64_t)1000;
 }

 uint64_t tilNextMillis(uint64_t* lastTimestamp){
 	uint64_t timestamp;
 	timeGen(&timestamp);
 	while (timestamp <= *lastTimestamp) {
     timeGen(&timestamp);
   }
   return timestamp;
 }

char* i64toa(uint64_t src, char* dest, int dest_len){
  int index = -1,value = 0;

  for (int i = 0;i < dest_len;++i)
  {
    value = src % 10;
    if(value != 0) index = dest_len - i - 1;
    dest[dest_len - i - 1] = (char)('0' + src % 10);
    src /= 10;
  }

  if(index != 0)
  {
    for (int i = index; i < dest_len; ++i)
    {
      *(dest + i - index) = *(dest + i);
    }
    *(dest + dest_len - index) = '\0';
  }

  if(index == -1)
  {
    *(dest) = '0';
    *(dest + 1) = '\0';
  }

  return dest;
}

napi_value MaxWorkerId(napi_env env, napi_callback_info info) {
  napi_value res;

  if(maxWorkerId == -1){
    maxWorkerId = pow(2, workerIdBits) - 1;
  }

  NAPI_CALL(env, napi_create_int32(env, maxWorkerId, &res));

  return res;
}

napi_value MaxDataCenterId(napi_env env, napi_callback_info info) {
  napi_value res;

  if(maxDataCenterId == -1){
    maxDataCenterId = pow(2, dataCenterIdBits) - 1;
  }

  NAPI_CALL(env, napi_create_int32(env, maxDataCenterId, &res));

  return res;
}

napi_value init(napi_env env, napi_callback_info info) {
  napi_value res;
  
  maxWorkerId = pow(2, workerIdBits) - 1;
  maxDataCenterId = pow(2, dataCenterIdBits) - 1;
  sequenceMask = pow(2, sequenceBits) - 1;

  workerIdShift = sequenceBits;
  dataCenterIdShift = sequenceBits + workerIdBits;
  timestampLeftShift = sequenceBits + workerIdBits + dataCenterIdBits;

  NAPI_CALL(env, napi_create_int32(env, 1, &res));

  return res;
}

napi_value nextId(napi_env env, napi_callback_info info) {
    napi_value arg[2], argThis, result;
    size_t arg_len = 2;
    NAPI_CALL(env, napi_get_cb_info(env, info, &arg_len, &arg, &argThis, NULL));
    int workerId,dataCenterId;
    uint64_t timestamp = (uint64_t)0;

    NAPI_CALL(env, napi_get_value_int32(env, arg[0], &workerId));
    NAPI_CALL(env, napi_get_value_int32(env, arg[1], &dataCenterId));
//    NAPI_CALL(env, napi_get_value_int64(env, arg[2], &timestamp));

    timeGen(&timestamp);
    if(lastTimestamp == timestamp){
    	sequence = (sequence + 1) & sequenceMask;
    	if(sequence == 0){
    	    timestamp = tilNextMillis(&lastTimestamp);
//        NAPI_CALL(env, napi_create_int64(env, -1, &result));
//        return result;
    	}
    }else{
    	sequence = 0;
    }

    if (timestamp < lastTimestamp) {
      napi_throw_error(env, NULL, "Clock moved backwards.");
    }

    lastTimestamp = timestamp;

    uint64_t i64nextId = ((timestamp) << timestampLeftShift | (dataCenterId << dataCenterIdShift) |
    	(workerId << workerIdShift) | sequence);

    char res[20] = "\0";
    i64toa(i64nextId, res, 19);

//    NAPI_CALL(env, napi_create_object(env, &result));
//
//    napi_value int64, string;
//
//    NAPI_CALL(env, napi_create_int64(env, i64nextId, &int64));
//    NAPI_CALL(env, napi_create_string_utf8(env, res, NAPI_AUTO_LENGTH, &string));
//
//    NAPI_CALL(env, napi_set_named_property(env, result, "int64", int64));
//    NAPI_CALL(env, napi_set_named_property(env, result, "string", string));

    NAPI_CALL(env, napi_create_string_utf8(env, res, NAPI_AUTO_LENGTH, &result));

    return result;
}


napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
  		DECLARE_NAPI_PROPERTY("nextId", nextId),
  		DECLARE_NAPI_PROPERTY("init", init),
  		DECLARE_NAPI_PROPERTY("maxWorkerId", MaxWorkerId),
  		DECLARE_NAPI_PROPERTY("maxDataCenterId", MaxDataCenterId)
  	};
  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)