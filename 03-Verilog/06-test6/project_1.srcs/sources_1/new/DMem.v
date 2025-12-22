module DMem(
  input CLK,
  input WE,//MemWrite
  input [31:0]A,
  input [31:0] WD,//writeData
  output [31:0] RD//readData
);
  reg [31:0] memory [0:1023];
  integer i;
  
  initial begin
      for (i = 0; i < 1024; i = i + 1)
          memory[i] = 32'b0;
  end
  
  assign RD = memory[A[11:2]];
  
  always @(posedge CLK) begin
      if (WE) begin
          memory[A[11:2]] <= WD;
      end
  end
endmodule
