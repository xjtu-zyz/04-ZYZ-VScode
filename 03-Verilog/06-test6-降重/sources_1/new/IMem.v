module IMem(
    input [31:0] A,           // 6位地址输入
    output reg [31:0] RD         // 32位数据输出
);

    parameter DATA_WIDTH = 32;    // 数据宽度参数
    parameter IMEM_SIZE = 256;     // 存储器大小

    // 指令存储器
    reg [DATA_WIDTH-1:0] memory [0:IMEM_SIZE-1];

    integer i;
  
    initial begin
        // 初始化所有指令为nop
        for (i = 0; i < 256; i = i + 1)
            memory[i] = 32'h00000000;

        // 测试指令存储器内容
        //001000_00000_10001_0000000000000100
        memory[0]  = 32'h20110004;  // addi $s1, $0, 4
        memory[1]  = 32'h20120007;  // addi $s2, $0, 7
        //000000_10001_10010_10011_00000100000
        memory[2]  = 32'h02329820;  // add $s3, $s1, $s2    // 数据前递
        memory[3]  = 32'h0272a022;  // sub $s4, $s3, $s2    // 数据前递
        memory[4]  = 32'h00000000;  // nop
        
        // Load-Use相关指令
        //100011_10001_10101_0000000000000000
        memory[5]  = 32'h8e350000;  // lw $s5, 0($s1)       // Load
        memory[6]  = 32'h02b5b020;  // add $s6, $s5, $s5    // Use(需要阻塞)
        memory[7]  = 32'h00000000;  // nop
        
        // 分支指令相关
        //000100_10001_10010_0000000000000010
        memory[8]  = 32'h12320002;  // beq $s1, $s2, 2      // 分支不taken
        memory[9]  = 32'h20110002;  // addi $s1, $0, 2
        memory[10] = 32'h20120002; // addi $s2, $0, 2
        memory[11] = 32'h12320001; // beq $s1, $s2, 1      // 分支taken
        memory[12] = 32'h20110014; // addi $s1, $0, 14     // 应该被跳过
        memory[13] = 32'h20110013; // addi $s1, $0, 13     // 可以执行
        memory[14] = 32'h00000000; // nop

        // 跳转指令
        //000010_00000000000000000000010001
        memory[15] = 32'h08000011; // j 17                 // 跳转
        memory[16] = 32'h20110099; // addi $s1, $0, 99  // 应该被跳过
        memory[17] = 32'h00000000; // nop
        
      end

    // 指令输出
    always @(*) begin
       RD = memory[A[9:2]];
    end   

endmodule