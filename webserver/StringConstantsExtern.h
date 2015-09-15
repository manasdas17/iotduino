/*
 * StringConstantsExtern.h
 *
 * Created: 05.09.2015 15:53:45
 *  Author: helge
 */


#ifndef STRINGCONSTANTSEXTERN_H_
#define STRINGCONSTANTSEXTERN_H_



extern const char pageAddressMain[] PROGMEM;
extern const char pageAddressGetSensorInfo[] PROGMEM;
extern const char pageNodes[] PROGMEM;
extern const char pageCss[] PROGMEM;
extern const char pageRequestSensor[] PROGMEM;
extern const char pageWriteSensor[] PROGMEM;
extern const char pageSensorData[] PROGMEM;
extern const char pageMaintenanceNodesInfos[] PROGMEM;
extern PGM_P pageAddresses[];

extern const char pageTitleMain[] PROGMEM;
extern const char pageTitleGetSensorInfo[] PROGMEM;
extern const char pageTitleNodes[] PROGMEM;
extern const char pageTitleRequestSensor[] PROGMEM;
extern const char pageTitleWriteSensor[] PROGMEM;
extern const char pageTitleSensordata[] PROGMEM;
extern const char pageTitleMaintenanceNodeInfos[] PROGMEM;

extern const char mimeTypeHtml[] PROGMEM;
extern const char mimeTypeCss[] PROGMEM;
extern const char mimeTypeBinary[] PROGMEM;
extern const char mimeTypeCsv[] PROGMEM;

extern PGM_P mimeTypes[];

extern PGM_P pageTitles[];

extern uint8_t pageBelongsToMenu[];

extern const char variableRemote[] PROGMEM;
extern const char variableDelete[] PROGMEM;
extern const char variableHwAddress[] PROGMEM;
extern const char variableHwType[] PROGMEM;
extern const char variableVal[] PROGMEM;
extern const char variableListType[] PROGMEM;
extern const char variableListNum[] PROGMEM;
extern const char variableFilename[] PROGMEM;
extern const char variableFiletype[] PROGMEM;
extern const char variableFiletypeCsv[] PROGMEM;
extern const char variableName[] PROGMEM;

extern const char linkNameX[] PROGMEM;
extern const char linkNameOn[] PROGMEM;
extern const char linkNameOff[] PROGMEM;
extern const char linkNameToggle[] PROGMEM;

extern const char linkCmdValDefault[] PROGMEM;
extern const char linkCmdListChecked[] PROGMEM;
extern const char linkCmdListTypeUint8[] PROGMEM;
extern const char linkCmdListTypeUint16[] PROGMEM;
extern const char linkCmdListTypeInt8[] PROGMEM;
extern const char linkCmdListTypeInt16[] PROGMEM;


/** hw type strs */
extern const char strHWType_UNKNOWN[] PROGMEM;
extern const char strHWType_ANALOG[] PROGMEM;
extern const char strHWType_DIGITAL[] PROGMEM;
extern const char strHWType_accelerometer[] PROGMEM;
extern const char strHWType_button[] PROGMEM;
extern const char strHWType_gyroscope[] PROGMEM;
extern const char strHWType_humidity[] PROGMEM;
extern const char strHWType_ir[] PROGMEM;
extern const char strHWType_keypad[] PROGMEM;
extern const char strHWType_magneticField[] PROGMEM;
extern const char strHWType_methane[] PROGMEM;
extern const char strHWType_motion[] PROGMEM;
extern const char strHWType_pressure[] PROGMEM;
extern const char strHWType_rtc[] PROGMEM;
extern const char strHWType_dcf77[] PROGMEM;
extern const char strHWType_sonar[] PROGMEM;
extern const char strHWType_hwSwitch[] PROGMEM;
extern const char strHWType_temprature[] PROGMEM;
extern const char strHWType_touchpad[] PROGMEM;
extern const char strHWType_led[] PROGMEM;
extern const char strHWType_rcswitch[] PROGMEM;
extern const char strHWType_relay[] PROGMEM;
extern const char strHWType_light[] PROGMEM;
extern const char strHWType_tone[] PROGMEM;

/** string representations of hwtypes */
extern PGM_P hardwareTypeStrings[];

#endif /* STRINGCONSTANTSEXTERN_H_ */