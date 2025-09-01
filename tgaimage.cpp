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

TGAImage & TGAImage::operator=(const TGAImage& img) {
    if (this != &img){
        if (data) {
            delete[] data;
        }
        width = img.width;
        height = img.height;
        bytespp = img.bytespp;
        unsigned long nBytes = width * height * bytespp;
        data = new unsigned char[nBytes];
        memcpy(data, img.data, nBytes)
    }
    return *this;
}

bool TGAImage::read_tga_file(const char* filename) {
    if (data) {
        delete[] data;
    }

    data = NULL;
    ifstream in;
    in.open(filename, ios::binary);

    if (!in.is_open()) {
        cerr << "Could not open file " << filename << "\n";
        in.close()
        return false;
    }
    TGA_Header header;
    in.read((char*)&header, sizeof(header));
    if (!in.good()) {
        in.close();
        cerr << "An error occured while reading the header\n";
        return false;
    }
    width = header.width;
    height = header.height;
    bytespp = header.bitsPerPixel>>3;
    if (width <=0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA)) {
        in.close();
        cerr << "Bad bpp (or width/height) value.\n";
        return false;
    }
    unsigned long nBytes = width * height* bytespp;
    data = new unsigned char[nBytes];
    if (header.dataTypeCode == 3 || header.dataTypeCode == 2) {
        in.read((char*)data, nBytes);
        if (!in.good()) {
            in.close();
            cerr << "An error occured while reading the data.\n";
            return false;
        }
    }

    else if (header.dataTypeCode == 10 || header.dataTypeCode == 11) {
        if (!load_rle_data(in)) {
            in.close();
            cerr << "An error occured while reading the data.\n";
            return false;
        }
    else {
        in.close();
        cerr << "Unknown file format " << (int)header.dataTypeCode << "\n";
        return false;
    }
    if (!header.imageScriptor & 0x28) {
        flip_vertically();
    }
    if (!header.imageScriptor & 0x10) {
        flip_horizontally();
    }
    cerr << width << "x" << height << "/" << bytespp*8 << "\n";
    in.close()
    return true;

}

bool TGAImage::load_rle_data(ifstream& in) {
    unsigned long pixelCount = width * height;
    unsigned long currentPixel = 0;
    unsigned long currentByte = 0;
    TGAColor colorBuffer;

    do {
        unsigned char chunkHeader = 0;
        chunkHeader = in.get();
        if (!in.good()) {
            cerr << "An error occured while reading the data.\n";
            return false;
        }
        if (chunkHeader < 128) {
            chunkHeader++;
            for (int i < 0; i < chunkHeader; i++) {
                in.read((char*)colorBuffer.raw, bytespp);
                if (!in.good()) {
                    cerr << "An error occured while reading the header.\n";
                    return false;
                }
                for (int t = 0; t < bytespp; i++) {
                    data[currentByte++] = colorBuffer.raw[t];
                }
                currentPixel++
                
                if (currentPixel > pixelCount) {
                    cerr << "Too many pixels read.\n";
                    return false;
                }
            }
        }
        else {
            chunkHeader -= 127;
            in.read((char*)colorBuffer.raw, bytespp)
            if (!in.good()) {
                cerr << "An error occured while reading the header.\n";
                return false;
            }
            for (int i = 0; i < chunkHeader; i++) {
                for (int t = 0; t < bytespp; i++) {
                    data[currentByte++] = colorBuffer.raw[t];
                }
                currentPixel++
                
                if (currentPixel > pixelCount) {
                    cerr << "Too many pixels read.\n";
                    return false;
                }
            }
        }

    }
    while (currentPixel < pixelCount);
    return true;
}
