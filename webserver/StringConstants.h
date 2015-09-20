/*
 * StringConstants.h
 *
 * Created: 05.09.2015 15:45:57
 *  Author: helge
 */


#ifndef STRINGCONSTANTS_H_
#define STRINGCONSTANTS_H_

#include <avr/pgmspace.h>
#include <Configuration.h>

#ifdef WEBSERVER_ENABLE

const char pageAddressMain[] PROGMEM = {"/"};
const char pageAddressGetSensorInfo[] PROGMEM = {"/getSensorInfo"};
const char pageNodes[] PROGMEM = {"/nodes"};
const char pageCss[] PROGMEM = {"/css"};
const char pageRequestSensor[] PROGMEM = {"/sensor"};
const char pageWriteSensor[] PROGMEM = {"/sensorWrite"};
const char pageSensorData[] PROGMEM = {"/sensorData"};
const char pageMaintenanceNodesInfos[] PROGMEM = {"/nodeInfos"};
PGM_P pageAddresses[] = {
	NULL,
	pageAddressMain,
	pageAddressGetSensorInfo,
	pageNodes,
	pageCss,
	pageRequestSensor,
	pageWriteSensor,
	pageSensorData,
	pageMaintenanceNodesInfos
};

const char pageTitleMain[] PROGMEM = {"Start"};
const char pageTitleGetSensorInfo[] PROGMEM = {"Sensor Info"};
const char pageTitleNodes[] PROGMEM = {"Nodes"};
const char pageTitleRequestSensor[] PROGMEM = {"Rquested Sensor Information"};
const char pageTitleWriteSensor[] PROGMEM = {"Write to Sensor"};
const char pageTitleSensordata[] PROGMEM = {"Sensordata"};
const char pageTitleMaintenanceNodeInfos[] PROGMEM = {"Maintenance"};

const char mimeTypeHtml[] PROGMEM = { "text/html" };
const char mimeTypeCss[] PROGMEM = { "text/css" };
const char mimeTypeBinary[] PROGMEM = { "application/octet-stream" };
const char mimeTypeCsv[] PROGMEM = { "text/csv" };

PGM_P mimeTypes[] = {mimeTypeHtml, mimeTypeCss, mimeTypeBinary, mimeTypeCsv};

PGM_P pageTitles[] = {
	NULL,
	pageTitleMain,
	pageTitleGetSensorInfo,
	pageTitleNodes,
	NULL,
	pageTitleRequestSensor,
	pageTitleWriteSensor,
	pageTitleSensordata,
	pageTitleMaintenanceNodeInfos
};
uint8_t pageBelongsToMenu[] = {0, 1, 3, 3, 0, 3, 3, 7, 8};

const char variableRemote[] PROGMEM = {"remote"};
const char variableDelete[] PROGMEM = {"delete"};
const char variableHwAddress[] PROGMEM = {"hwaddress"};
const char variableHwType[] PROGMEM = {"hwtype"};
const char variableVal[] PROGMEM = {"val"};
const char variableListType[] PROGMEM = {"listtype"};
const char variableListNum[] PROGMEM = {"listnum"};
const char variableFilename[] PROGMEM = {"file"};
const char variableFiletype[] PROGMEM = {"type"};
const char variableFiletypeCsv[] PROGMEM = {"csv"};
const char variableName[] PROGMEM = {"name"};

const char linkNameX[] PROGMEM = {"x"};
const char linkNameOn[] PROGMEM = {"on"};
const char linkNameOff[] PROGMEM = {"off"};
const char linkNameToggle[] PROGMEM = {"toggle"};

const char linkCmdValDefault[] PROGMEM = {"0x00"};
const char linkCmdListChecked[] PROGMEM = {" checked='checked'"};
const char linkCmdListTypeUint8[] PROGMEM = {"u8"};
const char linkCmdListTypeUint16[] PROGMEM = {"u16"};
const char linkCmdListTypeInt8[] PROGMEM = {"i8"};
const char linkCmdListTypeInt16[] PROGMEM = {"i16"};


/** hw type strs */
const char strHWType_UNKNOWN[] PROGMEM = {"UNKNOWN"};
const char strHWType_ANALOG[] PROGMEM = {"ANALOG"};
const char strHWType_DIGITAL[] PROGMEM = {"DIGITAL"};
const char strHWType_accelerometer[] PROGMEM = {"Accelerometer"};
const char strHWType_button[] PROGMEM = {"Button"};
const char strHWType_gyroscope[] PROGMEM = {"Gyroscope"};
const char strHWType_humidity[] PROGMEM = {"Humidity"};
const char strHWType_ir[] PROGMEM = {"IR"};
const char strHWType_keypad[] PROGMEM = {"Keypad"};
const char strHWType_magneticField[] PROGMEM = {"Magnetic Field"};
const char strHWType_methane[] PROGMEM = {"Methane"};
const char strHWType_motion[] PROGMEM = {"Motion"};
const char strHWType_pressure[] PROGMEM = {"Pressure"};
const char strHWType_rtc[] PROGMEM = {"RTC"};
const char strHWType_dcf77[] PROGMEM = {"DCF77"};
const char strHWType_sonar[] PROGMEM = {"Sonar"};
const char strHWType_hwSwitch[] PROGMEM = {"Switch"};
const char strHWType_temprature[] PROGMEM = {"Temperature"};
const char strHWType_touchpad[] PROGMEM = {"Touchpad"};
const char strHWType_led[] PROGMEM = {"LED"};
const char strHWType_rcswitch[] PROGMEM = {"RC Switch"};
const char strHWType_relay[] PROGMEM = {"Relay"};
const char strHWType_light[] PROGMEM = {"Light"};
const char strHWType_tone[] PROGMEM = {"Tone"};

/** string representations of hwtypes */
PGM_P hardwareTypeStrings[] = {	strHWType_UNKNOWN,
	strHWType_ANALOG,
	strHWType_DIGITAL,
	strHWType_accelerometer,
	strHWType_button,
	strHWType_gyroscope,
	strHWType_humidity,
	strHWType_ir,
	strHWType_keypad,
	strHWType_magneticField,
	strHWType_methane,
	strHWType_motion,
	strHWType_pressure,
	strHWType_rtc,
	strHWType_dcf77,
	strHWType_sonar,
	strHWType_hwSwitch,
	strHWType_temprature,
	strHWType_touchpad,
	strHWType_led,
	strHWType_rcswitch,
	strHWType_relay,
	strHWType_light,
strHWType_tone};

#endif
#endif /* STRINGCONSTANTS_H_ */