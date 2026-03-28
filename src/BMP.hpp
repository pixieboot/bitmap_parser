#ifndef BMP_H_
#define BMP_H_

#include "buffer.hpp"
#include "term_config.hpp"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

struct Header {
  uint16 file_type{0};
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

struct BMP {
  Header header{0};
  InfoHeader info_header{0};
  InfoHeaderV5Extras info_header_v5{0};
  std::vector<uint8_t> data{0};

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
      int32 entries_count{(static_cast<int32>(header.data_offset) - b.pos) / 4};
      std::cout << "Color Table:\n";
      std::cout << "> Total entries count: " << entries_count << '\n';
      std::cout << "> RGB values (3x" << entries_count / 3 << " table):\n\n";
      while (b.pos < static_cast<int32>(header.data_offset)) {
        uint32 color = b.bufGet8(b);
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
    if (header.file_type == 19778) {
      // NOTE: maybe consider a different approach for this
      std::cout << "> File type: BM\n";
    } else {
      std::cout << "> Unrecognized file type\n";
    }
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
    std::cout << "> Colors used: " << info_header.colors_in_color_table << '\n';
    std::cout << "> Important colors: " << info_header.important_colors
              << "\n\n";

    if (info_header.size == 124) {
      std::cout << "Info Header V5 Extras:\n";
      std::cout << "> Red ch bitmask: " << info_header_v5.r_channel_bitmask
                << '\n';
      std::cout << "> Green ch bitmask: " << info_header_v5.g_channel_bitmask
                << '\n';
      std::cout << "> Blue ch bitmask: " << info_header_v5.b_channel_bitmask
                << '\n';
      std::cout << "> Alpha ch bitmask: " << info_header_v5.a_channel_bitmask
                << '\n';
      std::cout << "> Color space type: " << info_header_v5.color_space_type
                << '\n';
      std::cout << "> Color space endpoints: "
                << info_header_v5.color_space_endpoints << '\n';
      std::cout << "> Gamma red endpoints: " << info_header_v5.gamma_r_endpoints
                << '\n';
      std::cout << "> Gamma green endpoints: "
                << info_header_v5.gamma_g_endpoints << '\n';
      std::cout << "> Gamma blue endpoints: "
                << info_header_v5.gamma_b_endpoints << '\n';
      std::cout << "> Intent: " << info_header_v5.intent << '\n';
      std::cout << "> ICC profile data: " << info_header_v5.icc_profile_data
                << '\n';
      std::cout << "> ICC profile size: " << info_header_v5.icc_profile_size
                << '\n';
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
    header.file_type = Buf::bufGet16(b);
    if (header.file_type != 19778) {
      throw std::runtime_error("> Error: Unsupported file format!\n");
    }
    header.file_size = Buf::bufGet32(b);
    header.reserved = Buf::bufGet32(b);
    header.data_offset = Buf::bufGet32(b);
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
    info_header.size = Buf::bufGet32(b);
    info_header.width = Buf::bufGet32(b);
    info_header.height = Buf::bufGet32(b);
    info_header.planes = Buf::bufGet16(b);
    info_header.bit_count = Buf::bufGet16(b);
    info_header.compression = Buf::bufGet32(b);
    info_header.image_size = Buf::bufGet32(b);
    info_header.x_pixels_per_meter = Buf::bufGet32(b);
    info_header.y_pixels_per_meter = Buf::bufGet32(b);
    info_header.colors_in_color_table = Buf::bufGet32(b);
    info_header.important_colors = Buf::bufGet32(b);
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
    info_header_v5.r_channel_bitmask = Buf::bufGet32(b);
    info_header_v5.g_channel_bitmask = Buf::bufGet32(b);
    info_header_v5.b_channel_bitmask = Buf::bufGet32(b);
    info_header_v5.a_channel_bitmask = Buf::bufGet32(b);
    info_header_v5.color_space_type = Buf::bufGet32(b);
    info_header_v5.color_space_endpoints = Buf::bufGet32(b);
    info_header_v5.gamma_r_endpoints = Buf::bufGet32(b);
    info_header_v5.gamma_g_endpoints = Buf::bufGet32(b);
    info_header_v5.gamma_b_endpoints = Buf::bufGet32(b);
    info_header_v5.intent = Buf::bufGet32(b);
    info_header_v5.icc_profile_data = Buf::bufGet32(b);
    info_header_v5.icc_profile_size = Buf::bufGet32(b);
    info_header_v5.reserved = Buf::bufGet32(b);
  }

  /**
   * Core func for file parsing and seperating how data will be allocated
   *
   * @param: Buf &b; custom buffer struct
   * @param: const std::string &rgb; --rgb arg
   * @return: void;
   */
  void parseData(Buf &b, const std::string &args) {
    BMP::parseHeaderData(b, header);
    BMP::parseInfoHeaderData(b, info_header);

    if (info_header.size == 124) {
      parseInfoHeaderV5Data(b, info_header_v5);
    }

    std::unique_ptr<char> color_table;

    if (info_header.bit_count <= 8) {
      color_table = std::unique_ptr<char>(new char[info_header.bit_count * 4]);
    } else {
      color_table = nullptr;
    }

    showFileInfo(b, args, header, info_header, info_header_v5, color_table);
  }

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
    std::cout << "-h\tShows help for argument inputs\n";
  }

  void writeDataToStructs(int32 width, int32 height, bool has_alpha = 1) {
    if (width <= 0 || height <= 0) {
      throw std::runtime_error(
          "Image height and width must be positive numbers\n");
    }

    info_header.width = width;
    info_header.height = height;

    if (has_alpha) {
      info_header.size = sizeof(InfoHeader) + sizeof(InfoHeaderV5Extras);
      header.data_offset =
          sizeof(Header) + sizeof(InfoHeader) + sizeof(InfoHeaderV5Extras);

      info_header.bit_count = 32;
      info_header.compression = 3;
      row_stride = width * 4;
      data.resize(row_stride * height);
      header.file_size = header.data_offset + data.size();
    } else {
      info_header.size = sizeof(InfoHeader);
      header.data_offset = sizeof(Header) + sizeof(InfoHeader);

      // NOTE: check later for different bit counts
      info_header.bit_count = 24;
      info_header.compression = 0;
      row_stride = width * 3;
      data.resize(row_stride * height);

      uint32 new_stride = makeStrideAligned(4);
      header.file_size = header.data_offset + static_cast<uint32>(data.size()) +
                         info_header.height * (new_stride - row_stride);
    }
  }

  void fillRegion(uint32 x0, uint32 y0, uint32 w, uint32 h, uint8 B, uint8 G,
                  uint8 R, uint8 A) {
    if (x0 + w > static_cast<uint32>(info_header.width) ||
        y0 + h > static_cast<uint32>(info_header.height)) {
      throw std::runtime_error("Image region doesn't fit the image!\n");
    }

    uint32 channels = info_header.bit_count / 8;
    for (uint32 y = y0; y < y0 + h; ++y) {
      for (uint32 x = x0; x < x0 + w; ++x) {
        data[channels * (y * info_header.width + x) + 0] = B;
        data[channels * (y * info_header.width + x) + 1] = G;
        data[channels * (y * info_header.width + x) + 2] = R;
        if (channels == 4) {
          data[channels * (y * info_header.width + x) + 3] = A;
        }
      }
    }
  }

  void setPixel(uint32 x0, uint32 y0, uint8 B, uint8 G, uint8 R, uint32 A) {
    if (x0 >= static_cast<uint32>(info_header.width) ||
        y0 >= static_cast<uint32>(info_header.height)) {
      throw std::runtime_error("Image region doesn't fit the image!\n");
    }

    uint32 channels = info_header.bit_count / 8;
    data[channels * (y0 * info_header.width + x0) + 0] = B;
    data[channels * (y0 * info_header.width + x0) + 1] = G;
    data[channels * (y0 * info_header.width + x0) + 2] = R;
    if (channels == 4) {
      data[channels * (y0 * info_header.width + x0) + 3] = A;
    }
  }

  void writeFile(const std::string &fname) {
    std::ofstream file{fname, std::ios::binary};
    if (file) {
      writeHeadersAndData(file);
    } else {
      throw std::runtime_error("Unable to open the output image file\n");
    }
  }

  void parseInput(TermArgsConfig &term) {
    std::string_view help = "help";
    std::string_view h = "-h";

    if (!h.compare(term.arg1) || !help.compare(term.arg1)) {
      BMP::showManual();
      return;
    }

    std::string_view read = "-r";
    std::string_view write = "-w";
    std::string_view edit = "-e";

    if (!read.compare(term.arg1)) {
      Buf b{};
      b.loadFileToBuf(b, term.arg2);
      parseData(b, term.arg3);
    }

    if (!edit.compare(term.arg1)) {
      Buf b{};
      b.loadFileToBuf(b, term.arg2);
    }

    if (!write.compare(term.arg1)) {
      writeDataToStructs(std::stoi(term.arg3), std::stoi(term.arg4),
                         std::stoi(term.arg5));
      // fillRegion(0, 0, std::stoi(term.arg3), std::stoi(term.arg4), 255, 0,
      // 255,
      //            0);
      fillRegion(0, 0, 100, 100, 255, 0, 255, 0);
      writeFile(term.arg2);
    };
  }

private:
  uint32 row_stride{0};

  uint32 makeStrideAligned(uint32 align_stride) {
    uint32 new_stride = row_stride;
    while (new_stride % align_stride != 0) {
      new_stride++;
    }
    return new_stride;
  }

  void writeHeaders(std::ofstream &f) {
    f.write(reinterpret_cast<const char *>(&header), sizeof(header));
    f.write(reinterpret_cast<const char *>(&info_header), sizeof(info_header));
    if (info_header.bit_count > 8) {
      f.write(reinterpret_cast<const char *>(&info_header_v5),
              sizeof(info_header_v5));
    }
  }

  void writeHeadersAndData(std::ofstream &f) {
    writeHeaders(f);
    f.write(reinterpret_cast<const char *>(data.data()), data.size());
  }
};

#endif // BMP_H_
