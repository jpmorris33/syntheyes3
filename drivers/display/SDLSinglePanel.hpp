
#include "../PanelDriver.hpp"

class SDLSinglePanel : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
