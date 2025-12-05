// ==================== 顶层MIPS模块 ====================
module MIPS_CPU(
    input clk, reset
);
    // 信号定义
    wire [31:0] PC, nextPC, instr;
    wire RegWrite, RegDst, ALUSrc, Branch, MemWrite, MemToReg, Jump;
    wire [2:0] ALUControl;
    wire [31:0] RD1, RD2, WD, signImm, ALUResult, readData;
    wire [4:0] writeReg;
    wire ZF, PCSrc;
    
    // 使用结构描述方式 - PC模块
    PC pc_unit(.clk(clk), .reset(reset), .nextPC(nextPC), .currentPC(PC));
    
    // 使用行为描述方式 - 指令存储器
    IMem im_unit(.A(PC), .RD(instr));
    
    // 使用结构描述方式 - 控制器
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
    
    // 使用数据流描述方式 - 写寄存器选择
    assign writeReg = RegDst ? instr[15:11] : instr[20:16];
    
    // 使用行为描述方式 - 寄存器文件
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
    
    // 使用数据流描述方式 - 符号扩展
    assign signImm = {{16{instr[15]}}, instr[15:0]};
    
    // 使用数据流描述方式 - ALU输入选择
    wire [31:0] ALUSrcB = ALUSrc ? signImm : RD2;
    
    // 使用行为描述方式 - ALU
    ALU alu_unit(
        .A(RD1),
        .B(ALUSrcB),
        .OP(ALUControl),
        .F(ALUResult),
        .ZF(ZF)
    );
    
    // 使用行为描述方式 - 数据存储器
    DMem dm_unit(
        .CLK(clk),
        .WE(MemWrite),
        .A(ALUResult),
        .WD(RD2),
        .RD(readData)
    );
    
    // 使用数据流描述方式 - 写回数据选择
    assign WD = MemToReg ? readData : ALUResult;
    
    // 使用数据流描述方式 - 下一条PC计算
    wire [31:0] PCPlus4 = PC + 4;
    wire [31:0] PCBranch = PCPlus4 + (signImm << 2);
    wire [31:0] PCJump = {PCPlus4[31:28], instr[25:0], 2'b00};
    assign nextPC = Jump ? PCJump : (PCSrc ? PCBranch : PCPlus4);

endmodule
