
#include "../PanelDriver.hpp"

#define UNICORNPANEL_W 16
#define UNICORNPANEL_H 16

class Unicorn : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		void updateRGB(unsigned char *img, int w, int h);
		void updateRGB(unsigned char *img, int w, int h, uint32_t colour);
		void updateRGBpattern(unsigned char *img, int w, int h, int offset);
		void setPattern(unsigned char pattern[16][16]);
		void clear(uint32_t colour);
	private:
		unsigned char framebuffer[UNICORNPANEL_W*UNICORNPANEL_H*3];
};
