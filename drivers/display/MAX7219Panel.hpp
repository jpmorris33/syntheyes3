
#include "../PanelDriver.hpp"

class MAX7219Panel : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
		void setBrightness(int percentage);
};
