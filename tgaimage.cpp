#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include <tgaimage.h>

using namespace std;

TGAImage::TGAImage() : data(NULL), width(0), height(0), bytespp(0) {}

TGAImage::TGAImage(int w, int h, int bpp) : data(NULL), width(w), height(h), bytespp(bpp) {
    unsigned long nBytes = width * height * bytespp;
    data = new unsigned char[nBytes];
    memset(data, 0, nBytes);
}

TGAImage::TGAImage(const TGAImage* img) {
    width = img.width;
    height = img.height;
    bytespp = img.bytespp;
    unsigned long nBytes = width * height * bytespp;
    data = new unsigned char[nBytes];
    memcpy(data, img.data, nBytes);
}

TGAImage::~TGAImage() {
    if (data) {
        delete{} data;
    }
}
