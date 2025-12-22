module Controller(
    input [5:0] Op, Funct,  //6位Op码和6位功能码
    output reg RegWrite,
    output reg MemToReg,
    output reg MemWrite,
    output reg ALUSrc,
    output reg RegDst,
    output reg Branch,
    output reg Jump,
    output reg [2:0] ALUControl
);

always @(*) begin
    RegWrite = 1'b0;
    MemToReg = 1'b0;
    MemWrite = 1'b0;
    ALUSrc = 1'b0;
    RegDst = 1'b0;
    Branch = 1'b0;
    Jump = 1'b0;
    ALUControl = 3'b000;
    
    case (Op)
        6'b000000: begin // R-type指令
            RegWrite = 1'b1;
            RegDst = 1'b1;
            case (Funct)
                6'b100000: ALUControl = 3'b010; // add
                6'b100010: ALUControl = 3'b110; // sub
                default: ALUControl = 3'b000;
            endcase
        end
        
        6'b001000: begin // addi
            RegWrite = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 3'b010; // add
        end
        
        6'b100011: begin // lw
            RegWrite = 1'b1;
            MemToReg = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 3'b010; // add
        end
        
        6'b101011: begin // sw
            MemWrite = 1'b1;
            ALUSrc = 1'b1;
            ALUControl = 3'b010; // add
        end
        
        6'b000100: begin // beq
            Branch = 1'b1;
            ALUControl = 3'b110; // sub
        end
        
        6'b000010:Jump = 1'b1;
        
        default: ; // 其他情况保持默认值
    endcase
end

endmodule