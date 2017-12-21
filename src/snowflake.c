
#include <node_api.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include "./common.h"


__int64_t twepoch = (__int64_t)1288834974657;
//各个数据所在位偏移
unsigned int workerIdBits = 5;
unsigned int dataCenterIdBits = 5;
unsigned int sequenceBits = 12;

int maxWrokerId = 1;
int maxDataCenterId = 1;
int sequenceMask = 1;

int workerIdShift, dataCenterIdShift, timestampLeftShift;

__int64_t lastTimestamp = (__int64_t)0;
__int64_t aux = (__int64_t)18014398509481983;

unsigned int sequence = 0;

// int timeGen(__int64_t* time){
// 	struct timeval tv;
// 	int res;
// 	res = gettimeofday(&tv, NULL);
// 	if(!!res){
// 		*time = (__int64_t)tv.tv_sec*1000 + (__int64_t)tv.tv_usec/1000;
// 	}
// 	return res;
// }

// __int64_t tilNextMillis(__int64_t lastTimestamp){
// 	__int64_t timestamp;
// 	timeGen(&timestamp);
// 	while (timestamp <= lastTimestamp) {
//     timeGen(&timestamp);
//   }
//   return timestamp;
// }

// char* int64toa(__int64_t src, char* dest, int radix){
// 	return _i64toa( src, dest, radix );
// }

napi_value init(napi_env env, napi_callback_info info) {
  napi_value res;
  
  maxWrokerId = pow(2, workerIdBits) - 1;
  maxDataCenterId = pow(2, dataCenterIdBits) - 1;
  sequenceMask = pow(2, sequenceBits) - 1;

  workerIdShift = sequenceBits;
  dataCenterIdShift = sequenceBits + workerIdBits;
  timestampLeftShift = sequenceBits + workerIdBits + dataCenterIdBits;

  NAPI_CALL(env, napi_create_int32(env, 1, &res));

  return res;
}

napi_value nextId(napi_env env, napi_callback_info info) {
  napi_value arg[3], argThis, result;
  size_t arg_len = 3;	
  NAPI_CALL(env, napi_get_cb_info(env, info, &arg_len, &arg, &argThis, NULL));
  int workerId,dataCenterId;
  __int64_t timestamp ;

  NAPI_CALL(env, napi_get_value_int32(env, arg[0], &workerId));
  NAPI_CALL(env, napi_get_value_int32(env, arg[1], &dataCenterId));
  NAPI_CALL(env, napi_get_value_int64(env, arg[2], &timestamp));

  //timeGen(&timestamp);
  if(lastTimestamp == timestamp){
  	sequence = (sequence + 1) & sequenceMask;
  	if(sequence == 0){
  		//timestamp = tilNextMillis(lastTimestamp);
      NAPI_CALL(env, napi_create_int64(env, -1, &result));
      return result;
  	}
  }else{
  	sequence = 0;
  }

  if (timestamp < lastTimestamp) {
    napi_throw_error(env, NULL, "Clock moved backwards.");
  }

  lastTimestamp = timestamp;

  __int64_t i64nextId = ((timestamp) << timestampLeftShift | (dataCenterId << dataCenterIdShift) |
  	(workerId << workerIdShift) | sequence);

  //__int64_t i64nextId = ((timestamp) << timestampLeftShift);

  i64nextId = (i64nextId & aux);

  NAPI_CALL(env, napi_create_int64(env, i64nextId, &result));

  return result;
}


napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
  		DECLARE_NAPI_PROPERTY("nextId", nextId),
  		DECLARE_NAPI_PROPERTY("init", init)
  	};
  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)