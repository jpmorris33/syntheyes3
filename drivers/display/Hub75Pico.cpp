/**
 * Render to a HUB75 panel using the Pico's programmable IO state machines.
 * Portions based on the Pico HUB75 example, Copyright (c) 2020 Raspberry Pi (Trading) Ltd under the BSD-3-Clause license
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Hub75Pico.hpp"
#include "../Timing.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hardware/gpio.h"
#include "hub75.pio.h"

#define HUBPANEL_W 64
#define HUBPANEL_H 32

#define SM_DATA 0
#define SM_ROW 1

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define CLK_PIN 11
#define STROBE_PIN 12
#define OEN_PIN 13

#define REFRESH_MS 12	// 16ms is about 60fps
static Timing *refresh;

//
//	Init the PIO programs.
//	This is based on code from the Pico HUB75 example.
//

void Hub75Pico::init(const char *param) {

	panelW = HUBPANEL_W;
	panelH = HUBPANEL_H;

	printf("*Init Hub75 panel driver\n");

	const char *p = getDriverParam(param, "w");
	if(p) {
		int w = getDriverInt(p);
		if(w == 32 || w == 64) {
			panelW = w;
		}
		printf("*Hub75 display set width to %d\n",panelW);
	}
	p = getDriverParam(param, "h");
	if(p) {
		int h = getDriverInt(p);
		if(h == 16 || h == 32) {
			panelH = h;
		}
		printf("*Hub75 display set height to %d\n",panelH);
	}

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);	// Single eye RGB
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	outbuffer = (uint32_t *)calloc(sizeof(uint32_t),panelW*panelH*2);	// Both eyes as int32's
	if(!outbuffer) {
		printf("Failed to allocate outbuffer\n");
		exit(1);
	}

//	TODO: probably want to reserve the pins later

	rowpins = panelH == 32 ? 4 : 3;	// 4 = 32 pixel height, 3 = 16 pixels.  Don't yet support H=64 (5 pins)
	pio = pio0;
	data_prog_offset = pio_add_program(pio, &hub75_data_rgb888_program);
	row_prog_offset = pio_add_program(pio, &hub75_row_program);
	hub75_data_rgb888_program_init(pio, SM_DATA, data_prog_offset, DATA_BASE_PIN, CLK_PIN);
	hub75_row_program_init(pio, SM_ROW, row_prog_offset, ROWSEL_BASE_PIN, rowpins, STROBE_PIN);

	refresh = get_timer();
	refresh->set(0);

}

uint32_t Hub75Pico::getCaps() {
	return 0; // Not fixed, not monochrome
}


//
//	Convert the framebuffer to the format the Hub75 display engine wants and blit it to the panel
//
void Hub75Pico::draw() {
	unsigned char *inptr = framebuffer;
	uint32_t *outptr = outbuffer;

	for(int y=0;y<panelH;y++) {
		unsigned char *start = inptr;
		for(int x=0;x<panelW;x++) {
			*outptr++ = ((inptr[0] << 16) | (inptr[1] << 8) | inptr[2]);
			inptr+=3;
		}
		// Now do it again for the second panel
		inptr = start;
		for(int x=0;x<panelW;x++) {
			*outptr++ = ((inptr[0] << 16) | (inptr[1] << 8) | inptr[2]);
			inptr+=3;
		}
	}

	blit();
}

//
//	And again, but with the second panel mirrored
//

void Hub75Pico::drawMirrored() {
	unsigned char *inptr = framebuffer;
	uint32_t *outptr = outbuffer;

	for(int y=0;y<panelH;y++) {
		unsigned char *start = inptr;
		for(int x=0;x<panelW;x++) {
			*outptr++ = ((inptr[0] << 16) | (inptr[1] << 8) | inptr[2]);
			inptr+=3;
		}
		// Now walk backwards to mirror the image on the second panel
		for(int x=0;x<panelW;x++) {
			inptr-=3;
			*outptr++ = ((inptr[0] << 16) | (inptr[1] << 8) | inptr[2]);
		}

		// now fast-forward to the next row
		inptr = start + (panelW * 3);
	}

	blit();
}

//
//	Put the framebuffer onto the HUB75 panels using the PIO state machines.
//	This is based on code from the Pico HUB75 example.
//

void Hub75Pico::blit() {

	// Save bandwidth by not always updating
	if(refresh->elapsed()) {
		refresh->set(REFRESH_MS);
	} else {
		return;
	}

	uint32_t *row1,*row2,*rowptr1,*rowptr2;
	int w = panelW * 2;

	for (int rowsel = 0; rowsel < (1 << rowpins); ++rowsel) {
		row1 = &outbuffer[rowsel * w];
		row2 = &outbuffer[((1u << rowpins)+ rowsel) * w];
		for (int bit = 0; bit < 8; ++bit) {
			rowptr1=row1;
			rowptr2=row2;
			hub75_data_rgb888_set_shift(pio, SM_DATA, data_prog_offset, bit);
			for (int x = 0; x < w; ++x) {
				pio_sm_put_blocking(pio, SM_DATA, *rowptr1++);
				pio_sm_put_blocking(pio, SM_DATA, *rowptr2++);
			}
		        // Dummy pixel per lane
		        pio_sm_put_blocking(pio, SM_DATA, 0);
		        pio_sm_put_blocking(pio, SM_DATA, 0);
		        // SM is finished when it stalls on empty TX FIFO
		        hub75_wait_tx_stall(pio, SM_DATA);
		        // Also check that previous OEn pulse is finished, else things can get out of sequence
		        hub75_wait_tx_stall(pio, SM_ROW);

		        // Latch row data, pulse output enable for new row.
		        pio_sm_put_blocking(pio, SM_ROW, rowsel | (100u * (1u << bit) << 5));
		}
	}
}
