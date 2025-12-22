module SingleCycleMIPS_CPU(
    input clk,    // 时钟
    input rst     // 复位
);

    // ========== 信号定义 ==========
    // PC相关信号
    wire [31:0] PC, nextPC;
    
    // 指令相关信号
    wire [31:0] instr;
    
    // 控制信号
    wire RegWrite, RegDst, ALUSrc, Branch, MemWrite, MemToReg, Jump;
    wire [2:0] ALUControl;
    wire PCSrc;
    
    // 寄存器文件信号
    wire [31:0] RD1, RD2, WD;
    wire [4:0] writeReg;
    
    // ALU信号
    wire [31:0] ALUResult;
    wire Zero;
    
    // 数据存储器信号
    wire [31:0] readData;
    
    // 立即数
    wire [31:0] signImm;
    
    // PC计算相关信号
    wire [31:0] PCPlus4, PCBranch, PCJump;

    // ========== 模块实例化 ==========
    
    // PC模块
    PC pc_unit(
        .clk(clk),
        .reset(rst),
        .nextPC(nextPC),
        .currentPC(PC)
    );
    
    // 指令存储器
    IMem imem_unit(
        .A(PC),
        .RD(instr)
    );
    
    // 控制器
    Controller ctrl_unit(
        .Op(instr[31:26]),
        .Funct(instr[5:0]),
        .Zero(Zero),
        .MemToReg(MemToReg),
        .MemWrite(MemWrite),
        .PCSrc(PCSrc),
        .ALUSrc(ALUSrc),
        .RegDst(RegDst),
        .RegWrite(RegWrite),
        .Jump(Jump),
        .ALUControl(ALUControl)
    );
    
    // 寄存器文件
    RegFile regfile_unit(
        .CLK(clk),
        .WE3(RegWrite),
        .RA1(instr[25:21]),
        .RA2(instr[20:16]),
        .WA3(writeReg),
        .WD(WD),
        .RD1(RD1),
        .RD2(RD2)
    );
    
    // ALU
    ALU alu_unit(
        .A(RD1),
        .B(ALUSrc ? signImm : RD2),
        .OP(ALUControl),
        .F(ALUResult),
        .ZF(Zero)
    );
    
    // 数据存储器
    DMem dmem_unit(
        .CLK(clk),
        .WE(MemWrite),
        .A(ALUResult),
        .WD(RD2),
        .RD(readData)
    );

    // ========== 数据通路连接 ==========
    
    // 符号扩展
    assign signImm = {{16{instr[15]}}, instr[15:0]};
    
    // 写寄存器选择
    assign writeReg = RegDst ? instr[15:11] : instr[20:16];
    
    // 写回数据选择
    assign WD = MemToReg ? readData : ALUResult;
    
    // 下一条PC计算
    assign PCPlus4 = PC + 4;
    assign PCBranch = PCPlus4 + (signImm << 2);
    assign PCJump = {PCPlus4[31:28], instr[25:0], 2'b00};
    assign nextPC = Jump ? PCJump : (PCSrc ? PCBranch : PCPlus4);

endmodule
