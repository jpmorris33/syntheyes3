#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gifload.hpp"
#include <gif_lib.h>
#include "platforms/Platform.hpp"

extern Platform *sys;

int readFunc(GifFileType *gif, GifByteType *gbyte, int bytes);

int readFunc(GifFileType *gif, GifByteType *gbyte, int bytes) {
	FileIO *fp = (FileIO *)gif->UserData;
	return (int)fp->read(gbyte,bytes);
}

GIFANIM *loadgif(const char *path) {

	unsigned char pal[768];
	unsigned char oldpal[768];
	memset(pal,0,768);
	int transparency = -1;
	int error;

	FileIO *handle = sys->openFile(path, "rb");
	GifFileType* gif = DGifOpen((void *)handle, readFunc, &error);
	if(!gif) {
	        return NULL;
	}

	if(DGifSlurp(gif) == GIF_ERROR) {
		DGifCloseFile(gif, &error);
		sys->closeFile(handle);
		return NULL;
	}

	GIFANIM* out = (GIFANIM*)calloc(1, sizeof(GIFANIM));
	if(!out) {
		DGifCloseFile(gif, &error);
		sys->closeFile(handle);
		return NULL;
	}
	out->w = gif->SWidth;
	out->h = gif->SHeight;

	if(gif->SColorMap) {
		int palptr=0;
		for(int ctr=0;ctr<gif->SColorMap->ColorCount;ctr++) {
			pal[palptr++] = gif->SColorMap->Colors[ctr].Red;
			pal[palptr++] = gif->SColorMap->Colors[ctr].Green;
			pal[palptr++] = gif->SColorMap->Colors[ctr].Blue;
		}
        }
	memcpy(oldpal,pal,768); // Back up original global palette

	out->frames = gif->ImageCount;
	out->frame = (GIFFRAME *)calloc(1, sizeof(GIFFRAME)*out->frames);
	if(!out->frame) {
		DGifCloseFile(gif, &error);
		free(out);
		sys->closeFile(handle);
		return NULL;
	}

	for(int ctr=0;ctr<out->frames;ctr++) {
		SavedImage *inframe = &gif->SavedImages[ctr];
		GIFFRAME *outframe = &out->frame[ctr];

		// Patch the working palette rather than storing it for later		
		if(inframe->ImageDesc.ColorMap) {
			int palptr=0;
			for(int ctr2=0;ctr2<inframe->ImageDesc.ColorMap->ColorCount;ctr2++) {
				pal[palptr++] = inframe->ImageDesc.ColorMap->Colors[ctr].Red;
				pal[palptr++] = inframe->ImageDesc.ColorMap->Colors[ctr].Green;
				pal[palptr++] = inframe->ImageDesc.ColorMap->Colors[ctr].Blue;
			}
		}

		// Now look for delay and transparency
		for(int ctr2=0;ctr2<inframe->ExtensionBlockCount;ctr2++) {
			if(inframe->ExtensionBlocks[ctr2].Function == GRAPHICS_EXT_FUNC_CODE) {
				unsigned char block[4];
				memcpy(block, inframe->ExtensionBlocks[ctr2].Bytes, 4);
		                // Check for transparency
		                if(block[0] & 0x01) {
					transparency = block[3];
				}
                
                		// Disposal mode
                		// frame->overlay_previous = ((block[0] & 0x08) != 0x08);
                
		                // Delay
				outframe->delay = 10*(block[1] + block[2]*256);
			}
		}
        
		int pixels = out->w * out->h;
		outframe->imgdata = (unsigned char *)calloc(3,pixels);
		if(!outframe->imgdata) {
			// Ow
			exit(1);
		}

		unsigned char *outptr=outframe->imgdata;
		for(int p=0;p<pixels;p++) {
			if(transparency < 0 || inframe->RasterBits[p] != transparency) {
				memcpy(outptr,&pal[inframe->RasterBits[p] * 3],3);
			}
			outptr+=3;
		}
	}
    
	DGifCloseFile(gif, &error);
	sys->closeFile(handle);
	
	return out;
}

