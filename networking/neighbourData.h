
#ifndef NEIGHBOURDATA_H_
#define NEIGHBOURDATA_H_

#include "LayerConfig.h"
#include "Layer3.h"

class NeighbourManager {
	public:
		typedef struct neighbourData_struct {
			public:
			l3_address_t nodeId;
			uint8_t hopCount;
			l3_address_t hopNextNodeId;
			l3_timestamp timestamp;
		} neighbourData_t;

		neighbourData_t neighbours[CONFIG_L3_NUM_NEIGHBOURS];

		const static uint8_t NeighbourListSize = CONFIG_L3_NUM_NEIGHBOURS;

		l3_address_t localAddress;

		void init(l3_address_t localAddress) {
			this->localAddress = localAddress;
		}

		NeighbourManager() {
			memset(neighbours, 0, sizeof(neighbours));
			//set hopcount to max!
			for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
				neighbours[i].hopCount = 0xff;
			}
		}

		neighbourData_t* getNeighbours() {
			return neighbours;
		}

		/**
		* get neighbour row.
		* @param neighbour
		*/
		neighbourData_t* getNeighbour(l3_address_t destination) {
			for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
				if(neighbours[i].nodeId == destination)
				return &neighbours[i];
			}

			return NULL;
		}

		/**
		 * update/add new neighbour
		 * @param packet
		 * @return success
		 */
		boolean updateNeighbour( l3_address_t address ) {
			return updateNeighbour(address, address, 0);
		}

		/**
		 * @param neighbour addrtess
		 * @param next hop
		 * @param hops
		 * @return success
		 */
		boolean updateNeighbour( l3_address_t destination, l3_address_t nextHop, uint8_t hopCount) {
			#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(millis());
			Serial.print(F(": updateNeighbour()"));
			Serial.print(F(" destination="));
			Serial.print(destination);
			Serial.print(F(" nextHop="));
			Serial.print(nextHop);
			Serial.print(F(" hopCount="));
			Serial.println(hopCount);
			Serial.flush();
			#endif

			if(destination == localAddress) {
				#ifdef DEBUG_NETWORK_ENABLE
				Serial.println(F("\tthis is us, no routing update!"));
				#endif
				return true;
			}
			if(hopCount > CONFIG_L3_MAX_HOPCOUNT) {
				#ifdef DEBUG_NETWORK_ENABLE
				Serial.println(F("\ttoo many hops, discard."));
				#endif
				return true;
			}

			neighbourData_t* n = getNeighbour(destination);

			//we do not have it yet - create.
			if(n == NULL) {
				//search free index
				uint8_t freeIndex = 0xff;
				for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
					if(neighbours[i].nodeId == 0) {
						freeIndex = i;
						break;
					}
				}

				//do we have space?
				if(freeIndex == 0xff) {
					return false;
					#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F("\ttable full."));
					Serial.flush();
					#endif
				}

				n = &neighbours[freeIndex];

				#ifdef DEBUG_NETWORK_ENABLE
				Serial.print(F("\tnew entry@index="));
				Serial.print(freeIndex);
				Serial.print(F(": "));
				Serial.flush();
				#endif
			}
			#ifdef DEBUG_NETWORK_ENABLE
			else {
				Serial.print(F("\texisting: "));
				Serial.flush();
			}
			#endif

			//update data - if better.
			#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(F("currentHopCount="));
			Serial.print(n->hopCount);
			Serial.print(F(" newHopCount="));
			Serial.print(hopCount);
			Serial.flush();
			#endif

			if(hopCount < n->hopCount)  {
				n->nodeId = destination;
				n->timestamp = millis();
				n->hopNextNodeId = nextHop;
				n->hopCount = hopCount;

				#ifdef DEBUG_NETWORK_ENABLE
				Serial.println(F(" - updated."));
				Serial.flush();
				#endif
				} else if(hopCount == n->hopCount && nextHop == n->hopNextNodeId) {
				n->timestamp = millis();
				#ifdef DEBUG_NETWORK_ENABLE
				Serial.println(F(" - same information, updated timestamp."));
				Serial.flush();
				#endif
			}
			#ifdef DEBUG_NETWORK_ENABLE
			else
			Serial.println(F(" - not updated."));
			Serial.flush();
			#endif

			//handled, return true.
			return true;
		}

		/**
		 * update neighbours
		 * @param beacon
		 * @return success
		 */
		boolean updateNeighbours(packet_beacon_t* beacon) {
			uint8_t num = beacon->numNeighbourInfo;

			boolean result = true;
			for(uint8_t i = 0; i < num; i++) {
				result |= updateNeighbour(beacon->neighbours[i].nodeId, beacon->nodeId, beacon->neighbours[i].hopcount);
			}

			return result;
		}

		/**
		 * neighbourtable maintenance, to be called periodically
		 */
		void cleanNeighbours() {
			#ifdef DEBUG_NETWORK_ENABLE
			Serial.print(millis());
			Serial.println(F(": cleanNeighbours()"));
			Serial.flush();
			#endif

			for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
				//do we have a valid neighbour?
				if(neighbours[i].nodeId > 0) {
					#ifdef DEBUG_NETWORK_ENABLE
					Serial.print(F("\tindex="));
					Serial.print(i);
					Serial.print(F(" nodeId="));
					Serial.print(neighbours[i].nodeId);
					Serial.print(F(" timestamp="));
					Serial.print(neighbours[i].timestamp);
					Serial.print(F(" hops="));
					Serial.print(neighbours[i].hopCount);
					Serial.print(F(" nextHop="));
					Serial.print(neighbours[i].hopNextNodeId);
					#endif

					if(millis() - neighbours[i].timestamp > CONFIG_L3_NEIGHBOUR_MAX_AGE_MS) {
						//remove
						memset(&neighbours[i], 0, sizeof(neighbourData_t));
						neighbours[i].hopCount = 0xff;

						#ifdef DEBUG_NETWORK_ENABLE
						Serial.println(F(" cleared!"));
						Serial.flush();
						#endif
					}
					#ifdef DEBUG_NETWORK_ENABLE
					else {
						Serial.println();
						Serial.flush();
					}
					#endif
				}
			}
		}

		/**
		 * get the number of neighbours
		 * @return num of neighbours
		 */
		uint8_t neighboursSize() {
			uint8_t num = 0;
			for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
				if(neighbours[i].nodeId != 0)
				num++;
			}

			return num;
		}
};

#endif
