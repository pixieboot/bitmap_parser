#ifndef buffer_H_
#define buffer_H_

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cstdint>

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;

typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;

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
  static void bufNew(Buf &b, void *p, std::size_t file_size) {
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
  static uint8 bufGet8(Buf &b) {
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
  static uint16 bufGet16(Buf &b) {
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
  static uint32 bufGet32(Buf &b) {
    uint32 v = 0;
    assert(b.pos + 3 < b.size);
    v |= b.data[b.pos + 3];
    v = (v << 8) | b.data[b.pos + 2];
    v = (v << 8) | b.data[b.pos + 1];
    v = (v << 8) | b.data[b.pos];
    b.pos += 4;
    return v;
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

      Buf::bufNew(b, memblock, size);
      std::cout << "> Total buffer memory: " << size << "b\n";

      file.close();
      std::cout << "> File closed\n";
    } else {
      std::cerr << "> Error: ";
      throw std::runtime_error(strerror(errno));
    }
  };
};

#endif // buffer_H_
