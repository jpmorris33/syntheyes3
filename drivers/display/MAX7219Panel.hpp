
#include "../PanelDriver.hpp"

class MAX7219Panel : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
