#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

namespace BMP {
struct Header {
    std::array<char, 2> signature{};
    uint32 file_size{};
    uint32 reserved{0};
    uint32 data_offset{};
};

struct InfoHeader {
    uint32 size{};
    uint32 width{};
    uint32 height{};
    uint16 planes{};
    uint16 bit_count{};
    uint32 compression{};
    uint32 image_size{};
    uint32 x_pixels_per_m{};
    uint32 y_pixels_per_m{};
    uint32 colors_used{};
    uint32 important_colors{};
};

struct PixelData {
    uint32 pixel_data{};
};
} // namespace BMP

struct Buf {
    unsigned char *data{};
    int pos{};
    int size{};
};

Buf new_buf(void *p, std::size_t size) {
    Buf r;
    assert(size < 0x40000000);
    r.data = static_cast<uint8 *>(p);
    r.size = static_cast<int>(size);
    r.pos = 0;
    return r;
}

uint8 buf_get8(Buf &b) {
    uint8 v = 0;
    assert(b.pos < b.size);
    v = b.data[b.pos];
    b.pos++;
    return v;
}

uint16 buf_get16(Buf &b) {
    uint16 v = 0;
    assert(b.pos + 1 < b.size);
    v |= b.data[b.pos + 1];
    v = (v << 8) | b.data[b.pos];
    b.pos += 2;
    return v;
}

uint32 buf_get32(Buf &b) {
    uint32 v = 0;
    assert(b.pos + 3 < b.size);
    v |= b.data[b.pos + 3];
    v = (v << 8) | b.data[b.pos + 2];
    v = (v << 8) | b.data[b.pos + 1];
    v = (v << 8) | b.data[b.pos];
    b.pos += 4;
    return v;
}

void displayColorTable(uint32 color, int counter) {
    int len = std::to_string(color).length();

    switch (counter) {
    case 0:
        if (len == 1) {
            std::cout << "R: " << color << "  ";
        } else if (len == 2) {
            std::cout << "R: " << color << ' ';
        } else {
            std::cout << "R: " << color;
        }
        break;
    case 1:
        if (len == 1) {
            std::cout << "G: " << color << "  ";
        } else if (len == 2) {
            std::cout << "G: " << color << ' ';
        } else {
            std::cout << "G: " << color;
        }
        break;
    case 2:
        if (len == 1) {
            std::cout << "B: " << color << "  ";
        } else if (len == 2) {
            std::cout << "B: " << color << ' ';
        } else {
            std::cout << "B: " << color;
        }
        break;
    default:;
    }
    std::cout << " | ";
}

void showFileInfo(const BMP::Header &header, const BMP::InfoHeader &info_header,
                  const std::unique_ptr<char> &color_table, Buf &b) {
    std::cout << "\nFile details\n";
    std::cout << "------------\n";

    std::cout << "\nHeader:\n";
    std::cout << "> Signature: ";
    for (int i = 0; i < 2; i++) {
        std::cout << header.signature[i];
    }
    std::cout << "\n> File size: " << header.file_size << '\n';
    std::cout << "> Reserved: " << header.reserved << '\n';
    std::cout << "> Data offset: " << header.data_offset << '\n';

    std::cout << "\nInfo Header:\n";
    std::cout << "> Info Header size: " << info_header.size << '\n';
    std::cout << "> Width: " << info_header.width << '\n';
    std::cout << "> Height: " << info_header.height << '\n';
    std::cout << "> Planes: " << info_header.planes << '\n';

    std::cout << "> Bits per px: " << info_header.bit_count << '\n';
    std::cout << "--> Num desc: ";
    switch (info_header.bit_count) {
    case 1:
        std::cout << "monochrome palette\n";
        break;
    case 4:
        std::cout << "4bit palletized\n";
        break;
    case 8:
        std::cout << "8bit palletized\n";
        break;
    case 16:
        std::cout << "16bit RGB\n";
        break;
    case 24:
        std::cout << "24bit RGB\n";
        break;
    }
    std::cout << "--> Max num colors: ";
    switch (info_header.bit_count) {
    case 1:
        std::cout << "1\n";
        break;
    case 4:
        std::cout << "16\n";
        break;
    case 8:
        std::cout << "256\n";
        break;
    case 16:
        std::cout << "65536\n";
        break;
    case 24:
        std::cout << "16M\n";
        break;
    default:;
    }

    std::cout << "> Compression: " << info_header.compression << '\n';
    std::cout << "--> Type: ";
    switch (info_header.compression) {
    case 0:
        std::cout << "BI_RGB (no compression)\n";
        break;
    case 1:
        std::cout << "BI_RLE8 (8bit RLE encoding)\n";
        break;
    case 2:
        std::cout << "BI_RLE4 (4bit RLE encoding)\n";
        break;
    default:;
    }

    std::cout << "> Image size: " << info_header.image_size << '\n';
    std::cout << "> X px per M: " << info_header.x_pixels_per_m << '\n';
    std::cout << "> Y px per M: " << info_header.y_pixels_per_m << '\n';
    std::cout << "> Colors used: " << info_header.colors_used << '\n';
    std::cout << "> Important colors: " << info_header.important_colors
              << "\n\n";

    if (color_table != nullptr) {
        int counter{0};
        int splitter{0};
        int entries_count{(static_cast<int>(header.data_offset) - b.pos) / 4};
        std::cout << "Color Table:\n";
        std::cout << "> Total entries count: " << entries_count << '\n';
        std::cout << "> RGB values (3x" << entries_count / 3 << " table):\n";
        while (b.pos < static_cast<int>(header.data_offset)) {
            uint32 color = buf_get8(b);
            if (counter < 3) {
                displayColorTable(color, counter);
                counter++;
                splitter++;
            } else {
                counter = 0;
            }
            if (splitter >= 9) {
                std::cout << '\n';
                splitter = 0;
            }
        }
    } else {
        std::cout << "Color table not found\n";
    }
}

void parseHeaderData(BMP::Header &header, Buf &b) {
    header.signature[0] = buf_get8(b);
    header.signature[1] = buf_get8(b);
    header.file_size = buf_get32(b);
    header.reserved = buf_get32(b);
    header.data_offset = buf_get32(b);
}

void parseInfoHeaderData(BMP::InfoHeader &info_header, Buf &b) {
    info_header.size = buf_get32(b);
    info_header.width = buf_get32(b);
    info_header.height = buf_get32(b);
    info_header.planes = buf_get16(b);
    info_header.bit_count = buf_get16(b);
    info_header.compression = buf_get32(b);
    info_header.image_size = buf_get32(b);
    info_header.x_pixels_per_m = buf_get32(b);
    info_header.y_pixels_per_m = buf_get32(b);
    info_header.colors_used = buf_get32(b);
    info_header.important_colors = buf_get32(b);
}

void parseFileData(Buf &b) {
    BMP::Header header{};
    BMP::InfoHeader info_header{};

    parseHeaderData(header, b);
    parseInfoHeaderData(info_header, b);

    std::unique_ptr<char> color_table;
    // std::unique_ptr<char> raster_data;

    if (info_header.bit_count <= 8) {
        color_table =
            std::unique_ptr<char>(new char[info_header.bit_count * 4]);
    } else {
        color_table = nullptr;
    }

    // b.pos = header.data_offset;
    // raster_data = std::unique_ptr<char>(new char[info_header.image_size]);
    // parseRasterData(raster_data, b);

    showFileInfo(header, info_header, color_table, b);
}

/**
 * Loading given file to the memblock to be worked on
 *
 * @param: char *path; pointer to the path of the file
 * @return: void
 */
void loadFile(char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        std::cout << "> File opened\n";

        std::streampos size;
        std::cout << "> Memblock created\n";

        size = file.tellg();
        std::unique_ptr<char> memblock{new char[size]};
        file.seekg(0, std::ios::beg);
        file.read(memblock.get(), size);
        std::cout << "> File read\n";

        Buf b{new_buf(memblock.get(), size)};
        std::cout << "> New buffer created\n";

        file.close();
        std::cout << "> File closed\n";

        parseFileData(b);
    } else {
        std::cerr << "Error: " << strerror(errno) << '\n';
    }
};

int main([[maybe_unused]] int argc, char *argv[]) {
    loadFile(argv[1]);

    return 0;
}
