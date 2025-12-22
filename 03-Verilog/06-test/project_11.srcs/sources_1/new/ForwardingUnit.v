`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/11 11:43:30
// Design Name: 
// Module Name: ForwardingUnit
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


module ForwardingUnit(
    input wire [4:0] ID_EX_rs,
    input wire [4:0] ID_EX_rt,
    input wire EX_MA_RegWrite,
    input wire [4:0] EX_MA_WriteReg,
    input wire MA_WB_RegWrite,
    input wire [4:0] MA_WB_WriteReg,
    output reg [1:0] ForwardA,
    output reg [1:0] ForwardB
);

always @(*) begin
    // ForwardA: 00无前递, 01MEM前递, 10EX前递
    // EX冒险(优先级高)
    if (EX_MA_RegWrite && (EX_MA_WriteReg != 5'b00000) && 
        (EX_MA_WriteReg == ID_EX_rs))
        ForwardA = 2'b10;
    // MEM冒险
    else if (MA_WB_RegWrite && (MA_WB_WriteReg != 5'b00000) && 
             (MA_WB_WriteReg == ID_EX_rs))
        ForwardA = 2'b01;
    else
        ForwardA = 2'b00;
    
    // ForwardB
    if (EX_MA_RegWrite && (EX_MA_WriteReg != 5'b00000) && 
        (EX_MA_WriteReg == ID_EX_rt))
        ForwardB = 2'b10;

    else if (MA_WB_RegWrite && (MA_WB_WriteReg != 5'b00000) && 
             (MA_WB_WriteReg == ID_EX_rt))
        ForwardB = 2'b01;
    else
        ForwardB = 2'b00;
end

endmodule
