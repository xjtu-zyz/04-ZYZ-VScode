module ControlUnit(
    input wire [5:0] opcode,
    input wire [5:0] funct,
    output reg RegWrite,
    output reg MemToReg,
    output reg MemWrite,
    output reg ALUSrc,
    output reg RegDst,
    output reg Branch,
    output reg Jump,
    output reg [3:0] ALUControl
);

always @(*) begin
    // 默认值
    RegWrite = 1'b0;
    MemToReg = 1'b0;
    MemWrite = 1'b0;
    ALUSrc = 1'b0;
    RegDst = 1'b0;
    Branch = 1'b0;
    Jump = 1'b0;
    ALUControl = 4'b0000;
    
    case (opcode)
        6'b000000: begin // R-type指令
            RegWrite = 1'b1;
            RegDst = 1'b1;
            case (funct)
                6'b100000: ALUControl = 4'b0100; // add
                6'b100010: ALUControl = 4'b0101; // sub
                default: ALUControl = 4'b0000;
            endcase
        end
        
        6'b001000: begin // addi
            RegWrite = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 4'b0100; // add
        end
        
        6'b100011: begin // lw
            RegWrite = 1'b1;
            MemToReg = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 4'b0100; // add
        end
        
        6'b101011: begin // sw
            MemWrite = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 4'b0100; // add
        end
        
        6'b000100: begin // beq
            Branch = 1'b1;
            ALUControl = 4'b0101; // sub
        end
        
        6'b000010: begin // j
            Jump = 1'b1;
        end
        
        default: begin // nop
            // 保持默认值
        end
    endcase
end

endmodule
