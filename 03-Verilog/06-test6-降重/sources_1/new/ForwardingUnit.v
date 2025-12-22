module ForwardingUnit(
    input [4:0] ID_EX_rs,
    input [4:0] ID_EX_rt,
    input EX_MEM_RegWrite,
    input [4:0] EX_MEM_WriteReg,
    input MEM_WB_RegWrite,
    input [4:0] MEM_WB_WriteReg,
    output reg [1:0] ForwardA,
    output reg [1:0] ForwardB
);

always @(*) begin
    // ForwardA: EX_MEM_RegWrite=1说明EX阶段有写寄存器操作
    // 写寄存器地址=ID阶段的源寄存器相同
    if (EX_MEM_RegWrite && (EX_MEM_WriteReg == ID_EX_rs) && (EX_MEM_WriteReg != 5'b00000))
        ForwardA = 2'b10;
        // 10代表从EX阶段向前递
    else if (MEM_WB_RegWrite && (MEM_WB_WriteReg == ID_EX_rs) && (MEM_WB_WriteReg != 5'b00000))
        ForwardA = 2'b01; 
        // 01代表从MEM阶段向前递
    else
        ForwardA = 2'b00;
    
    // ForwardB
    if (EX_MEM_RegWrite && (EX_MEM_WriteReg == ID_EX_rt) && (EX_MEM_WriteReg != 5'b00000))
        ForwardB = 2'b10;
    else if (MEM_WB_RegWrite && (MEM_WB_WriteReg == ID_EX_rt) && (MEM_WB_WriteReg != 5'b00000))
        ForwardB = 2'b01;
    else
        ForwardB = 2'b00;
end

endmodule