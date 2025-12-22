module ControlUnit(
    input clk,
    input [5:0] Opcode,
    output reg RegDst,
    output reg ALUSrc,
    output reg MemtoReg,
    output reg RegWrite,
    output reg MemRead,
    output reg MemWrite,
    output reg Branch,
    output reg [1:0] ALUOp
);
    always @(posedge clk) begin
        case (Opcode)
            6'b000000: begin // R型指令
                RegDst   = 1;
                ALUSrc   = 0;
                MemtoReg = 0;
                RegWrite = 1;
                MemRead  = 0;
                MemWrite = 0;
                Branch   = 0;
                ALUOp    = 2'b10;
            end
            6'b100011: begin // LW
               RegDst   = 0;
               ALUSrc   = 1;
               MemtoReg = 1;
               RegWrite = 1;
               MemRead  = 1;
               MemWrite = 0;
               Branch   = 0;
               ALUOp    = 2'b00;
           end
           6'b101011: begin // SW
               RegDst   = 0; // 无意义
               ALUSrc   = 1;
               MemtoReg = 0; // 无意义
               RegWrite = 0;
               MemRead  = 0;
               MemWrite = 1;
               Branch   = 0;
               ALUOp    = 2'b00;
           end
           6'b000100: begin // BEQ
               RegDst   = 0; // 无意义
               ALUSrc   = 0;
               MemtoReg = 0; // 无意义
               RegWrite = 0;
               MemRead  = 0;
               MemWrite = 0;
               Branch   = 1;
               ALUOp    = 2'b01;
           end
           6'b001101: begin // ORI
              RegDst   = 0;
              ALUSrc   = 1;
              MemtoReg = 0;
              RegWrite = 1;
              MemRead  = 0;
              MemWrite = 0;
              Branch   = 0;
              ALUOp    = 2'b11;
          end
          6'b001111: begin // LUI
              RegDst   = 0;
              ALUSrc   = 1;
              MemtoReg = 0;
              RegWrite = 1;
              MemRead  = 0;
              MemWrite = 0;
              Branch   = 0;
              ALUOp    = 2'b00; // ALU操作不重要
          end
          default: begin // NOP或未知指令
              RegDst   = 0;
              ALUSrc   = 0;
              MemtoReg = 0;
              RegWrite = 0;
              MemRead  = 0;
              MemWrite = 0;
              Branch   = 0;
              ALUOp    = 2'b00;
          end
      endcase
  end
endmodule

