
#include "../ServoDriver.hpp"

class PiServo : public ServoDriver {
	public:
		void init(int defaultAngle, const char *param);
		void update();
	private:
		void write(int angle);
		int hwPin;
};
