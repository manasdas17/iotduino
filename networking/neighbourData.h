
#ifndef NEIGHBOURDATA_H_
#define NEIGHBOURDATA_H_

#include <Configuration.h>
#include <networking/Layer3.h>
#include <ramManager.h>

extern SPIRamManager ram;

//#undef ENABLE_EXTERNAL_RAM

class NeighbourManager {
	public:
		typedef struct neighbourData_struct {
			public:
			l3_address_t nodeId;
			uint8_t hopCount;
			l3_address_t hopNextNodeId;
			l3_timestamp timestamp;
		} neighbourData_t;

		#ifdef ENABLE_EXTERNAL_RAM
			uint8_t memRegionId;
		#else
			neighbourData_t neighbours[CONFIG_L3_NUM_NEIGHBOURS];
		#endif

		const static uint8_t NeighbourListSize = CONFIG_L3_NUM_NEIGHBOURS;

		l3_address_t localAddress;

		void init(l3_address_t localAddress) {
			this->localAddress = localAddress;

			#ifdef ENABLE_EXTERNAL_RAM
				memRegionId = ram.createRegion(sizeof(neighbourData_t), CONFIG_L3_NUM_NEIGHBOURS);

				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				neighbourData_t* n;
				while(it.hasNext()) {
					n = (neighbourData_t*) it.next();
					n->hopCount = 0xff;
					it.writeBack();
				}
			#else
				memset(neighbours, 0, sizeof(neighbours));
				//set hopcount to max!
				for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
					neighbours[i].hopCount = 0xff;
				}
			#endif

		}

		NeighbourManager() {
		}

		boolean getIterator(SPIRamManager::iterator* it) {
			#ifdef ENABLE_EXTERNAL_RAM
			return it->init(&ram, memRegionId);
			#else
			return false;
			#endif
		}


		neighbourData_t* getNeighbour(l3_address_t destination) {
			return getNeighbour(NULL, destination);
		}
		/**
		* get neighbour row.
		* @param neighbour
		*/
		neighbourData_t* getNeighbour(uint8_t* index, l3_address_t destination) {
			#ifdef ENABLE_EXTERNAL_RAM
				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				while(it.hasNext()) {
					neighbourData_t* currentItem = (neighbourData_t*) it.next();
			#else
				for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
					neighbourData_t* currentItem = &neighbours[i];
			#endif
					if(currentItem->nodeId == destination) {
						if(index != NULL)
							*index = it.getIteratorIndex()-1;
						return currentItem;
					}
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
				Serial.print(F(":updateN:"));
				Serial.print(F(" dst="));
				Serial.print(destination);
				Serial.print(F(" nextHop="));
				Serial.print(nextHop);
				Serial.print(F(" hop#="));
				Serial.println(hopCount);
				Serial.flush();
			#endif

			if(destination == localAddress) {
				#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F("\t=us,no upd"));
				#endif
				return true;
			}
			if(hopCount > CONFIG_L3_MAX_HOPCOUNT) {
				#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F("\t#hops excdd"));
				#endif
				return true;
			}

			#ifdef ENABLE_EXTERNAL_RAM
				uint8_t index = 0xff;
				neighbourData_t* n = getNeighbour(&index, destination);
			#else
				neighbourData_t* n = getNeighbour(destination);
			#endif

			//we do not have it yet - create.
			if(n == NULL) {
				//search free index
				#ifdef ENABLE_EXTERNAL_RAM
					SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
					while(it.hasNext()) {
						neighbourData_t* currentItem = (neighbourData_t*) it.next();
				#else
					for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
						neighbourData_t* currentItem = &neighbours[i];
				#endif

						//free entry
						if(currentItem->nodeId == 0) {
							#ifdef DEBUG_NETWORK_ENABLE
								Serial.print(F("\tnew@idx="));
								#ifdef ENABLE_EXTERNAL_RAM
									Serial.print(it.getIteratorIndex()-1);
								#else
									Serial.print(i);
								#endif
								Serial.print(F(": "));
								Serial.flush();
							#endif

							n = currentItem;
							#ifdef ENABLE_EXTERNAL_RAM
								index = it.getIteratorIndex()-1;
							#endif
							break;
						}
					}

				if(index == 0xff) {
					#ifdef DEBUG_NETWORK_ENABLE
						Serial.println(F("\tfull"));
						Serial.flush();
					#endif
					return false;
				}
			}

			#ifdef DEBUG_NETWORK_ENABLE
				else {
					Serial.print(F("\tex:"));
					Serial.flush();
				}
			#endif

			//update data - if better.
			#ifdef DEBUG_NETWORK_ENABLE
				Serial.print(F("crrntHop#="));
				Serial.print(n->hopCount);
				Serial.print(F(" newHop#="));
				Serial.print(hopCount);
			#endif

			if(hopCount < n->hopCount)  {
				n->nodeId = destination;
				n->timestamp = millis();
				n->hopNextNodeId = nextHop;
				n->hopCount = hopCount;

				#ifdef ENABLE_EXTERNAL_RAM
					ram.writeElementToRam(memRegionId, index, n);
				#endif

				#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F("-updt"));
					Serial.flush();
				#endif
			} else if(hopCount == n->hopCount && nextHop == n->hopNextNodeId) {
				n->timestamp = millis();

				#ifdef ENABLE_EXTERNAL_RAM
					ram.writeElementToRam(memRegionId, index, n);
				#endif

				#ifdef DEBUG_NETWORK_ENABLE
					Serial.println(F("==,upd T"));
					Serial.flush();
				#endif
			}
			#ifdef DEBUG_NETWORK_ENABLE
			else {
					Serial.println(F("-no upd"));
				}
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
				Serial.println(F(":clnNghbrs:"));
				Serial.flush();
			#endif

			#ifdef ENABLE_EXTERNAL_RAM
				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				while(it.hasNext()) {
					neighbourData_t* currentItem = (neighbourData_t*) it.next();
			#else
				for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
					neighbourData_t* currentItem = &neighbours[i];
			#endif

					//do we have a valid neighbour?
					if(currentItem->nodeId > 0) {
						#ifdef DEBUG_NETWORK_ENABLE
							Serial.print(F("\tindex="));
							#ifdef ENABLE_EXTERNAL_RAM
								Serial.print(it.getIteratorIndex()-1);
							#else
								Serial.print(i);
							#endif
							Serial.print(F(" node="));
							Serial.print(currentItem->nodeId);
							Serial.print(F(" T="));
							Serial.print(currentItem->timestamp);
							Serial.print(F(" #hop="));
							Serial.print(currentItem->hopCount);
							Serial.print(F(" nextH="));
							Serial.print(currentItem->hopNextNodeId);
						#endif

						if(millis() - currentItem->timestamp > CONFIG_L3_NEIGHBOUR_MAX_AGE_MS) {
							//remove
							memset(currentItem, 0, sizeof(neighbourData_t));
							currentItem->hopCount = 0xff;
							#ifdef ENABLE_EXTERNAL_RAM
								ram.writeElementToRam(memRegionId, it.getIteratorIndex()-1, currentItem);
							#endif

							#ifdef DEBUG_NETWORK_ENABLE
								Serial.println(F(" clrd"));
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
			#ifdef ENABLE_EXTERNAL_RAM
				SPIRamManager::iterator it = SPIRamManager::iterator(&ram, memRegionId);
				while(it.hasNext()) {
					neighbourData_t* currentItem = (neighbourData_t*) it.next();
			#else
				for(uint8_t i = 0; i < CONFIG_L3_NUM_NEIGHBOURS; i++) {
					neighbourData_t* currentItem = &neighbours[i];
			#endif
					if(currentItem->nodeId != 0)
					num++;
				}

			return num;
		}
};

#endif
