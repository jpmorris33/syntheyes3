
#include "../PanelDriver.hpp"

class Unicorn : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
