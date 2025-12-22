module ForwardingUnit(
    input [4:0] ID_EX_rs,
    input [4:0] ID_EX_rt,
    input EX_MEM_RegWrite,
    input [4:0] EX_MEM_WriteReg,
    input MEM_WB_RegWrite,
    input [4:0] MEM_WB_WriteReg,
    output reg [1:0] forwardA,
    output reg [1:0] forwardB
);
    always @(*) begin
        // 初始化
        forwardA = 2'b00; // 00: 从寄存器文件读取
        forwardB = 2'b00; // 00: 从寄存器文件读取
        
        // EX冒险：前推EX/MEM阶段的结果
        // 10: 前推EX/MEM的ALU结果
        if (EX_MEM_RegWrite && (EX_MEM_WriteReg != 0) && (EX_MEM_WriteReg == ID_EX_rs)) begin
            forwardA = 2'b10;
        end
        
        if (EX_MEM_RegWrite && (EX_MEM_WriteReg != 0) && (EX_MEM_WriteReg == ID_EX_rt)) begin
            forwardB = 2'b10;
        end
        
        // MEM冒险：前推MEM/WB阶段的结果
        // 01: 前推MEM/WB的写回数据
        // 注意：EX/MEM前推优先级更高
        if (MEM_WB_RegWrite && (MEM_WB_WriteReg != 0) && 
            !(EX_MEM_RegWrite && (EX_MEM_WriteReg != 0) && (EX_MEM_WriteReg == ID_EX_rs)) &&
            (MEM_WB_WriteReg == ID_EX_rs)) begin
            forwardA = 2'b01;
        end
        
        if (MEM_WB_RegWrite && (MEM_WB_WriteReg != 0) && 
            !(EX_MEM_RegWrite && (EX_MEM_WriteReg != 0) && (EX_MEM_WriteReg == ID_EX_rt)) &&
            (MEM_WB_WriteReg == ID_EX_rt)) begin
            forwardB = 2'b01;
        end
    end
endmodule