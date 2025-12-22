`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/17 21:11:07
// Design Name: 
// Module Name: SLL
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


module SLL(F, A, B, EN);
    output reg [31:0] F;
    input [31:0] A, B;
    input EN;
    
    always @(*) begin
        if(EN == 1) F = B << A[4:0];
        else F = 32'bz;
    end
endmodule
