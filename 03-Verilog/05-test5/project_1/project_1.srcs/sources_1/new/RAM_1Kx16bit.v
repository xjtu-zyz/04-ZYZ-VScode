module RAM_1Kx16(
    output reg [15:0] Data_out,    // 数据输出
    input [9:0] Addr,              // 地址数据
    input Rst,                     // 复位信号
    input RE,                      // 读使能信号
    input WE,                      // 写使能信号
    input CS,                      // 使能/片选信号
    input CLK,                     // 时钟信号
    input [15:0] Data_in           // 数据输入
);

    // 参数定义
    parameter Addr_Width = 10;     // 参数化地址线宽
    parameter Data_Width = 16;     // 参数化数据线宽
    parameter SIZE = 2 ** Addr_Width; // 参数化大小1024
    
    // 存储器声明
    reg [Data_Width-1:0] RAM [SIZE-1:0];
    
    // 临时变量
    integer i;
    
/*当 Addr 和 Data_in 在同一时刻变化时（如测试平台中 #10 后同时更新），always @(*) 会被触发两次，可能导致数据写入到错误的地址，或写入不确定的数据。*/
    // 异步操作 - 组合逻辑
   // always @(*) begin
always @(posedge CLK) begin	//时钟同步
        casex({CS, Rst, RE, WE})
            4'bx1xx: begin // 复位
                for(i = 0; i <= SIZE-1; i = i+1) 
                    RAM[i] = 0;
                Data_out = 16'bz;
            end
            4'b1010: begin // 读数据
                Data_out <= RAM[Addr];
            end
            4'b1001: begin // 写数据
                // 在异步版本中，写操作需要时钟同步
                // 这里保持RAM不变，实际写操作在时钟边沿处理
                RAM[Addr] <= Data_in;
            end
            default: begin
                Data_out = 16'bz;
            end
        endcase
    end

endmodule
