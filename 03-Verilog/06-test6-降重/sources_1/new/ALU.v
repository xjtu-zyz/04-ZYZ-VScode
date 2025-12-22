module ALU(
    input [31:0] A,          // 操作数A
    input [31:0] B,          // 操作数B
    input [2:0] ALUControl,  // 3位运算控制信号
    output reg [31:0] Result,     // 运算结果
    output Zero              // 零标志位：结果为0时置1
);

// 核心运算逻辑（替代译码器+子模块调用）
always @(*) begin
    case(ALUControl)
        3'b000:  Result = A & B;                  // 按位与
        3'b001:   Result = A | B;                  // 按位或
        3'b010:  Result = A + B;                  // 加法（全加器实现）
        3'b110:  Result = A - B;                  // 减法（补码+全加器实现）
        3'b111:  Result = ($signed(A) < $signed(B)) ? 32'd1 : 32'd0; // 有符号比较
        default:  Result = 32'd0;                  // 默认结果为0（避免综合警告）
    endcase
end

// 零标志位：结果全0时置1
assign Zero = (Result == 32'h00000000);

endmodule