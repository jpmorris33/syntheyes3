#include "../PanelDriver.hpp"

class Hub75Pi : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
		void blit(unsigned char pwmClock);	// Helper thread needs to call this
	private:
		unsigned char *outbuffer;
		unsigned char *outline;
};
