#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>

using namespace std;

typedef char cbyte;
typedef unsigned int uint;
/*
typedef struct BMP_Data {
    char byte;
    unsigned int
};*/

struct Header {
    short signature;
    int fileSize;
    int reserved;
    int dataOffset;
};

struct InfoHeader {
    int size;
    int width;
    int height;
    short planes;
    short bitsPerPixel;
    int compression;
    int imageSize;
    int xPixelsPerM;
    int yPixelsperM;
    int colorsUsed;
    int importantColors;
};

struct ColorTable {
    cbyte R;
    cbyte G;
    cbyte B;
};

// get file size
void fileSize(std::string& path) {
  streampos begin, end;

  ifstream file (path, ios::binary);

  begin = file.tellg();
  file.seekg(0, ios::end);
  end = file.tellg();
  cout << "File size: " << (end - begin) << " bytes.\n";
  file.close();
};

// get memblock
fstream getMemblock(std::string& path) {
  streampos size;
  char* memblock;

  fstream file (path, ios::binary|ios::ate);

  if(file.is_open()) {
    size = file.tellg();
    memblock = new char [size];
    file.seekg(0, ios::beg);
    file.read(memblock, size);

    delete[] memblock;
  }
};

int main(int argc, char* argv[])
{
  // streampos size;
  // char* memblock;

  std::string path {"images/all_gray.bmp"};

  fileSize(path);
  return 0;
}
