`timescale 1ns / 1ps

module sim_MultiCPU;
    reg clk;
    reg reset;
    
    MultiCycleCPU uut (.clk(clk), .reset(reset));
    
    always #5 clk = ~clk;
    
    // 监视信号 - 多周期特有信号
    wire [31:0] PC = uut.PC_out;
    wire [31:0] Instr = uut.Instruction;
    wire [31:0] IR = uut.IR;                    // 指令寄存器
    wire [2:0] state = uut.state;               // 当前状态
    wire [2:0] next_state = uut.next_state;     // 下一状态
    wire [31:0] RD1 = uut.ReadData1;
    wire [31:0] RD2 = uut.ReadData2;
    wire [31:0] ALUResult = uut.ALUResult;
  //  wire [31:0] ALUOut = uut.ALUOut;            // ALU输出寄存器
 //   wire [31:0] A1 = uut.A;                      // 寄存器A值
 //   wire [31:0] A2 = uut.B;                      // 寄存器B值
//    wire [31:0] MDR = uut.MDR;                  // 存储器数据寄存器
    wire [31:0] WD = uut.WriteData;
    wire RegWrite = uut.RegWrite;
    wire [4:0] WriteReg = uut.WriteReg;
 //   wire MemWrite = uut.MemWrite;
//    wire Zero = uut.Zero;
    
    integer i;
    
    initial begin
        clk = 0;
        reset = 1;
        
        initialize_instruction_memory();
        initialize_data_memory();
     
        #10 reset = 0;
        #500;  // 多周期需要更长的仿真时间
        
        
    end
    
    task initialize_instruction_memory;
    begin
        // 清除所有指令
        for (i = 0; i < 1024; i = i + 1)
            uut.IM.memory[i] = 32'h00000000;
        
        // 测试程序序列 - 专门设计用于观察多周期特性
        // 地址 0: addi $1, $0, 10     (R1 = 10)
        uut.IM.memory[0] = 32'h2001000A;
        
        // 地址 1: addi $2, $0, 20     (R2 = 20) 
        uut.IM.memory[1] = 32'h20020014;
        
        // 地址 2: add $3, $1, $2      (R3 = R1 + R2 = 30) - R-type指令
        uut.IM.memory[2] = 32'h00221820;
        
        // 地址 3: sw $3, 16($0)       (Mem[16] = R3) - Store指令
        uut.IM.memory[3] = 32'hAC030010;
        
        // 地址 4: lw $4, 16($0)       (R4 = Mem[16]) - Load指令
        uut.IM.memory[4] = 32'h8C040010;
        
        // 地址 5: beq $3, $4, 2       (如果R3==R4则跳转2条指令) - Branch指令
        uut.IM.memory[5] = 32'h10640002;
        
        // 地址 6: addi $5, $0, 1      (这条指令在beq成功时被跳过)
        uut.IM.memory[6] = 32'h20050001;
        
        // 地址 7: addi $6, $0, 2      (这条指令在beq成功时被跳过)
        uut.IM.memory[7] = 32'h20060002;
        
        // 地址 8: addi $7, $0, 3      (beq跳转目标)
        uut.IM.memory[8] = 32'h20070003;
        
        // 地址 9: sub $8, $3, $1      (R8 = R3 - R1 = 20) - R-type指令
        uut.IM.memory[9] = 32'h00614022;
        
        // 地址 10: j 13                (无条件跳转) - Jump指令
        uut.IM.memory[10] = 32'h0800000D;
        
        // 地址 11: addi $9, $0, 4     (这条指令被jump跳过)
        uut.IM.memory[11] = 32'h20090004;
        
        // 地址 12: addi $10, $0, 5    (这条指令被jump跳过)
        uut.IM.memory[12] = 32'h200A0005;
        
        // 地址 13: addi $11, $0, 6    (jump跳转目标)
        uut.IM.memory[13] = 32'h200B0006;
        
        // 地址 14: and $12, $1, $2    (R12 = R1 & R2) - R-type指令
        uut.IM.memory[14] = 32'h00226024;
        
        // 地址 15: or $13, $1, $2     (R13 = R1 | R2) - R-type指令
        uut.IM.memory[15] = 32'h00226825;
        
        // 地址 16: slt $14, $1, $2    (R14 = (R1 < R2) ? 1 : 0) - R-type指令
        uut.IM.memory[16] = 32'h0022702A;
        
        // 循环回到开始
        uut.IM.memory[17] = 32'h08000000;
    end
    endtask
    
    task initialize_data_memory;
    begin
        // 初始化数据存储器
        for (i = 0; i < 1024; i = i + 1)
            uut.DM.memory[i] = 32'h00000000;
            
        // 预置一些测试数据
        uut.DM.memory[0] = 32'h00000001;
        uut.DM.memory[1] = 32'h00000002;
        uut.DM.memory[2] = 32'h00000003;
        uut.DM.memory[3] = 32'h00000004;
    end
    endtask

endmodule