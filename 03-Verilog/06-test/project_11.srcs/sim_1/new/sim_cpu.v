`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/13 16:54:09
// Design Name: 
// Module Name: sim_cpu
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module sim_cpu;

  reg clk;
  reg rst;

  // DUT
  FiveStageCPU dut (
    .clk(clk),
    .rst(rst)
  );

  // Clock: 10ns period
  initial begin
    clk = 1'b0;
    forever #5 clk = ~clk;
  end

  // Reset + run
  initial begin


    rst = 1'b1;
    repeat(4) @(posedge clk);
    rst = 1'b0;

    // Run long enough to pass all IM init tests (tune if needed)
    repeat(120) @(posedge clk);
    $finish;
  end

endmodule
