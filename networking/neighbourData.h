#include "LayerConfig.h"

#ifndef NEIGHBOURDATA_H_
#define NEIGHBOURDATA_H_

class neighbourData {
	public:
		l3_address_t nodeId;
		uint8_t hopCount;
		l3_address_t hopNextNodeId;
		l3_timestamp timestamp;
};

#endif
