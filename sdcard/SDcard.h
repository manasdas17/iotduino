/*
 * SDcard.h
 *
 * Created: 31.05.2015 10:32:51
 *  Author: helge
 */


#ifndef SDCARD_H_
#define SDCARD_H_

#include <Arduino.h>
#include <SD.h>
#include <dispatcher/EventCallbackInterface.h>
#include <dispatcher/Commands.h>
#include <dispatcher/DiscoveryService.h>
#include <dispatcher/SubscriptionService.h>
#include <networking/LayerConfig.h>

/** SS pin for SD reader */
#define PIN_SD_SS 4

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

/** start address for node info table */
#define nodeTableStart 0
/** start addres for discovery info table */
#define discoveryInfoStart (SD_DISCOVERY_NUM_NODES * sizeof(SD_nodeInfoTableEntry_t))
/**  */
#define subscriptionInfoStart (discoveryInfoStart + SD_DISCOVERY_NUM_NODES * SD_DISCOVERY_NUM_SUBSCRIPTIONS_PER_NODE * sizeof(SD_subscriptionInfoTableEntry_t))

class SDcard {
	//private:
	public:
		/**  */
		typedef struct SD_nodeInfoTableEntryStruct {
			uint8_t nodeId;
			uint32_t lastDiscoveryRequest;
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
			uint8_t active;
		} SD_subscriptionInfoTableEntry_t;

		/** working object */
		File myFileDiscovery;

	public:
		/** modes */
		enum FileMode {READ = FILE_READ, WRITE = (FILE_WRITE | O_SYNC)};

		/** discovery + subscriotion buffer */
		const char* fileNameDiscoveryInfo = {"DISCOVER.BIN"};

		/**
		 * delete all subscription infos
		 * @return success
		 */
		boolean deleteSubscriptionInfo();

		/**
		 * delete all subscriptions for a node
		 * @param nodeId
		 * @return success
		 */
		boolean deleteSubscriptionInfo(uint8_t nodeId);

		/**
		 * delete subscription for a node
		 * @param nodeId
		 * @param index
		 * @return success
		 */
		boolean deleteSubscriptionInfo(uint8_t nodeId, uint8_t index);

		/**
		 * save subscriptions
		 * @param nodeId
		 * @param index
		 * @param info
		 * @return success
		 */
		boolean saveSubscriptionInfo(uint8_t nodeId, uint8_t index, SD_subscriptionInfoTableEntry_t* info);

		/**
		 * @param nodeId
		 * @param index
		 * @return address
		 */
		uint32_t getSubscriptionInfosAddressForNodeById(uint8_t nodeId, uint8_t index);

		/**
		 * save set of subscription infos
		 * @param nodeId
		 * @param infos
		 * @param numInfos
		 * @return success
		 */
		boolean saveSubscriptionsInfos(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* info, uint8_t numInfos);

		/**
		 * delete all discovery infos
		 * @return success
		 */
		boolean deleteDiscoveryInfosAll();

		/**
		 * delete discovery infos for a node
		 * @param nodeId
		 * @return success
		 */
		boolean deleteDiscoveryInfos(uint8_t nodeId);

		/**
		 * save set of discovery infos
		 * @param nodeId
		 * @param infos
		 * @param numInfos
		 * @param startIndex
		 * @return success
		 */
		boolean saveDiscoveryInfos(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* info, uint8_t numInfos, uint8_t startIndex);

		/**
		 * save set of discovery infos
		 * @param nodeId
		 * @param infos
		 * @param numInfos
		 * @return success
		 */
		boolean saveDiscoveryInfos(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* info, uint8_t numInfos);

		/**
		 * write data to discovery info file
		 * @param pos
		 * @param buffer
		 * @param bytes
		 * @return success
		 */
		boolean discoveryWriteToSD(uint32_t pos, void* buf, size_t len);

		/**
		 * save node info
		 * @param nodeId
		 * @param info
		 * @return success
		 */
		boolean saveNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* info);

		/**
		 * get subscription infos for node
		 * @param nodeId
		 * @param buf fer
		 * @param numEntries of buffer
		 * @return success
		 */
		boolean getSubscriptionInfosForNode(uint8_t nodeId, SD_subscriptionInfoTableEntry_t* buf, uint8_t numEntries);

		/**
		 * @param nodeId
		 * @return address
		 */
		uint32_t getSubscriptionsInfosAddressForNode(uint8_t nodeId);

		/**
		 * get discovery infos for node
		 * @param nodeId
		 * @param buf fer
		 * @param numEntries of buffer
		 * @return success
		 */
		boolean getDiscoveryInfosForNode(uint8_t nodeId, SD_nodeDiscoveryInfoTableEntry_t* buf, uint8_t numEntries);

		uint32_t getDiscoveryInfosAddressForNode(uint8_t nodeId);

		/**
		 * get node info
		 * @param nodeId
		 * @param buf data object
		 * @return success
		 */
		boolean getDiscoveryNodeInfo(uint8_t nodeId, SD_nodeInfoTableEntry_t* buf);

		/** empty constructor*/
		SDcard() {};

		/**
		 * initialiser must be called before using SDcard functions
		 * @return success
		 */
		boolean initSD();

		uint32_t getNodeDiscoveryInfoAddress(uint8_t nodeId);

		/**
		 * @param pos
		 * @success
		 */
		boolean seekDiscovery(uint32_t pos);

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

		boolean appendToFile(const char* fileName, uint8_t* buf, uint8_t bufSize);

};

#endif /* SDCARD_H_ */