
#include "../PanelDriver.hpp"

class SDLPanel : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
