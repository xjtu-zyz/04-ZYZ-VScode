module MemoryStage(
    input wire [31:0] alu_result,      // EX/MEM阶段的ALU结果
    input wire [31:0] rs2_data,        // store指令的写入数据
    input wire [31:0] wb_data,         // 从WB阶段前推的数据
    input wire [1:0] forward_mem,      // 前推控制信号
    output wire [31:0] mem_data        // 实际写入存储器的数据
);
    // 数据冒险解决逻辑
    assign mem_data = (forward_mem == 2'b10) ? alu_result :  // 前推EX/MEM结果
                     (forward_mem == 2'b01) ? wb_data :      // 前推WB结果
                     rs2_data;                               // 原始数据
endmodule