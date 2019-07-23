// Greg Stitt
// University of Florida
// main.cpp
//
// Description: This file is the software portion of the simple pipeline 
// application implemented on the FPGA.

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>

#include "Board.h"
#include "Timer.h"

using namespace std;

#define MEM_IN_ADDR 0x40
#define MEM_OUT_ADDR 0x48

#define FILT_ADDR 0x50

#define id_x 0x10
#define id_y 0x18
#define id_z 0x20

#define glb_x 0x28
#define glb_y 0x30
#define glb_z 0x38

#define CONTROL_ADDR 0x0

#define data_t float

#define IMG_SIZE 128
#define FILTER_SIZE 5
#define BUFFER_SIZE (IMG_SIZE + FILTER_SIZE-1)
#define HALF_FILTER_SIZE (FILTER_SIZE-1)/2




// software implementation of the code implemented on the FPGA
data_t get(data_t* arr, int row, int col);
void sw(data_t *input, data_t *output, data_t *filter) {
	for(int row = 0; row < IMG_SIZE; row++){
		for(int col = 0; col < IMG_SIZE; col++){
			int my = col + row*IMG_SIZE;
			float sum = 0.0;
			for (int r = -HALF_FILTER_SIZE; r <= HALF_FILTER_SIZE; r++){
				int filter_row_offset = (r + HALF_FILTER_SIZE)*FILTER_SIZE + HALF_FILTER_SIZE;

				for (int c = -HALF_FILTER_SIZE; c <= HALF_FILTER_SIZE; c++)
				{
					int filt_ind = filter_row_offset + c;

					sum += get(input, row+r, col+c) * filter[ filt_ind ];
				}
			}
			output[my] = sum;
		}
	}
}
data_t get(data_t* arr, int row, int col){
	if(row < 0 || row >= IMG_SIZE || col < 0 || col >= IMG_SIZE)
		return 0.0;
	else
		return arr[row*IMG_SIZE + col];
}


int main(int argc, char* argv[]) {
  
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " bitfile" << endl;
    return -1;
  }
  
  cout << "Main start" << endl;

  // setup clock frequencies
  vector<float> clocks(Board::NUM_FPGA_CLOCKS);
  clocks[0] = 100.0;
  clocks[1] = 0.0;
  clocks[2] = 0.0;
  clocks[3] = 0.0;
  
  // initialize board
  Board *board;
  try {
    board = new Board(argv[1], clocks);
  }
  catch(...) {
    exit(-1);
  }

  char control;
  data_t *swOutput, *hwOutput;
  Timer swTime, hwTime, writeTime, waitTime;

  // load hw input and sw input
  data_t input[] = {
    #include "lena128pad.txt"
  };
  data_t sw_input[] = {
    #include "lena128.txt"
  };
  
  // initialize convolution filter, here it uses an averaging filter
  data_t filter[5*5];
  for (int i=0; i<25; i++){
    filter[i] = (data_t)1/25;
  }

  hwOutput = new data_t[IMG_SIZE*IMG_SIZE];
  swOutput = new data_t[IMG_SIZE*IMG_SIZE];
  assert(swOutput != NULL);
  assert(hwOutput != NULL);

  // initialize output arrays
  for (unsigned i=0; i < IMG_SIZE*IMG_SIZE; i++) {
    swOutput[i] = 0.0;
    hwOutput[i] = 0.0;
  }
  // execute the code in software
  swTime.start();
  sw(sw_input, swOutput, filter);
  swTime.stop();

  // write the pointer to input/output/filter to corresponding addr
  hwTime.start();
  writeTime.start();
  board->write(&input, MEM_IN_ADDR, 1);
  board->write(&hwOutput, MEM_OUT_ADDR, 1);
  board->write(&filter, FILT_ADDR, 1);
  
  // set group id and global offset to 0
  unsigned zero = 0;
  board->write(&zero, id_x, 1);
  board->write(&zero, id_y, 1);
  board->write(&zero, id_z, 1);
  board->write(&zero, glb_x, 1);
  board->write(&zero, glb_y, 1);
  board->write(&zero, glb_z, 1);
  writeTime.stop();

  // set ap_start to 1
  board->read(&control, CONTROL_ADDR, 1);
  control = control | 1;
  board->write(&control, CONTROL_ADDR, 1);
    
  // wait for ap_done to be 1
  waitTime.start();
  while (!control&2) {
    board->read(&control, CONTROL_ADDR, 1);
  }
  waitTime.stop();

/*
  // read the outputs back from the FPGA
  readTime.start();
  board->read(hwOutput, MEM_OUT_ADDR, size);
  readTime.stop();
*/
  hwTime.stop();


  printf("Results:\n");
  for (unsigned i=0; i < size; i++) {
    printf("%d: HW = %d, SW = %d\n", i, hwOutput[i], swOutput[i]);
  }

  // calculate speedup
  double transferTime = writeTime.elapsedTime();
  double hwTimeNoTransfer = hwTime.elapsedTime() - transferTime;
  cout << "Speedup: " << swTime.elapsedTime()/hwTime.elapsedTime() << endl;
  cout << "Speedup (no transfers): " << swTime.elapsedTime()/hwTimeNoTransfer << endl;

  delete hwOutput;
  delete swOutput;
  return 0;
}
