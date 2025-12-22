`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/17 21:08:48
// Design Name: 
// Module Name: ADD
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


module ADD(Fw, A, B, EN);
    output reg [31:0] Fw;
    input [31:0] A, B;
    input EN;
    
    wire [32:0] C;    
    wire [31:0] Fww;   
    
    genvar i;
    assign C[0] = 0;  
    
    generate  
       for(i=0; i<32; i=i+1) begin : adder_loop
           fulladder FA(Fww[i], C[i+1], A[i], B[i], C[i]); 
       end
    endgenerate
     
    always @(*) begin
        if(EN) begin
            Fw = Fww;
        end else begin
            Fw = 32'bz;
        end
     end
endmodule
