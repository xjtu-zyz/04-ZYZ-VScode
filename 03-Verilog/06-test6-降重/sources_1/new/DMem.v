module DMem(
  input CLK,
  input MemWrite,
  input [31:0]A,
  input [31:0] WD,//writeData
  output [31:0] RD//readData
);
   parameter DATA_WIDTH = 32;   // 数据宽度参数
   parameter DMEM_SIZE = 256;   // 存储器大小

  reg [DATA_WIDTH-1:0] memory [0:DMEM_SIZE-1];
  integer i;
  
  initial begin
      for (i = 0; i < 256; i = i + 1)
          memory[i] = 32'b0;
  end  
  always @(posedge CLK) begin
      if (MemWrite) begin
          memory[A[9:2]] <= WD;
      end
  end
  assign RD = memory[A[9:2]];
endmodule