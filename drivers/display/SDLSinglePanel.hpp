
#include "../PanelDriver.hpp"

class SDLSinglePanel : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		uint32_t getCaps();
	private:
		struct SDL_Window *win;
		struct SDL_Renderer *renderer;
		struct SDL_Texture *texture;
		unsigned char *outbuffer;
};
