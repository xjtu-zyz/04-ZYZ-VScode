`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2025/12/11 11:28:12
// Design Name: 
// Module Name: ALU
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


module ALU(
    input wire [31:0] A,
    input wire [31:0] B,
    input wire [3:0] ALUControl,
    output reg [31:0] Result,
    output wire Zero
);

parameter ALU_AND = 4'b0000;
    parameter ALU_OR = 4'b0001;
    parameter ALU_XOR = 4'b0010;
    parameter ALU_NOR = 4'b0011;
    parameter ALU_ADD = 4'b0100;
    parameter ALU_SUB = 4'b0101;
    parameter ALU_SLT = 4'b0110;
    parameter ALU_SLL = 4'b0111;
    
    wire [7:0] EN;
    wire [31:0] Fw,Fa;
    
    assign Fa = A&B;
    
    always @(*)begin
        case(ALUControl)
            ALU_AND: begin Result<= Fa; end
            ALU_OR: begin Result<= A| B; end
            ALU_XOR: begin Result<=A^B; end
            ALU_NOR: begin Result<= ~(A|B); end
            default: Result = Fw;
        endcase
    end
    Decoder38 decoder38_1(ALUControl[2:0],EN);
    ADD add_1(Fw,A,B,EN[4]);
    SUB sub_1(Fw,A,B,EN[5]);
    SLT slt_1(Fw,A,B,EN[6]);
    SLL sll_1(Fw,A,B,EN[7]);

assign Zero = (Result == 32'h00000000);

endmodule

