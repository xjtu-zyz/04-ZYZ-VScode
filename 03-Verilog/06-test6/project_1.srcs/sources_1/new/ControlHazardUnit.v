/*
module ControlHazardUnit(
    input Branch,          // 分支指令
    input Zero,           // ALU零标志
    input Jump,           // 跳转指令
    input PredictTaken,   // 预测是否跳转
    output reg Flush,     // 冲刷信号
    output reg PCSrc      // PC选择信号
);
    always @(*) begin
        Flush = 1'b0;
        PCSrc = 1'b0;
        
        // 实际分支结果

        
        // 检测预测错误
        if (Branch && (PredictTaken !=PCSrc)) begin
            Flush = 1'b1;  // 预测错误，需要冲刷
        end
        
        // 跳转指令处理
        if (Jump) begin
            Flush = 1'b1;  // 跳转需要冲刷
            PCSrc = 1'b1;  // PC选择跳转目标
        end
    end
endmodule
*/
module ControlHazardUnit(
    input wire Branch,         // EX阶段：是否为分支指令
    input wire Zero,           // EX阶段：ALU结果是否为0（分支条件）
    input wire Jump,           // EX阶段：是否为跳转指令
    input wire PredictTaken,   // 预测是否跳转
    output reg Flush,          // 流水线冲刷信号
    output reg PCSrc           // 分支成功标志（PC应跳转到分支目标）
);

always @(*) begin
    // 分支成功：Branch有效且Zero为1
    PCSrc = Branch & Zero;
    
    // 需冲刷流水线的情况：
    // 1. 分支成功（无论预测是否正确）
    // 2. 跳转指令有效
    Flush = (Branch & Zero) | Jump;
end

endmodule