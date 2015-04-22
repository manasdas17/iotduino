#include "InvenSenseMPU6050.h"

Triple<int16_t> InvenSenseMPU6050::readGyro() {
	checkForTimeStampAndReadIfThresholdPassed();
	return Triple<int16_t>(this->accel_t_gyro.value.x_gyro, this->accel_t_gyro.value.y_gyro, this->accel_t_gyro.value.z_gyro);
}

void InvenSenseMPU6050::readGyro( HardwareCommandResult* hwresult ) {
	hwresult->setUintListNum(6);
	uint8_t* list = hwresult->getUintList();
			
	Triple<int16_t> result = readAccels();
	list[0] = result.getA() >> 8;
	list[1] = result.getA() & 0xff;
	list[2] = result.getB() >> 8;
	list[3] = result.getB() & 0xff;
	list[4] = result.getC() >> 8;
	list[5] = result.getC() & 0xff;
}

Triple<int16_t> InvenSenseMPU6050::readAccels() {
	checkForTimeStampAndReadIfThresholdPassed();
	return Triple<int16_t>(this->accel_t_gyro.value.x_accel, this->accel_t_gyro.value.y_accel, this->accel_t_gyro.value.z_accel);
}

void InvenSenseMPU6050::readAccels( HardwareCommandResult* hwresult ) {
	hwresult->setUintListNum(6);
	uint8_t* list = hwresult->getUintList();
			
	Triple<int16_t> result = readAccels();
	list[0] = result.getA() >> 8;
	list[1] = result.getA() & 0xff;
	list[2] = result.getB() >> 8;
	list[3] = result.getB() & 0xff;
	list[4] = result.getC() >> 8;
	list[5] = result.getC() & 0xff;
}

void InvenSenseMPU6050::checkForTimeStampAndReadIfThresholdPassed() {
	if(millis() - lastReadTimestamp > readNewMillisThreshold) {
		getSensorData();
	}
}

int InvenSenseMPU6050::readTemperature() {
	checkForTimeStampAndReadIfThresholdPassed();
	return dT;
}

void InvenSenseMPU6050::readTemperature( HardwareCommandResult* hwresult ) {
	hwresult->setUintListNum(1);
	hwresult->getUintList()[0] = readTemperature();
}

void InvenSenseMPU6050::getSensorData() {
	// Read the raw values.
	// Read 14 bytes at once,
	// containing acceleration, temperature and gyro.
	// With the default settings of the MPU-6050,
	// there is no filter enabled, and the values
	// are not very stable.
	error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t*) &this->accel_t_gyro, sizeof(accel_t_gyro_union));
			
	// Swap all high and low bytes.
	// After this, the registers values are swapped,
	// so the structure name like x_accel_l does no
	// longer contain the lower byte.
	uint8_t swap;
	#define SWAP(x,y) swap = x; x = y; y = swap

	SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
	SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
	SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
	SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
	SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
	SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
	SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);
			  
	// The temperature sensor is -40 to +85 degrees Celsius.
	// It is a signed integer.
	// According to the datasheet:
	//   340 per degrees Celsius, -512 at 35 degrees.
	// At 0 degrees: -512 - (340 * 35) = -12412
	dT = ( (double) accel_t_gyro.value.temperature + 12412.0) / 340.0;
}

 InvenSenseMPU6050::InvenSenseMPU6050() {
	this->lastReadTimestamp = 0;
	this->address = MPU6050_I2C_ADDRESS;
	Wire.begin();

	// default at power-up:
	//    Gyro at 250 degrees second
	//    Acceleration at 2g
	//    Clock source at internal 8MHz
	//    The device is in sleep mode.
	error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);

	// According to the datasheet, the 'sleep' bit
	// should read a '1'.
	// That bit has to be cleared, since the sensor
	// is in sleep mode at power-up.
	error = MPU6050_read (MPU6050_PWR_MGMT_1, &c, 1);

	// Clear the 'sleep' bit to start the sensor.
	MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

int InvenSenseMPU6050::MPU6050_write_reg( int reg, uint8_t data ) {
	int error;
	error = MPU6050_write(reg, &data, 1);
	return error;
}

int InvenSenseMPU6050::MPU6050_write( int start, const uint8_t *pData, int size ) {
	int n, error;

	Wire.beginTransmission(MPU6050_I2C_ADDRESS);
	n = Wire.write(start);        // write the start address
	if (n != 1)
	return -20;

	n = Wire.write(pData, size);  // write data bytes
	if(n != size)
	return -21;

	error = Wire.endTransmission(true); // release the I2C-bus
	if (error != 0)
	return error;

	return 0;         // return : no error
}

int InvenSenseMPU6050::MPU6050_read(int start, uint8_t *buffer, int size) {
	int i, n;//, error;

	Wire.beginTransmission(MPU6050_I2C_ADDRESS);
	n = Wire.write(start);
	if (n != 1)
	return -10;

	n = Wire.endTransmission(false);    // hold the I2C-bus
	if (n != 0)
		return n;

	// Third parameter is true: relase I2C-bus after data is read.
	Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
	i = 0;
	while(Wire.available() && i<size) {
		buffer[i++] = Wire.read();
	}
	
	if (i != size)
		return -11;

	return 0;  // return : no error
}

boolean InvenSenseMPU6050::readVal( HardwareTypeIdentifier type, HardwareCommandResult* result ) {
	if(type == HWType_accelerometer) {
		readAccels(result);
		return true;
	} else if(type == HWType_temprature) {
		readTemperature(result);
		return true;
	} else if(type == HWType_gyroscope) {
		readGyro(result);
		return true;
	}
	
	return false;
}

boolean InvenSenseMPU6050::implementsInterface( HardwareTypeIdentifier type ) {
	if(type == HWType_accelerometer || type == HWType_gyroscope || type == HWType_temprature)
		return true;
	return false;
}
