module InstructionFetchStage(
    input wire clk,
    input wire rst,
    input wire [31:0] pc,              // 当前PC
    input wire branch,                 // 分支指令
    input wire branch_taken,           // 分支是否执行
    input wire [31:0] target_address,  // 分支目标地址
    input wire jump,                   // 跳转指令
    input wire [31:0] jump_address,    // 跳转目标地址
    input wire predict_taken,          // 分支预测结果
    input wire [31:0] predict_target,  // 预测目标地址
    output wire [31:0] next_pc         // 下一条PC
);
    reg [31:0] pc_reg;
    
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            pc_reg <= 32'b0;
        end else begin
            // 优先级：实际跳转 > 分支预测 > 顺序执行
            if (jump) begin
                pc_reg <= jump_address;
            end else if (branch && branch_taken) begin
                pc_reg <= target_address;
            end else if (predict_taken) begin
                pc_reg <= predict_target;
            end else begin
                pc_reg <= pc + 4;
            end
        end
    end
    
    assign next_pc = pc_reg;
    
endmodule