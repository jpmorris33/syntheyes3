
#include "../PanelDriver.hpp"

#define MAXPANEL_W 16
#define MAXPANEL_H 16

class MAX7219Panel : public PanelDriver {
	public:
		void init();
		void draw();
		void drawMirrored();
		void updateRGB(unsigned char *img, int w, int h);
		void updateRGB(unsigned char *img, int w, int h, uint32_t colour);
		void updateRGBpattern(unsigned char *img, int w, int h, int offset);
		void setPattern(unsigned char pattern[16][16]);
		void clear(uint32_t colour);
		void clearV(int x, uint32_t colour);
		void clearH(int y, uint32_t colour);
		uint32_t getCaps();

	private:
		unsigned char framebuffer[MAXPANEL_W*MAXPANEL_H*3];
};
