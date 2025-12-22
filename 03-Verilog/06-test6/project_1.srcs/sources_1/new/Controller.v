module Controller(
    input [5:0] Op, Funct,//6位Op码和6位功能码
    input Zero,
    output MemToReg, MemWrite,
    output PCSrc, ALUSrc,
    output RegDst, RegWrite,
    output Jump,
    output [2:0] ALUControl
    output Branch
);
    wire [1:0] ALUOp;
    wire Branch_internal;
    
    MainDec MainDec_inst(
        .Op(Op),
        .MemToReg(MemToReg),
        .MemWrite(MemWrite),
        .Branch(Branch_internal),
        .ALUSrc(ALUSrc),
        .RegDst(RegDst),
        .RegWrite(RegWrite),
        .Jump(Jump),
        .ALUOp(ALUOp)
    );
    
    ALUDec ALUDec_inst(
        .Funct(Funct),
        .ALUOp(ALUOp),
        .ALUControl(ALUControl)
    );
    
    assign Branch=Branch_internal;
    assign PCSrc = (state==EXECUTE)?(Branch_internal & Zero):1'b0;
endmodule