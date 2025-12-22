`timescale 1ns / 1ps
module IM(
    input wire [31:0] Address,
    output wire [31:0] Instruction
);

reg [31:0] mem [0:255];
integer i;
initial begin
    
    for (i = 0; i < 256; i = i + 1)
        mem[i] = 32'h00000000;
    // 数据冒险
    //001000_00000_10001_0000000000000101
    mem[0] = 32'h20110005;  // addi $s1, $zero, 5
    mem[1] = 32'h20120003;  // addi $s2, $zero, 3
    //000000_10001_10010_10011_00000100000
    mem[2] = 32'h02329820;  // add $s3, $s1, $s2    // 数据前递
    mem[3] = 32'h0272a022;  // sub $s4, $s3, $s2    // 数据前递
    mem[4] = 32'h00000000;  // nop
    
    // Load-Use
    mem[5] = 32'h8e350000;  // lw $s5, 0($s1)       // Load
    mem[6] = 32'h02b5b020;  // add $s6, $s5, $s5    // Use(需要阻塞)
    mem[7] = 32'h00000000;  // nop
    
    // 分支指令
    mem[8] = 32'h12320002;  // beq $s1, $s2, 2      // 分支不taken
    mem[9] = 32'h20110001;  // addi $s1, $zero, 1
    mem[10] = 32'h20120001; // addi $s2, $zero, 1
    mem[11] = 32'h12320001; // beq $s1, $s2, 1      // 分支taken
    mem[12] = 32'h20110099; // addi $s1, $zero, 99  // 应该被跳过
    mem[13] = 32'h20110098; // 
    
    // 跳转指令
    mem[14] = 32'h08000010; // j 16                 // 跳转
    mem[15] = 32'h20110099; // addi $s1, $zero, 99  // 应该被跳过
    mem[16] = 32'h00000000; // nop
    
    // 存储器
    mem[17] = 32'h20110064; // addi $s1, $zero, 100
    mem[18] = 32'hae320000; // sw $s2, 0($s1)
    mem[19] = 32'h8e370000; // lw $s7, 0($s1)
    mem[20] = 32'h00000000; // nop
    
    mem[21] = 32'h00000000;
end

assign Instruction = mem[Address[9:2]];

endmodule
