
#include "../PanelDriver.hpp"

#define SDLPANEL_W 16
#define SDLPANEL_H 16

class SDLPanel : public PanelDriver {
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
		unsigned char framebuffer[SDLPANEL_W*SDLPANEL_H*3];
};
