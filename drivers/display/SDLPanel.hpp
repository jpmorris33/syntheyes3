
#include "../PanelDriver.hpp"

class SDLPanel : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
};
