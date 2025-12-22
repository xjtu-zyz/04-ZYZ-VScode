`timescale 1ns / 1ps

module DataMemory(
    input wire clk,
    input wire MemWrite,
    input wire [31:0] Address,
    input wire [31:0] WriteData,
    output wire [31:0] ReadData
);

reg [31:0] mem [0:255];

integer i;
initial begin
    for (i = 0; i < 256; i = i + 1)
        mem[i] = 32'h00000000;
    mem[0] = 32'h0000000A;   // µØÖ·0´æ´¢10
    mem[1] = 32'h00000088;  //1 88h
    mem[25] = 32'h00000005;  // µØÖ·100´æ´¢5
end

always @(posedge clk) begin
    if (MemWrite)
        mem[Address[9:2]] <= WriteData;
end

assign ReadData = mem[Address[9:2]];

endmodule
