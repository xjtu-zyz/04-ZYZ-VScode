// module ALU(
//     input [31:0] srcA, srcB,
//     input [2:0] ALUControl,
//     output reg [31:0] ALUResult,
//     output Zero
// );
//     always @(*) begin
//         case(ALUControl)
//             3'b000: ALUResult = srcA & srcB;   // and
//             3'b001: ALUResult = srcA | srcB;   // or
//             3'b010: ALUResult = srcA + srcB;   // add
//             3'b110: ALUResult = srcA - srcB;   // subtract
//             3'b100: ALUResult = {srcB[15:0], 16'b0}; // lui
//             3'b111: ALUResult = (srcA < srcB) ? 32'b1 : 32'b0; // slt (新增)
//             default: ALUResult = srcA + srcB;  // 默认加法
//         endcase
//     end
    
//     assign Zero = (ALUResult == 32'b0);
// endmodule
module ALU(OP, A, B, F, ZF);
    parameter SIZE = 32; //运算位数
    input [2:0] OP; //运算操作
    input [SIZE-1:0] A; //左运算数
    input [SIZE-1:0] B; //右运算数
    output [SIZE-1:0] F; //运算结果
    output ZF; //0标志位，运算结果为0(全零)则图1，否则图0


    reg [SIZE-1:0] F;
    reg ZF; //C为最高位进位

    always@(*)
    begin
        C = 0;
        case(OP)
        3'b000: F = A&B;    // and
        3'b001: F = A|B;    // or
        3'b010: F = A + B;  // add     //不考虑溢出
        3'b110: F = A - B;  // sub
        3'b100: F = {B[15:0], 16'b0};   // lui
        3'b111: F = (A < B)? 32'b1 : 32'b0; // slt (新增)
        default: F = A + B;  // 默认加法
        endcase
        ZF = F == 0; //F全为0，则ZF=1
    end
endmodule
