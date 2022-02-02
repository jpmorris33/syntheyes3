
#include "../PanelDriver.hpp"

class Unicorn : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
