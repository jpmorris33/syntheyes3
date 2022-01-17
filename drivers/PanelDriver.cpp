#include "PanelDriver.hpp"

void PanelDriver::init() {}
void PanelDriver::draw() {}
void PanelDriver::drawMirrored() {}
void PanelDriver::setPattern(unsigned char pattern[16][16]) {}
void PanelDriver::updateRGB(unsigned char *img, int w, int h) {}
void PanelDriver::updateRGB(unsigned char *img, int w, int h, uint32_t colour) {}
void PanelDriver::updateRGBpattern(unsigned char *img, int w, int h, int offset) {}
void PanelDriver::clear(uint32_t colour) {}
