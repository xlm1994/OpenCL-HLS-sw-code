# OpenCL HLS Software code

Software code for running OpenCL HLS bit file on FPGA. Based on the [code provided in lab5](http://www.hlam.ece.ufl.edu/A-EEL4720_5721Spring2019/Labs/Lab5/provided_code.zip).

## Changes

* main.cpp:  
customized to fit the control generated by Vivado HLS
* Board.h, Board.cpp:  
 *Board::read*  and  *Board::write*  are changed to templates, so that they can handle float and char as well

## Issues

Current when trying to run it the console prompts:

```
ERROR: No boards available. Try again later.
```

This is likely problem with the code, rather than the board. No other debug info is provided by the console.