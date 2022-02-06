//
//	Colour processing tables
//

unsigned char colourscale[256][256];
unsigned char lightpattern_saw[16];
unsigned char lightpattern_triangle[16];

void init_colourutils() {

	// Build intensity lookup table
	for(int intensity=0;intensity<256;intensity++) {
		for(int ctr=0;ctr<256;ctr++) {
			colourscale[intensity][ctr]=(ctr * intensity)>>8;
			if(ctr > 0 && colourscale[intensity][ctr] < 1) {
				colourscale[intensity][ctr]=1;
			}
		}
	}


	// Build default colour ramps
	for(int ctr=0;ctr<16;ctr++) {
		lightpattern_saw[ctr]=ctr*16;
	}
	for(int ctr=0;ctr<8;ctr++) {
		lightpattern_triangle[ctr]=ctr*32;
		lightpattern_triangle[15-ctr]=ctr*32;
	}

}