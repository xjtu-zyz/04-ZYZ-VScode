module ExecuteStage(
    input wire [31:0] alu_result,      // EX/MEM阶段的ALU结果
    input wire [31:0] rs2_data,        // 从ID/EX寄存器读取的rs2数据
    input wire [31:0] wb_data,         // 从WB阶段前推的数据
    input wire [1:0] forward_ex,       // 前推控制信号
    output wire [31:0] alu_in2         // 前推后的ALU输入2
);
    // 数据冒险解决逻辑
    assign alu_in2 = (forward_ex == 2'b10) ? alu_result :  // 前推EX/MEM结果
                    (forward_ex == 2'b01) ? wb_data :      // 前推WB结果
                    rs2_data;                              // 原始数据
endmodule