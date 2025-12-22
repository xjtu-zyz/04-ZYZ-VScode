`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/17 21:10:59
// Design Name: 
// Module Name: SLT
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


module SLT(F, A, B, EN);
    output reg [31:0] F;
    input [31:0] A, B;
    input EN;
    
    always @(*) begin
        if(EN == 1) begin 
            if($signed(A) < $signed(B)) F = 1;
            else F = 0;
        end
        else F = 32'bz;
    end
endmodule
