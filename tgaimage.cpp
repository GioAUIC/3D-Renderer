#ifndef __IMAGE_H_
#define __IMAGE_H_

#include <fstream>

#pragma pack(push, 1)

struct TGA_Header {
    char idlength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsPerPixel;
    char imagesCriptor;
};
#pragma pack(pop)

