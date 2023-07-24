//  CAP1188 driver, based on Adafruit library
#include <stdio.h>

#define REG_PRODUCT		0xfd
#define REG_MANUFACTURER	0xfe
#define REG_REVISION		0xff
#define REG_MAIN		0x00
#define REG_INTERRUPT		0x01
#define REG_INPUT		0x03
#define REG_SENSITIVITY		0x1f
#define REG_MULTITOUCH		0x2a
#define REG_LEDLINK		0x72
#define REG_STANDBY		0x41

#include "CAP1188Sensor.hpp"
CAP1188Sensor::CAP1188Sensor() {
	i2cHandle = -1;
	channels = 0;
}

void CAP1188Sensor::init(const char *param) {

	int addr = 0x29;
	int sensitivity = 0;
	bool lights = true;

	printf("*Init CAP1188 touch sensor\n");

	const char *p = getDriverParam(param,"address");
	if(p) {
		addr = getDriverHex(p);
		printf(" CAP1188: Using address 0x%02x\n", addr);
	}

	p = getDriverParam(param,"lights");
	if(p) {
		lights = getDriverHex(p);
		printf(" CAP1188: Lights = %d\n",lights);
	}

	i2cHandle = init_i2c(0, addr);
	if(i2cHandle == -1) {
		printf(" CAP1188: Error opening I2C device\n");
		return;
	}

	readReg(REG_PRODUCT);

	unsigned char pro=readReg(REG_PRODUCT);
	unsigned char man=readReg(REG_MANUFACTURER);
	unsigned char rev=readReg(REG_REVISION);

	if(pro !=  0x50)  {
		printf(" CAP1188: Error: unexpected product id 0x%02x\n", pro);
		i2cHandle = -1;
		return;
	}
	if(man !=  0x5d)  {
		printf(" CAP1188: Error: unexpected manufacturer id 0x%02x\n", man);
		i2cHandle = -1;
		return;
	}
	if(rev !=  0x83)  {
		printf(" CAP1188: Error: unexpected revision id 0x%02x\n", rev);
		i2cHandle = -1;
		return;
	}

	writeReg(REG_MULTITOUCH, 0);
	writeReg(REG_LEDLINK, lights ? 0xff : 0);
	writeReg(REG_STANDBY, 0x30);

	// Set up the sensitivity how we want it

	p = getDriverParam(param,"sensitivity");
	if(p) {
		sensitivity = getDriverHex(p);
		printf(" CAP1188: Using sensitivity %d\n", sensitivity);
	}

	if(sensitivity < 0) {
		sensitivity = 0;
	}
	if(sensitivity > 7) {
		sensitivity = 7;
	}
	sensitivity <<= 5;

	unsigned char reg = readReg(REG_SENSITIVITY) & 0x8f;
	writeReg(REG_SENSITIVITY, reg | sensitivity);

	
}

bool CAP1188Sensor::check() {

	if(i2cHandle == -1) {
		return false;
	}

	unsigned char reg = readReg(REG_INPUT);
	if(reg) {
//		printf("reg = %x\n",reg);
		channels=reg;
		writeReg(REG_MAIN, readReg(REG_MAIN) & ~REG_INTERRUPT);
		return true;
	}

	return false;
}


bool CAP1188Sensor::isChannel(int channel) {

	if (channel < 0 || channel > 7) {
		return false;
	}

//	printf("channels = %x, channel = %d, mask = %x\n",channels,channel,1<<channel);

	return channels & (1 << channel);
}

unsigned char CAP1188Sensor::readReg(int reg) {
	return i2c_readReg(i2cHandle, reg);
}

void CAP1188Sensor::writeReg(int reg, unsigned char data) {
	return i2c_writeReg(i2cHandle, reg, data);
}

