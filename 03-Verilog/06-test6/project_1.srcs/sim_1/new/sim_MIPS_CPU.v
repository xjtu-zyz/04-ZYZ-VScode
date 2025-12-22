`timescale 1ns / 1ps

module sim_MIPS_CPU;
    reg clk;
    reg reset;
    
    MIPS_CPU uut (.clk(clk), .reset(reset));
    
    always #5 clk = ~clk;
    
    // 监视信号
    wire [31:0] PC = uut.PC;
    wire [31:0] instr = uut.instr;
    wire [31:0] RD1 = uut.RD1;
    wire [31:0] RD2 = uut.RD2;
    wire [31:0] ALUResult = uut.ALUResult;
    wire [31:0] WD = uut.WD;
    wire RegWrite = uut.RegWrite;
    wire [4:0] writeReg = uut.writeReg;
    
    integer i;
    
    initial begin
        clk = 0;
        reset = 1;
        
        initialize_instruction_memory();
     
        #10 reset = 0;
        #300;
        
    end
    
    
    task initialize_instruction_memory;
    begin
        // 清除所有指令
        for (i = 0; i < 1024; i = i + 1)
            uut.im_unit.memory[i] = 32'h00000000;
        
        // 新的测试程序序列 - 与原代码不同
        // 测试 addi 指令: addi $1, $0, 10
        uut.im_unit.memory[0] = 32'h2001000A;
        
        // 测试 addi 指令: addi $2, $0, 20  
        uut.im_unit.memory[1] = 32'h20020014;
        
        // 测试 addu 指令: addu $3, $1, $2
        uut.im_unit.memory[2] = 32'h00221821;
        
        // 测试 subu 指令: subu $4, $3, $1
        uut.im_unit.memory[3] = 32'h00612023;
        
        // 测试 ori 指令: ori $5, $1, 0x00FF
        uut.im_unit.memory[4] = 32'h342500FF;
        
        // 测试 lui 指令: lui $6, 0xABCD
        uut.im_unit.memory[5] = 32'h3C06ABCD;
        
        // 测试 sw 指令: sw $3, 8($0)
        uut.im_unit.memory[6] = 32'hAC030008;
        
        // 测试 lw 指令: lw $7, 8($0)
        uut.im_unit.memory[7] = 32'h8C070008;
        
        // 测试 beq 指令: beq $3, $7, 2 (向前跳转2条指令)
        uut.im_unit.memory[8] = 32'h10670002;
        
        // 这条指令应该不会执行（如果beq跳转成功）
        uut.im_unit.memory[9] = 32'h20080001; // addi $8, $0, 1
        
        // 这条指令应该不会执行（如果beq跳转成功）
        uut.im_unit.memory[10] = 32'h20090002; // addi $9, $0, 2
        
        // beq跳转目标地址
        uut.im_unit.memory[11] = 32'h200A0003; // addi $10, $0, 3
        
        // 测试 slt 指令: slt $11, $1, $2 ($1=10 < $2=20, 所以$11=1)
        uut.im_unit.memory[12] = 32'h0022582A;
        
        // 测试 j 指令: j 15 (跳转到地址15)
        uut.im_unit.memory[13] = 32'h0800000F;
        
        // 这条指令应该不会执行（jump跳过后）
        uut.im_unit.memory[14] = 32'h200C0004; // addi $12, $0, 4
        
        // jump目标地址
        uut.im_unit.memory[15] = 32'h200D0005; // addi $13, $0, 5
        
        // 结束程序
        uut.im_unit.memory[16] = 32'h00000000; // nop
    end
    endtask
  

endmodule