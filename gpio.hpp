#ifndef __GPIO_HPP__
#define __GPIO_HPP__

#define DEVICE_BOTH		0
#define DEVICE_TRANSMITTER	1
#define DEVICE_RECEIVER		2

#define GPIOMODE_INPUT		0	// Force to input pin
#define GPIOMODE_OUTPUT		1	// Force to output pin
#define GPIOMODE_SPECIAL	2	// SPI etc, don't change the mode!

#define GPIO_LOGIC_HIGH 0		// Yes, this is stupid
#define GPIO_LOGIC_LOW 1

class GPIOPin {
	public:
		GPIOPin(int pin, char device, char mode);
		void write(bool state);
		void writeByte(unsigned char value, GPIOPin *clock);
		bool check();
		int getPin();
		char getDevice();
		bool isOutput();
		bool isInput();
		bool isReserved();
		void reserve();
		void setInverted();
		GPIOPin *findConflict();

		GPIOPin *next;

	private:
		bool rightDevice();
		bool conflicting(GPIOPin *other);

		int pin;
		int realpin;
		char device;
		char mode;
		bool reserved;
		bool invert;
};

extern GPIOPin *reserveOutputPin(int pin);
extern GPIOPin *reserveInputPin(int pin);
extern GPIOPin *reserveSpecialPin(int pin);

extern GPIOPin *init_spi(int cspin, long speed, int mode, int bus);
extern void blit_spi(int bus, unsigned char *data, int len);

#endif