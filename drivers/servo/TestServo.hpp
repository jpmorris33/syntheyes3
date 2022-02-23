
#include "../ServoDriver.hpp"

class TestServo : public ServoDriver {
	public:
		void init(int defaultAngle, const char *param);
		void update();
	private:
		void write(int angle);
};
