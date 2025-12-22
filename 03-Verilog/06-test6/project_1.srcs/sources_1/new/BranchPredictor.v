module BranchPredictor(
    input [31:0] pc_IF,           // 当前取指PC
    input [31:0] instr_IF,        // IF级指令
    output reg predict_taken,     // 预测是否跳转
    output [31:0] predict_target  // 预测目标地址
);
    wire [5:0] opcode = instr_IF[31:26];
    wire is_branch = (opcode == 6'b000100) ||  // BEQ
                    (opcode == 6'b000101);    // BNE
    
    // 取16-bit offset，符号扩展，再左移2位
    wire [15:0] imm = instr_IF[15:0];
    wire [31:0] imm_sext = {{16{imm[15]}}, imm};
    wire [31:0] branch_offset = imm_sext << 2;
    
    // 简单静态预测策略：向后跳预测taken，否则not taken
    always @(*) begin
        if (is_branch && imm[15]) begin  // 偏移量为负（向后跳）
            predict_taken = 1'b1;
        end else if (is_branch) begin     // 偏移量为正（向前跳）
            predict_taken = 1'b0;
        end else begin                    // 不是分支指令
            predict_taken = 1'b0;
        end
    end
    
    // 预测目标地址
    assign predict_target = pc_IF + 4 + branch_offset;
    
endmodule