// Greg Stitt
// University of Florida
// Board class
// This class implements a number of methods for interfacing with the FPGA. For the ZedBoard, this "board" is just the programmable logic section, but the class maintains the board analogy for use with other FPGAs that are on a separate board.

#ifndef _BOARD_H_
#define _BOARD_H_

#include <string>
#include <vector>

// starting address of AXI memory map
#define AXI_MMAP_ADDR 0x43c00000

// total size in bytes of memory-map address space
#define MEM_INT_ADDR_SPACE (1 << 20)

// total size in words of memory-map address psace
#define MMAP_ADDR_WIDTH 18

// bit width of each memory-map word
#define MMAP_DATA_WIDTH 32

#include <cmath>
#include <stdlib.h>

enum MemId {
  MEM_INTERNAL,
  MEM_LAST  // this is an invalid memory and is used for bounds checking only
};


class Board {

 public:
  Board(const char *bitfile, const std::vector<float> &frequencies);
  virtual ~Board();
  /*
  virtual bool write(unsigned *data, unsigned long addr, unsigned long words);
  virtual bool read(unsigned *data, unsigned long addr, unsigned long words);
  */
  template <typename T>
  bool write(T *data, unsigned long addr, unsigned long words) {
  
    // identify the starting page, the number of pages for the 
    // entire transfer, and the addr within the starting page
    unsigned page = addr*sizeof(T) / PAGE_SIZE;
    unsigned pages = ceil(words*sizeof(T) / (float) PAGE_SIZE);
    addr = addr % (PAGE_SIZE/4);
  
    // for each page, transfer the corresponding data
    for (unsigned i=0; i < pages; i++) {
  
      unsigned pageWords = words > PAGE_SIZE/4 ? PAGE_SIZE/4 - addr : words;
      memcpy(mmapPages[page]+addr, data+i*PAGE_SIZE/4, pageWords*sizeof(T));
      addr = 0;
      words -= pageWords;
      page ++;
    }
  
    return true;
  }
  
  
  template <typename T>
  bool read(T *data, unsigned long addr, unsigned long words) {
  
  // identify the starting page, the number of pages for the 
    // entire transfer, and the addr within the starting page
    unsigned page = addr*sizeof(T) / PAGE_SIZE;
    unsigned pages = ceil(words*sizeof(T) / (float) PAGE_SIZE);
    addr = addr % (PAGE_SIZE/4);
  
    // for each page, transfer the corresponding data
    for (unsigned i=0; i < pages; i++) {
  
      unsigned pageWords = words > PAGE_SIZE/4 ? PAGE_SIZE/4 - addr : words;
      memcpy(data+i*PAGE_SIZE/4, mmapPages[page]+addr, pageWords*sizeof(T));
      addr = 0;
      words -= pageWords;
      page ++;
    }
  
    return true;
  }

  // number of bytes in a page
  const unsigned PAGE_SIZE;
  
  static const unsigned NUM_FPGA_CLOCKS = 4;

 protected:
      
  // pointer to each page used in the memory-mapped AXI address space.
  unsigned **mmapPages;

  void copy(const char *to, const char *from);
  void loadBitfile(const char* bitfile);
  void writeToDriver(std::string file, std::string data) const;
  std::string readFromDriver(std::string file) const;
  void configureFpgaClock(unsigned clk, double freq);
  void configureFpgaClocks(const std::vector<float> &frequencies);
  void initializeMemoryMap();
  void handleError(std::string str) const;
};

#endif
