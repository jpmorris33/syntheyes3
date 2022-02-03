#ifndef __GPIO_HPP__
#define __GPIO_HPP__

#define DEVICE_BOTH 0
#define DEVICE_TRANSMITTER 1
#define DEVICE_RECEIVER 2

class GPIOPin {
	public:
		GPIOPin(int pin, char device, bool output);
		void write(bool state);
		bool check();
		int getPin();
		char getDevice();
		bool isOutput();
		bool isReserved();
		void reserve();
		GPIOPin *findConflict();

		GPIOPin *next;

	private:
		bool rightDevice();
		bool conflicting(GPIOPin *other);

		int pin;
		int realpin;
		char device;
		bool output;
		bool reserved;
};

extern GPIOPin *reserveOutputPin(int pin);
extern GPIOPin *reserveInputPin(int pin);

#endif