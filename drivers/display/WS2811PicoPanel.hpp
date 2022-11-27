
#include "../PanelDriver.hpp"

class WS2811PicoPanel : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
	private:
		uint32_t *outbuffer;
};
