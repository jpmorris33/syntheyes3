
#include "../PanelDriver.hpp"

class SDLPanel : public PanelDriver {
	public:
		void init(const char *param);
		void draw();
		void drawMirrored();
		void draw180();
		void draw180Mirrored();
		uint32_t getCaps();
	private:
		struct SDL_Window *win;
		struct SDL_Renderer *renderer;
		struct SDL_Texture *texture;
		unsigned char *outbuffer;
};
