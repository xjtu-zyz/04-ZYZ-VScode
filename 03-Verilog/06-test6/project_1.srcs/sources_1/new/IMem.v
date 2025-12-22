
module IMem(
    input [31:0] A,           // 6位地址输入
    output reg [31:0] RD         // 32位数据输出
);

    parameter DATA_WIDTH = 32;    // 数据宽度参数
    parameter IMEM_SIZE = 1024;     // 存储器大小

    // 指令存储器
    reg [DATA_WIDTH-1:0] memory [0:IMEM_SIZE-1];

    integer i;
  
    initial begin
        // 初始化所有指令为nop
        for (i = 0; i < 1024; i = i + 1)
            memory[i] = 32'h00000000;
    end

    // 指令输出
    always @(*) begin
       RD = memory[A[11:2]];
    end   

endmodule 