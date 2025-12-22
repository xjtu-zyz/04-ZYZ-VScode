`timescale 1ns / 1ps

module sim_PipelineMIPS_CPU;
    reg clk;
    reg rst;
    
    // 实例化五级流水线CPU
    PipelineMIPS_CPU uut (.clk(clk), .rst(rst));
    
    // 时钟生成 - 50MHz
    always #10 clk = ~clk;
    
    // 监视关键信号 - 五级流水线特有信号
    wire [31:0] PC = uut.PC;


    wire [31:0] WD = uut.WD;
    

    // 控制信号
    wire PCSrc = uut.PCSrc;
    wire [31:0] IF_ID_Instr = uut.IF_ID_Instr;
    wire [31:0] ID_EX_ReadData1 = uut.ID_EX_ReadData1;
    wire [31:0] ID_EX_ReadData2 = uut.ID_EX_ReadData2;
    wire ID_EX_RegWrite = uut.ID_EX_RegWrite;
    wire ID_EX_MemRead = uut.ID_EX_MemRead;
    wire ID_EX_MemWrite = uut.ID_EX_MemWrite;
    wire ID_EX_Branch = uut.ID_EX_Branch;
    
    wire EX_MEM_RegWrite = uut.EX_MEM_RegWrite;
    wire EX_MEM_MemRead = uut.EX_MEM_MemRead;
    wire EX_MEM_MemWrite = uut.EX_MEM_MemWrite;
    wire EX_MEM_Branch = uut.EX_MEM_Branch;
    wire EX_MEM_Zero = uut.EX_MEM_Zero;
    wire [31:0] EX_MEM_ALUResult = uut.EX_MEM_ALUResult;
    
    wire [31:0] MEM_WB_ALUResult = uut.MEM_WB_ALUResult;
    wire [31:0] MEM_WB_ReadData = uut.MEM_WB_ReadData;
    wire MEM_WB_RegWrite = uut.MEM_WB_RegWrite;
    wire [4:0] MEM_WB_WriteReg = uut.MEM_WB_WriteReg;
   
        // 冒险处理信号
    wire Stall = uut.Stall;
    wire Flush = uut.Flush;
    wire PredictTaken = uut.PredictTaken;
    
    // 前推控制信号
    wire [1:0] forwardA = uut.forwardA_ctrl;
    wire [1:0] forwardB = uut.forwardB_ctrl;
    
    integer i;
    
    initial begin
        clk = 0;
        rst = 1;
        
        initialize_instruction_memory();
        initialize_data_memory();
        
        // 保持复位一段时间
        #20 rst = 0;
        
        // 运行足够长时间观察流水线行为
        #2000;
        
        $finish;
    end
    
    task initialize_instruction_memory;
    begin
        // 清除所有指令
        for (i = 0; i < 1024; i = i + 1)
            uut.imem.memory[i] = 32'h00000000;
        
        // ===== 五级流水线测试程序 =====
        // 这个程序专门设计用于测试流水线冒险处理
        
        // 地址 0x00: addi $1, $0, 10      // R1 = 10
        uut.imem.memory[0] = 32'h2001000A;
        
        // 地址 0x04: addi $2, $0, 20      // R2 = 20
        uut.imem.memory[1] = 32'h20020014;
        
        // 地址 0x08: add $3, $1, $2       // R3 = R1 + R2 = 30 (测试EX前推)
        uut.imem.memory[2] = 32'h00221820;
        
        // 地址 0x0C: add $4, $3, $1       // R4 = R3 + R1 = 40 (测试数据冒险)
        uut.imem.memory[3] = 32'h00612020;
        
        // 地址 0x10: sw $4, 16($0)        // Mem[16] = R4 = 40
        uut.imem.memory[4] = 32'hAC040010;
        
        // 地址 0x14: lw $5, 16($0)        // R5 = Mem[16] = 40 (测试Load指令)
        uut.imem.memory[5] = 32'h8C050010;
        
        // 地址 0x18: add $6, $5, $2       // R6 = R5 + R2 = 60 (测试Load-use冒险)
        uut.imem.memory[6] = 32'h00A23020;
        
        // 地址 0x1C: sub $7, $6, $3       // R7 = R6 - R3 = 30 (测试多级前推)
        uut.imem.memory[7] = 32'h00C33822;
        
        // 地址 0x20: and $8, $4, $5       // R8 = R4 & R5 = 40
        uut.imem.memory[8] = 32'h00854024;
        
        // 地址 0x24: or $9, $1, $2        // R9 = R1 | R2 = 30
        uut.imem.memory[9] = 32'h00224825;
        
        // 地址 0x28: slt $10, $1, $2      // R10 = (R1 < R2) ? 1 : 0 = 1
        uut.imem.memory[10] = 32'h0022502A;
        
        // ===== 分支和跳转测试 =====
        // 地址 0x2C: beq $1, $1, 2        // 总是跳转（测试向后跳）
        uut.imem.memory[11] = 32'h10210002;
        
        // 地址 0x30: addi $11, $0, 99     // 这条指令应该被跳过
        uut.imem.memory[12] = 32'h200B0063;
        
        // 地址 0x34: addi $12, $0, 99     // 这条指令应该被跳过
        uut.imem.memory[13] = 32'h200C0063;
        
        // 地址 0x38: addi $13, $0, 13     // 分支目标：R13 = 13
        uut.imem.memory[14] = 32'h200D000D;
        
        // 地址 0x3C: beq $1, $2, -4       // 不跳转（测试向前跳）
        uut.imem.memory[15] = 32'h1022FFFC;
        
        // 地址 0x40: addi $14, $0, 14     // R14 = 14 (应该执行)
        uut.imem.memory[16] = 32'h200E000E;
        
        // 地址 0x44: j 0x60               // 跳转指令
        uut.imem.memory[17] = 32'h08000018;
        
        // 地址 0x48: addi $15, $0, 15     // 这条指令应该被跳过
        uut.imem.memory[18] = 32'h200F000F;
        
        // 地址 0x4C: addi $16, $0, 16     // 这条指令应该被跳过
        uut.imem.memory[19] = 32'h20100010;
        
        // 地址 0x50: addi $17, $0, 17     // 这条指令应该被跳过
        uut.imem.memory[20] = 32'h20110011;
        
        // ===== 跳转目标 =====
        // 地址 0x60: addi $18, $0, 18     // 跳转目标：R18 = 18
        uut.imem.memory[24] = 32'h20120012;
        
        // 地址 0x64: addi $19, $18, 5     // R19 = R18 + 5 = 23
        uut.imem.memory[25] = 32'h22530005;
        
        // 地址 0x68: sw $19, 32($0)       // Mem[32] = 23
        uut.imem.memory[26] = 32'hAC130020;
        
        // 地址 0x6C: lw $20, 32($0)       // R20 = 23 (测试Load-use冒险)
        uut.imem.memory[27] = 32'h8C140020;
        
        // 地址 0x70: add $21, $20, $19    // R21 = 23 + 23 = 46
        uut.imem.memory[28] = 32'h0293A820;
        
        // 地址 0x74: nop                   // 空指令，观察流水线
        uut.imem.memory[29] = 32'h00000000;
        
        // 地址 0x78: nop                   // 空指令
        uut.imem.memory[30] = 32'h00000000;
        
        // 地址 0x7C: nop                   // 空指令
        uut.imem.memory[31] = 32'h00000000;
    end
    endtask
    
    task initialize_data_memory;
    begin
        // 初始化数据存储器
        for (i = 0; i < 1024; i = i + 1)
            uut.dmem.memory[i] = 32'h00000000;
            
        // 预置一些测试数据
        uut.dmem.memory[0] = 32'h00000001;
        uut.dmem.memory[1] = 32'h00000002;
        uut.dmem.memory[2] = 32'h00000003;
        uut.dmem.memory[3] = 32'h00000004;
        uut.dmem.memory[4] = 32'h00000005;
        uut.dmem.memory[5] = 32'h00000006;
        uut.dmem.memory[6] = 32'h00000007;
        uut.dmem.memory[7] = 32'h00000008;
    end
    endtask

endmodule