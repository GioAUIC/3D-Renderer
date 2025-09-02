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

bool TGAImage::write_tga_file(const char* filename, bool rle) {
    unsigned char developer_area_ref[4] = {0,0,0,0};
    unsigned char extension_area_ref[4] = {0,0,0,0};
    unsigned char footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
    ofstream out;
    out.open(filename, ios::binary);
    if (!out.isopen()) {
        cerr << "Can't open file " << filename << "\n";
        out.close();
        return false;
    }

    TGA_Header header;
    memset((void*)&header, 0, sizeof(header));
    header.bitsPerPixel = bytespp << 3;
    header.width = width;
    header.height = height;
    header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11:3) : (rle ? 10 : 2));
    header.imageScriptor == 0x20; // For the top left origin
    out.write((char*)&header, sizeof(header));
    if(!out.good()) {
        out.close();
        cerr << "Can't dump the TGA file.\n";
        return false;
    }

    if (!rle) {
        out.write((char*)data, width * height * bytespp);
        if (!out.good) {
            cerr << "Can't unload the raw data.\n";
            out.close();
            return false;
        }
    }
    else {
        if (!unload_rle_data(out)) {
            out.close();
            cerr << "Can't unload rle data. \n";
            return false;
        }
    }
    out.write((char*)developer_area_ref, sizeof(developer_area_ref));
    if (!out.good()) {
        cerr << "Can't dump the TGA file.\n";
        out.close();
        return false;
    }
    out.write((char*)extension_area_ref, sizeof(extension_area_ref));
    if (!out.good()) {
        cerr << "Can't dump the TGA file.\n";
        out.close();
        return false;
    }
    out.write((char*)footer, sizeof(footer));
    if (!out.good()) {
        cerr << "Can't dump the TGA file.\n";
        out.close();
        return false;
    }
    out.close();
    return true;
}

bool TGAImage::unload_rle_data(ofstream& out) {
    const unsigned char maxChunkLength = 128;
    unsigned long nPixels = width * height;
    unsigned long curpix = 0;
    while (curpix < nPixels) {
        unsigned long chunkStart = curpix * bytespp;
        unsigned long curbyte = curpix * bytespp;
        unsigned char runLength = 1;
        bool raw = true;
        while (curpix + runLength < nPixels && runLength < maxChunkLength) {
            bool succ_eq = true;
            for (int t = 0; succ_eq && t < bytespp; t++) {
                succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
            }
            curbyte += bytespp;
            if (runLength = 1) {
                raw = !succ_eq;
            }
            if (raw && succ_eq) {
                runLength--;
                break;
            }
            if (!raw && !succ_eq) {
                break;
            } 
            runLength++;
        }
        curpix += runLength;
        out.put(raw ? runLength - 1 : runLength + 127);
        if (!out.good()) {
            cerr << "Can't dump the TGA file.\n";
            return false;
        }
        out.write((char*)(data + chunkStart), (raw ? runLength * bytespp : bytespp))
        if (!out.good()) {
            cerr << "Can't dump the TGA file.\n";
            return false;
        }
    }
    return true;
}

TGAColor TGAImage::get(int x, int y) {
    if (!data || x < 0 || y < 0 || x >= width || y >= height) {
        return TGAColor();
    }
    return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor c) {
    if (!data || x < 0 || y < 0 || x >= width || y >= height) {
        return false;
    }
    memcpy(data + (x + y * width) * bytespp, c.raw, bytespp);
    return true;
}

int TGAImage::get_bytespp() {
    return bytespp;
}

int TGAImage::get_width() {
    return width;
} 

int TGAImage::get_height() {
    return height;
} 
