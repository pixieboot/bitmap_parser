#ifndef BMP_H_
#define BMP_H_

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;

struct Header {
    uint16 file_type{0x4D42};
    uint32 file_size{0};
    uint32 reserved{0};
    uint32 data_offset{0};
};

struct InfoHeader {
    uint32 size{0};
    int32 width{0};
    int32 height{0};
    uint16 planes{1};
    uint16 bit_count{0};
    uint32 compression{0};
    uint32 image_size{0};
    int32 x_pixels_per_meter{0};
    int32 y_pixels_per_meter{0};
    uint32 colors_in_color_table{0};
    uint32 important_colors{0};
};

struct InfoHeaderV5Extras {
    uint32 r_channel_bitmask{0};
    uint32 g_channel_bitmask{0};
    uint32 b_channel_bitmask{0};
    uint32 a_channel_bitmask{0};
    uint32 color_space_type{0};
    uint32 color_space_endpoints{0};
    uint32 gamma_r_endpoints{0};
    uint32 gamma_g_endpoints{0};
    uint32 gamma_b_endpoints{0};
    uint32 intent{0};
    uint32 icc_profile_data{0};
    uint32 icc_profile_size{0};
    uint32 reserved{0};
};

struct PixelData {
    uint32 pixel_data{};
};

struct Buf {
    unsigned char *data{};
    int32 pos{};
    int32 size{};

    /**
     * Constructor for new custom buffer
     *
     * @param: void *p; pointer to the begging of the memory block to be stored
     * to the new buffer
     * @param: std::size_t size; size of the new buffer
     */
    static void buf_load(Buf &b, void *p, std::size_t file_size) {
        assert(file_size < 0x40000000); // 1gb limit
        b.data = static_cast<uint8 *>(p);
        b.size = static_cast<int32>(file_size);
        b.pos = 0;
    }

    /**
     * Parsing single bytes from the buffer
     *
     * @param: Buf &b; custom buffer struct
     * @return: uchar; parsed byte that has been converted to unsigned char
     */
    static uint8 buf_get8(Buf &b) {
        uint8 v = 0;
        assert(b.pos < b.size);
        v = b.data[b.pos];
        b.pos++;
        return v;
    }

    /**
     * Parsing single bytes from the buffer
     *
     * @param: Buf &b; custom buffer struct
     * @return: uint16; parsed 2-byte that has been converted to unsigned short
     */
    static uint16 buf_get16(Buf &b) {
        uint16 v = 0;
        assert(b.pos + 1 < b.size);
        v |= b.data[b.pos + 1];
        v = (v << 8) | b.data[b.pos];
        b.pos += 2;
        return v;
    }

    /**
     * Parsing single bytes from the buffer
     *
     * @param: Buf &b; custom buffer struct
     * @return: uint32; parsed 4-byte that has been converted to unsigned int
     */
    static uint32 buf_get32(Buf &b) {
        uint32 v = 0;
        assert(b.pos + 3 < b.size);
        v |= b.data[b.pos + 3];
        v = (v << 8) | b.data[b.pos + 2];
        v = (v << 8) | b.data[b.pos + 1];
        v = (v << 8) | b.data[b.pos];
        b.pos += 4;
        return v;
    }
};

struct config {
    std::string arg1{};
    std::string arg2{};
};

struct BMP {
    Header header{};
    InfoHeader info_header{};
    InfoHeaderV5Extras info_header_v5{};

    /**
     * Displays a matrix table 3x(something) with all RGB values of table
     * entries
     *
     * @param: uint32 color; which color is being parsed (current byte being
     * parsed)
     * @param: int counter; counter for color seperation (0: R, 1: G, 2: B)
     * @return: void;
     */
    void showColorTable(Buf &b, const std::unique_ptr<char> &color_table) {
        if (color_table != nullptr) {
            int32 counter{0};
            int32 splitter{0};
            int32 entries_count{
                (static_cast<int32>(header.data_offset) - b.pos) / 4};
            std::cout << "Color Table:\n";
            std::cout << "> Total entries count: " << entries_count << '\n';
            std::cout << "> RGB values (3x" << entries_count / 3
                      << " table):\n\n";
            while (b.pos < static_cast<int32>(header.data_offset)) {
                uint32 color = b.buf_get8(b);
                int32 len = std::to_string(color).length();
                if (counter < 3) {
                    switch (counter) {
                    case 0:
                        if (len == 1) {
                            std::cout << " R: " << color << "   ";
                        } else if (len == 2) {
                            std::cout << " R: " << color << "  ";
                        } else {
                            std::cout << " R: " << color << ' ';
                        }
                        break;
                    case 1:
                        if (len == 1) {
                            std::cout << " G: " << color << "   ";
                        } else if (len == 2) {
                            std::cout << " G: " << color << "  ";
                        } else {
                            std::cout << " G: " << color << ' ';
                        }
                        break;
                    case 2:
                        if (len == 1) {
                            std::cout << " B: " << color << "   |";
                        } else if (len == 2) {
                            std::cout << " B: " << color << "  |";
                        } else {
                            std::cout << " B: " << color << " |";
                        }
                        break;
                    default:;
                    }
                    std::cout << "|";
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
            std::cout << "Color table not found!\n";
            std::cout << "Color tables and it's RGB values are only present if "
                         "Bits per pixels are "
                         "less or equal to 8\n";
        }
    }

    /**
     * Writes out all the data that has been parsed from the .bmp image out to
     * the term
     *
     * @param: Header &header; header info struct
     * @param: InfoHeader &info_header; info header struct with details about
     * the .bmp image
     * @param: const std::unique_ptr<char> &color_table: pointer to the first
     * byte of the color table entries
     * @param: Buf &b; custom buffer struct
     * @param: const std::string &rgb; --rgb arg
     * @return: void;
     */
    void showFileInfo(Buf &b, const std::string &arg, const Header &header,
                      const InfoHeader &info_header,
                      const InfoHeaderV5Extras &info_header_v5,
                      const std::unique_ptr<char> &color_table) {
        std::cout << "\nFile details\n";
        std::cout << "------------\n";

        std::cout << "\nHeader:\n";
        std::cout << "> File type: " << header.file_type << '\n';
        std::cout << "> File size: " << header.file_size << "b\n";
        std::cout << "> Reserved: " << header.reserved << '\n';
        std::cout << "> Data offset: " << header.data_offset << '\n';

        std::cout << "\nInfo Header:\n";
        std::cout << "> Size: " << info_header.size << "b\n";
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
            std::cout << "16bit RGB high-color\n";
            break;
        case 24:
            std::cout << "24bit RGB true-color\n";
            break;
        case 32:
            std::cout << "32bit RGB deep-color\n";
        default:
            std::cout << '\n';
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
            std::cout << "16M+\n";
            break;
        case 32:
            std::cout << "1B+\n";
            break;
        default:
            std::cout << '\n';
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
        case 3:
            std::cout << "BI_BITFIELDS (Huffman 1D encoding)\n";
            break;
        case 4:
            std::cout << "BI_JPEG (24bit RLE encoding)\n";
            break;
        case 5:
            std::cout << "BI_PNG (24bit RLE encoding)\n";
            break;
        case 6:
            std::cout << "BI_ALPHABITFIELDS (RGBA bit field masks)\n";
            break;
        case 11:
            std::cout << "BI_CMYK\n";
            break;
        case 12:
            std::cout << "BI_CMYK8 (8bit RLE encoding)\n";
            break;
        case 13:
            std::cout << "BI_CMYK4 (4bit RLE encoding)\n";
            break;
        default:
            std::cout << '\n';
        }

        std::cout << "> Image size: " << info_header.image_size << '\n';
        std::cout << "> X px per M: " << info_header.x_pixels_per_meter << '\n';
        std::cout << "> Y px per M: " << info_header.y_pixels_per_meter << '\n';
        std::cout << "> Colors used: " << info_header.colors_in_color_table
                  << '\n';
        std::cout << "> Important colors: " << info_header.important_colors
                  << "\n\n";

        if (info_header.size == 124) {
            std::cout << "Info Header V5 Extras:\n";
            std::cout << "> Red ch bitmask: "
                      << info_header_v5.r_channel_bitmask << '\n';
            std::cout << "> Green ch bitmask: "
                      << info_header_v5.g_channel_bitmask << '\n';
            std::cout << "> Blue ch bitmask: "
                      << info_header_v5.b_channel_bitmask << '\n';
            std::cout << "> Alpha ch bitmask: "
                      << info_header_v5.a_channel_bitmask << '\n';
            std::cout << "> Color space type: "
                      << info_header_v5.color_space_type << '\n';
            std::cout << "> Color space endpoints: "
                      << info_header_v5.color_space_endpoints << '\n';
            std::cout << "> Gamma red endpoints: "
                      << info_header_v5.gamma_r_endpoints << '\n';
            std::cout << "> Gamma green endpoints: "
                      << info_header_v5.gamma_g_endpoints << '\n';
            std::cout << "> Gamma blue endpoints: "
                      << info_header_v5.gamma_b_endpoints << '\n';
            std::cout << "> Intent: " << info_header_v5.intent << '\n';
            std::cout << "> ICC profile data: "
                      << info_header_v5.icc_profile_data << '\n';
            std::cout << "> ICC profile size: "
                      << info_header_v5.icc_profile_size << '\n';
            std::cout << "> Reserved: " << info_header_v5.reserved << "\n\n";
        }

        if (arg == "--rgb") {
            showColorTable(b, color_table);
        }
    }

    /**
     * Func for parsing header bytes
     *
     * @param: Header &header; header struct with some main details
     * @param: Buf &b; custom buffer struct
     * @return: void;
     */
    static void parseHeaderData(Buf &b, Header &header) {
        Buf::buf_get16(b);
        header.file_size = Buf::buf_get32(b);
        header.reserved = Buf::buf_get32(b);
        header.data_offset = Buf::buf_get32(b);
    }

    /**
     * Func for parsing info_header bytes
     *
     * @param: InfoHeader &info_header; info header struct with details about
     * the .bmp image
     * @param: Buf &b; custom buffer struct
     * @return: void;
     */
    static void parseInfoHeaderData(Buf &b, InfoHeader &info_header) {
        info_header.size = Buf::buf_get32(b);
        info_header.width = Buf::buf_get32(b);
        info_header.height = Buf::buf_get32(b);
        info_header.planes = Buf::buf_get16(b);
        info_header.bit_count = Buf::buf_get16(b);
        info_header.compression = Buf::buf_get32(b);
        info_header.image_size = Buf::buf_get32(b);
        info_header.x_pixels_per_meter = Buf::buf_get32(b);
        info_header.y_pixels_per_meter = Buf::buf_get32(b);
        info_header.colors_in_color_table = Buf::buf_get32(b);
        info_header.important_colors = Buf::buf_get32(b);
    }

    /**
     * Func for parsing info_header_v5 bytes
     *
     * @param: InfoHeaderV5Extras &info_header_v5; info header version 5 struct
     * with details about the .bmp image
     * @param: Buf &b; custom buffer struct
     * @return: void;
     */
    static void parseInfoHeaderV5Data(Buf &b,
                                      InfoHeaderV5Extras &info_header_v5) {
        info_header_v5.r_channel_bitmask = Buf::buf_get32(b);
        info_header_v5.g_channel_bitmask = Buf::buf_get32(b);
        info_header_v5.b_channel_bitmask = Buf::buf_get32(b);
        info_header_v5.a_channel_bitmask = Buf::buf_get32(b);
        info_header_v5.color_space_type = Buf::buf_get32(b);
        info_header_v5.color_space_endpoints = Buf::buf_get32(b);
        info_header_v5.gamma_r_endpoints = Buf::buf_get32(b);
        info_header_v5.gamma_g_endpoints = Buf::buf_get32(b);
        info_header_v5.gamma_b_endpoints = Buf::buf_get32(b);
        info_header_v5.intent = Buf::buf_get32(b);
        info_header_v5.icc_profile_data = Buf::buf_get32(b);
        info_header_v5.icc_profile_size = Buf::buf_get32(b);
        info_header_v5.reserved = Buf::buf_get32(b);
    }

    /**
     * Core func for file parsing and seperating how data will be allocated
     *
     * @param: Buf &b; custom buffer struct
     * @param: const std::string &rgb; --rgb arg
     * @return: void;
     */
    void parseBmpData(Buf &b, const std::string &args) {

        BMP::parseHeaderData(b, header);
        BMP::parseInfoHeaderData(b, info_header);

        if (info_header.size == 124) {
            parseInfoHeaderV5Data(b, info_header_v5);
        }

        std::unique_ptr<char> color_table;
        // std::unique_ptr<char> raster_data;

        if (info_header.bit_count <= 8) {
            color_table =
                std::unique_ptr<char>(new char[info_header.bit_count * 4]);
        } else {
            color_table = nullptr;
        }

        // b.pos = header.data_offset;
        // raster_data = std::unique_ptr<char>(new
        // char[info_header.image_size]);
        showFileInfo(b, args, header, info_header, info_header_v5, color_table);
    }

    /**
     * Load a given file to the memblock to be worked on
     *
     * @param: char *path; pointer to the path of the file
     * @return: void
     */
    static void loadFileToBuf(Buf &b, std::string &path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        if (file.is_open()) {
            std::cout << "> File opened\n";

            std::streampos size;
            char *memblock;

            size = file.tellg();
            // no delete[] for this pointers as its being used throughout the
            // entirety of programs life
            memblock = new char[size];
            std::cout << "> Memblock created\n";
            file.seekg(0, std::ios::beg);
            file.read(memblock, size);
            std::cout << "> File read\n";

            Buf::buf_load(b, memblock, size);
            std::cout << "> Total buffer memory: " << size << "b\n";

            file.close();
            std::cout << "> File closed\n";
        } else {
            std::cerr << "> Error: ";
            throw std::runtime_error(strerror(errno));
        }
    };

    /**
     * Shows the manual with details
     *
     * @param: none;
     * @return: void;
     */
    static void showManual() {
        std::cout << "\nArgs:\tDescritpion:\n\n";
        std::cout << "path\tPath to the image to be parsed (e.g. "
                     "../images/example.bmp)\n";
        std::cout << "--rgb\tDisplays a color table of RGB values\n";
        std::cout << "--h\tShows help for argument inputs\n";
    }

    /**
     * Parses given args from the term
     *
     * @param: char **argv; array of args from the terminal
     * @return: struct config; data struct for args in std::string format
     * (cfg.arg1, cfg.arg2...)
     */
    static void parse_command_params(char **argv, config &cfg) {
        if (!argv[1]) {
            throw std::runtime_error("Path to the image is required as the 1st "
                                     "arg\nType help or --h for manual");
        }
        cfg.arg1 = argv[1];

        if (argv[2]) {
            cfg.arg2 = argv[2];
        }
    }
};
#endif // BMP_H_
