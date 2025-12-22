`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/17 21:15:24
// Design Name: 
// Module Name: fulladder
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


module fulladder(S, Co, A, B, Ci);
    input A, B, Ci;
    output S, Co;
    wire S1, D1, D2;
    
    halfadder h1(S1, D1, A, B);
    halfadder h2(S, D2, S1, Ci);
    
    assign Co = D1 | D2;            
endmodule
