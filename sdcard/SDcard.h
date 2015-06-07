/*
 * SDcard.h
 *
 * Created: 31.05.2015 10:32:51
 *  Author: helge
 */


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Arduino.h>
#include <SD/SD.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/Commands.h>
#include <dispatcher/DiscoveryService.h>
#include <dispatcher/SubscriptionService.h>
#include <networking/LayerConfig.h>

/** SS pin for SD reader */
#define PIN_SD_SS 4
/**  node string info size per node */
#define NODE_INFO_SIZE (0x10)

/** maximum number of nodeids */
#define SD_DISCOVERY_NUM_NODES 128
/** maximum discovery infos per node */
#define SD_DISCOVERY_NUM_INFOS_PER_NODE INTERFACES_BUF_SIZE
/** maximum subscriptions per node */
#define SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE numSubscriptionList

/**  size of total discovery storage*/
#define SD_DISCOVERY_FILESIZE (SD_DISCOVERY_NUM_NODES * sizeof(SDcard::SD_nodeInfoTableEntry_t) \
	+ SD_DISCOVERY_NUM_NODES * SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SDcard::SD_nodeDiscoveryInfoTableEntry_t) \
	+ SD_DISCOVERY_NUM_NODES * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SDcard::SD_subscriptionInfoTableEntry_t))

class SDcard {
	//private:
	public:
		/**  */
		typedef struct SD_nodeInfoTableEntryStruct {
			uint8_t nodeId;
		} SD_nodeInfoTableEntry_t;

		/**  */
		typedef struct SD_nodeDiscoveryInfoTableEntryStruct {
			uint8_t hardwareAddress;
			uint8_t hardwareType;
			uint32_t rtcTimestamp;
		} SD_nodeDiscoveryInfoTableEntry_t;

		/**  */
		typedef struct SD_subscriptionInfoTableEntryStruct {
			uint8_t hardwareAddress;
			uint8_t hardwareType;
			uint32_t delay;
			uint8_t subscription_event_type_t;
			seq_t sequence;
			uint32_t rtcLastkeepalive;
			uint32_t rtcLastRequest;
			uint8_t activated;
		} SD_subscriptionInfoTableEntry_t;

		/** start address for node info table */
		uint16_t nodeTableStart = 0;
		/** start addres for discovery info table */
		static const uint16_t discoveryInfoStart = SD_DISCOVERY_NUM_NODES * sizeof(SD_nodeInfoTableEntry_t);
		/**  */
		static const uint16_t subscriptionInfoStart = discoveryInfoStart + SD_DISCOVERY_NUM_NODES * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SD_subscriptionInfoTableEntry_t);

		/** working object */
		File myFile;
		/** pointer to currently opened file description */
		void* currentOpenedFile;
		/** mode of currently opened file */
		uint8_t currentOpenedMode;

	public:
		/** modes */
		enum FileMode {READ = FILE_READ, WRITE = (FILE_WRITE | O_SYNC)};

		/** string node information */
		const char* fileNameNodeInfo = {"NODEINFO.TXT"};
		/** discovery + subscriotion buffer */
		const char* fileNameDiscoveryInfo = {"DISCOVER.BIN"};

		/**
		 * delete all subscription infos
		 * @return success
		 */
		boolean deleteSubscriptionInfo() {
			boolean success = true;
			for(uint8_t i = 0; i < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE; i++) {
				success &= deleteSubscriptionInfo(i);
			}

			return success;
		}

		/**
		 * delete all subscriptions for a node
		 * @param nodeId
		 * @return success
		 */
		boolean deleteSubscriptionInfo(uint8_t nodeId) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES)
				return false;

			boolean success = true;
			for(uint8_t i = 0; i < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE; i++) {
				success &= deleteSubscriptionInfo(nodeId, i);
			}

			return success;
		}

		/**
		 * delete subscription for a node
		 * @param nodeId
		 * @param index
		 * @return success
		 */
		boolean deleteSubscriptionInfo(uint8_t nodeId, uint8_t index) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES || index >= SD_DISCOVERY_NUM_INFOS_PER_NODE)
				return false;

			uint8_t tmp[sizeof(SD_subscriptionInfoTableEntry_t)];
			memset(tmp, 0, sizeof(tmp));

			uint32_t pos = subscriptionInfoStart + nodeId * sizeof(SD_subscriptionInfoTableEntry_t) + index * sizeof(SD_subscriptionInfoTableEntry_t);
			return discoveryWriteToSD(pos, tmp, sizeof(SD_subscriptionInfoTableEntry_t));
		}

		/**
		 * save subscriptions
		 * @param nodeId
		 * @param index
		 * @param info
		 * @return success
		 */
		boolean saveSubscriptionInfo(uint8_t nodeId, uint8_t index, SD_subscriptionInfoTableEntry_t* info) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES || index >= SD_DISCOVERY_NUM_INFOS_PER_NODE)
				return false;

			uint32_t pos = subscriptionInfoStart + nodeId * sizeof(SD_subscriptionInfoTableEntry_t) + index * sizeof(SD_subscriptionInfoTableEntry_t);
			return discoveryWriteToSD(pos, (uint8_t*) info, sizeof(SD_subscriptionInfoTableEntry_t));
		}

		/**
		 * save set of subscription infos
		 * @param nodeId
		 * @param infos
		 * @param numInfos
		 * @return success
		 */
		boolean saveSubscriptionsInfos(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* info, uint8_t numInfos) {
			if(numInfos > SD_DISCOVERY_NUM_INFOS_PER_NODE)
				return false;

			//prepare data
			SD_subscriptionInfoTableEntry_t* data[SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SD_subscriptionInfoTableEntry_t)];
			uint16_t tmp = numInfos * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
			memset(data + tmp, 0, tmp - sizeof(data)); //may be optimised as we overwrite data anyway.
			memcpy(data, info, tmp);

			//write to sd
			uint32_t pos = subscriptionInfoStart + nodeId * sizeof(SD_subscriptionInfoTableEntry_t);
			uint16_t len = sizeof(data);
			return discoveryWriteToSD(pos, (uint8_t*) data, len);
		}

		/**
		 * delete all discovery infos
		 * @return success
		 */
		boolean deleteDiscoveryInfosAll() {
			boolean success = true;
			for(uint8_t i = 0; i < SD_DISCOVERY_NUM_NODES; i++) {
				success &= deleteDiscoveryInfos(i);
			}
			return success;
		}

		/**
		 * delete discovery infos for a node
		 * @param nodeId
		 * @return success
		 */
		boolean deleteDiscoveryInfos(uint8_t nodeId) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES)
				return false;
			uint8_t tmp[SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t)];
			memset(tmp, 0, sizeof(tmp));

			//write to sd
			uint32_t pos = discoveryInfoStart + nodeId * SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
			return discoveryWriteToSD(pos, tmp, sizeof(tmp));
		}

		/**
		 * save set of discovery infos
		 * @param nodeId
		 * @param infos
		 * @param numInfos
		 * @return success
		 */
		boolean saveDiscoveryInfos(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* info, uint8_t numInfos) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES || numInfos > SD_DISCOVERY_NUM_INFOS_PER_NODE)
				return false;

			//prepare data
			SD_nodeDiscoveryInfoTableEntry_t data[SD_DISCOVERY_NUM_INFOS_PER_NODE];
			memset(&data[numInfos], 0, (SD_DISCOVERY_NUM_INFOS_PER_NODE - numInfos) * sizeof(SD_nodeDiscoveryInfoTableEntry_t)); //may be optimised as we overwrite data anyway.
			memcpy(&data, info, numInfos * sizeof(SD_nodeDiscoveryInfoTableEntry_t));

			//write to sd
			uint32_t pos = discoveryInfoStart + nodeId * sizeof(SD_nodeDiscoveryInfoTableEntry_t);
			uint16_t len = sizeof(data);
			return discoveryWriteToSD(pos, (uint8_t*) data, len);
		}

		/**
		 * write data to discovery info file
		 * @param pos
		 * @param buffer
		 * @param bytes
		 * @return success
		 */
		boolean discoveryWriteToSD(uint32_t pos, void* buf, size_t len) {
			openFile(fileNameDiscoveryInfo, SDcard::WRITE);
			if(seek(pos)) {
				uint32_t bytes = myFile.write((uint8_t*) buf, len);
				myFile.flush();

				#ifdef DEBUG_SD_ENABLE
					Serial.print(millis());
					Serial.print(F(": SDcard::discoveryWriteToSD() pos="));
					Serial.print(pos);
					Serial.print(F(" len="));
					Serial.print(len);
					Serial.print(F(" written="));
					Serial.println(bytes);
				#endif

				return bytes;
			}
			return false;
		}

		/**
		 * save node info
		 * @param nodeId
		 * @param info
		 * @return success
		 */
		boolean saveNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* info) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES)
				return false;
			uint32_t pos = nodeId * sizeof(SD_nodeInfoTableEntry_t);
			uint16_t len = sizeof(SD_nodeInfoTableEntry_t);
			return discoveryWriteToSD(pos, (uint8_t*) info, len);
		}

		/**
		 * get subscription infos for node
		 * @param nodeId
		 * @param buf fer
		 * @param numEntries of buffer
		 * @return success
		 */
		boolean getSubscriptionInfosForNode(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* buf, uint8_t numEntries) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES || numEntries < SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE)
				return false;
			//read
			openFile(fileNameDiscoveryInfo, SDcard::READ);
			seek(subscriptionInfoStart + nodeId * sizeof(SD_subscriptionInfoTableEntry_t));
			return myFile.readBytes((uint8_t*) buf, numEntries * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SD_subscriptionInfoTableEntry_t));
		}

		/**
		 * get discovery infos for node
		 * @param nodeId
		 * @param buf fer
		 * @param numEntries of buffer
		 * @return success
		 */
		boolean getDiscoveryInfosForNode(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* buf, uint8_t numEntries) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES || numEntries < SD_DISCOVERY_NUM_INFOS_PER_NODE)
				return false;
			//read
			openFile(fileNameDiscoveryInfo, SDcard::READ);
			if(!myFile)
				return false;

			if(!seek(discoveryInfoStart + nodeId * SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t)))
				return false;

			return myFile.readBytes((uint8_t*) buf, SD_DISCOVERY_NUM_INFOS_PER_NODE * sizeof(SD_nodeDiscoveryInfoTableEntry_t));
		}

		/**
		 * get node info
		 * @param nodeId
		 * @param buf data object
		 * @return success
		 */
		boolean getNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* buf) {
			if(nodeId >= SD_DISCOVERY_NUM_NODES)
				return false;
			//read
			openFile(fileNameDiscoveryInfo, SDcard::READ);
			seek(nodeId * sizeof(SD_nodeInfoTableEntry_t));
			return myFile.readBytes((uint8_t*) buf, sizeof(SD_nodeInfoTableEntry_t));
		}

		/** empty constructor*/
		SDcard();

		/**
		 * initialiser must be called before using SDcard functions
		 * @return success
		 */
		boolean init();

		/**
		 * append data to file
		 * @param fileName
		 * @param buf
		 * @param len
		 * @return success
		 */
		boolean appendToFile(const char* fileName, uint8_t* buf, uint8_t bufSize);

		/**
		 * get node info from file.
		 * structure:
		 * Node 0:   0x00000000..0x00000000f [unused]
		 * Node 1:   0x00000010..0x00000001f
		 *  ...
		 * Node 255: 0x00000ff0..0x000000fff[broadcast]
		 */
		uint8_t getNodeInfo(uint8_t nodeId, uint8_t* buf, uint8_t bufSize);

		/**
		 * @param pos
		 * @success
		 */
		boolean seek(uint32_t pos) {
			if(!myFile)
				return false;
			return myFile.seek(pos);
		}

	//protected:
		/**
		 * internal method for opening a file
		 * @param filName
		 * @param mode
		 * @return success
		 */
		boolean openFile(const char* fileName, FileMode mode);

		/**
		 * prepare discovery file - possibly extend it to the necessary size
		 */
		void prepareDiscoveryFile();
};
#endif /* SDCARD_H_ */