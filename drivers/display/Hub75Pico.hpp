#include "../PanelDriver.hpp"
#include "hardware/pio.h"

class Hub75Pico : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
	private:
		void blit();
		uint32_t *outbuffer;
		int rowpins;
		uint data_prog_offset;
		uint row_prog_offset;
		PIO pio;
};
