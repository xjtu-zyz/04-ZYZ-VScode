/*
module HazardDetectionUnit(
    input ID_EX_MemRead,
    input [4:0] ID_EX_rt,
    input [4:0] IF_ID_rs,
    input [4:0] IF_ID_rt,
    output reg Stall,
    output reg [1:0] forward_mem
);
    always @(*) begin
        Stall = 1'b0;
        forward_mem = 2'b00;
        
        // Load-use冒险检测
        // 如果EX阶段是load指令，且其目标寄存器是ID阶段需要的源寄存器
        if (ID_EX_MemRead && 
           ((ID_EX_rt == IF_ID_rs) || (ID_EX_rt == IF_ID_rt))) begin
            Stall = 1'b1; // 暂停流水线
        end
        
        // Store指令的数据冒险处理
        // 检测是否需要为store指令前推数据
        if (ID_EX_MemRead && (ID_EX_rt != 0)) begin
            // 这里简化为总是使用前推
            forward_mem = 2'b10;
        end
    end
endmodule
*/
module HazardDetectionUnit(
    input wire ID_EX_MemRead,  // ID/EX：是否为读内存指令（lw）
    input wire [4:0] ID_EX_rt, // ID/EX：lw指令的目标寄存器（rt）
    input wire [4:0] IF_ID_rs, // IF/ID：当前指令的源寄存器1（rs）
    input wire [4:0] IF_ID_rt, // IF/ID：当前指令的源寄存器2（rt）
    output reg Stall           // 暂停信号（Load-use冒险）
);

always @(*) begin
    // Load-use冒险：上一条是lw（ID_EX_MemRead=1），当前指令使用lw的目标寄存器（rt）作为rs或rt
    Stall = ID_EX_MemRead && ((ID_EX_rt == IF_ID_rs) || (ID_EX_rt == IF_ID_rt));
end

endmodule