module MainDec(
    input [5:0] Op,
    output MemToReg, MemWrite,
    output Branch, ALUSrc,
    output RegDst, RegWrite,
    output Jump,
    output [1:0] ALUOp
);
    // 使用查找表方式定义控制信号
    reg [8:0] controls;
    
    // 控制信号顺序: [RegWrite, RegDst, ALUSrc, Branch, MemWrite, MemToReg, Jump, ALUOp[1:0]]
    always @(*) begin
        case(Op)
            6'b000000: controls = 9'b110000010; // R-type   //ADD/SUB/AND/OR/SLT/NOP
            6'b001101: controls = 9'b101000001; // ori      //没用到
            6'b100011: controls = 9'b101001000; // lw
            6'b101011: controls = 9'b001010000; // sw
            6'b000100: controls = 9'b000100001; // beq
            6'b001111: controls = 9'b101000011; // lui      //没用到
            6'b001000: controls = 9'b101000000; // addi (新增)
            6'b000010: controls = 9'b000000100; // j (新增)
            default:   controls = 9'b000000000; // 默认
        endcase
    end
    
    assign {RegWrite, RegDst, ALUSrc, Branch, MemWrite, MemToReg, Jump, ALUOp} = controls;
endmodule