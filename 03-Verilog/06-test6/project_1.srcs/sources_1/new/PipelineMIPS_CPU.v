module PipelineMIPS_CPU(
    input wire clk,
    input wire rst
);
    
    // ========== 流水线寄存器 ==========
    
    // IF/ID寄存器
    reg [31:0] IF_ID_PCPlus4;
    reg [31:0] IF_ID_Instr;
    
    // ID/EX寄存器
    reg [31:0] ID_EX_PCPlus4;
    reg [31:0] ID_EX_ReadData1;
    reg [31:0] ID_EX_ReadData2;
    reg [31:0] ID_EX_SignExtImm;
    reg [4:0]  ID_EX_rs;
    reg [4:0]  ID_EX_rt;
    reg [4:0]  ID_EX_rd;
    reg [5:0]  ID_EX_Funct;
    reg        ID_EX_RegWrite;
    reg        ID_EX_MemToReg;
    reg        ID_EX_Branch;
    reg        ID_EX_MemRead;
    reg        ID_EX_MemWrite;
    reg        ID_EX_RegDst;
    reg        ID_EX_ALUSrc;
    reg [2:0]  ID_EX_ALUControl;
    reg        ID_EX_Jump;
    
    // EX/MEM寄存器
    reg [31:0] EX_MEM_ALUResult;
    reg [31:0] EX_MEM_WriteData;
    reg [4:0]  EX_MEM_WriteReg;
    reg        EX_MEM_RegWrite;
    reg        EX_MEM_MemToReg;
    reg        EX_MEM_MemRead;
    reg        EX_MEM_MemWrite;
    reg        EX_MEM_Branch;
    reg        EX_MEM_Zero;
    reg [31:0] EX_MEM_BranchTarget;
    reg        EX_MEM_Jump;
    reg [31:0] EX_MEM_JumpTarget;
    
    // MEM/WB寄存器
    reg [31:0] MEM_WB_ReadData;
    reg [31:0] MEM_WB_ALUResult;
    reg [4:0]  MEM_WB_WriteReg;
    reg        MEM_WB_RegWrite;
    reg        MEM_WB_MemToReg;
    
    // ========== 模块实例化 ==========
    
    // PC寄存器
    reg [31:0] PC;
    wire [31:0] nextPC;
    
    // 指令存储器
    wire [31:0] instr;
    IMem imem(.A(PC), .RD(instr));
    
    // 寄存器文件
    wire [31:0] RD1, RD2;
    wire [31:0] WD;
    RegFile regfile(
        .CLK(clk),
        .WE3(MEM_WB_RegWrite),
        .RA1(IF_ID_Instr[25:21]),
        .RA2(IF_ID_Instr[20:16]),
        .WA3(MEM_WB_WriteReg),
        .WD(WD),
        .RD1(RD1),
        .RD2(RD2)
    );
    
    // 控制器
    wire RegWrite, RegDst, ALUSrc, Branch, MemRead, MemWrite, MemToReg, Jump;
    wire [2:0] ALUControl;
    wire PCSrc;
    
    Controller ctrl(
        .Op(IF_ID_Instr[31:26]),
        .Funct(IF_ID_Instr[5:0]),
        .Zero(EX_MEM_Zero),
        .MemToReg(MemToReg),
        .MemWrite(MemWrite),
        .PCSrc(PCSrc),
        .ALUSrc(ALUSrc),
        .RegDst(RegDst),
        .RegWrite(RegWrite),
        .Jump(Jump),
        .ALUControl(ALUControl)
    );
    
    // ALU
    wire [31:0] ALUResult;
    wire Zero;
    wire [31:0] ALUInputB;
    wire [31:0] forwardA, forwardB;
    
    ALU alu(
        .A(forwardA),
        .B(ALUInputB),
        .OP(ID_EX_ALUControl),
        .F(ALUResult),
        .ZF(Zero)
    );
    
    // 数据存储器
    wire [31:0] readData;
    DMem dmem(
        .CLK(clk),
        .WE(EX_MEM_MemWrite),
        .A(EX_MEM_ALUResult),
        .WD(EX_MEM_WriteData),
        .RD(readData)
    );
    
    // 写回数据选择
    assign WD = MEM_WB_MemToReg ? MEM_WB_ReadData : MEM_WB_ALUResult;
    
    // 符号扩展
    wire [31:0] SignExtImm;
    assign SignExtImm = {{16{IF_ID_Instr[15]}}, IF_ID_Instr[15:0]};
    
    // 分支目标计算
    wire [31:0] BranchTarget = ID_EX_PCPlus4 + (ID_EX_SignExtImm << 2);
    
    // 下一条PC计算
    wire [31:0] PCPlus4 = PC + 4;
    wire [31:0] PCJump = {PCPlus4[31:28], IF_ID_Instr[25:0], 2'b00};
    
    // 冒险处理单元
    wire Stall, Flush;
    wire [1:0] forwardA_ctrl, forwardB_ctrl;

    // 前推单元
    ForwardingUnit Forwarding_unit(
        .ID_EX_rs(ID_EX_rs),
        .ID_EX_rt(ID_EX_rt),
        .EX_MEM_RegWrite(EX_MEM_RegWrite),
        .EX_MEM_WriteReg(EX_MEM_WriteReg),
        .MEM_WB_RegWrite(MEM_WB_RegWrite),
        .MEM_WB_WriteReg(MEM_WB_WriteReg),
        .forwardA(forwardA_ctrl),
        .forwardB(forwardB_ctrl)
    );
    
    // 冒险检测单元
    HazardDetectionUnit hazard_unit(
        .ID_EX_MemRead(ID_EX_MemRead),
        .ID_EX_rt(ID_EX_rt),
        .IF_ID_rs(IF_ID_Instr[25:21]),
        .IF_ID_rt(IF_ID_Instr[20:16]),
        .Stall(Stall)
    );
    
    // 分支预测器
    wire PredictTaken;
    wire [31:0] PredictTarget;
    
    BranchPredictor branch_predictor(
        .pc_IF(PC),
        .instr_IF(instr),
        .predict_taken(PredictTaken),
        .predict_target(PredictTarget)
    );
    
    // 控制冒险处理
    ControlHazardUnit control_hazard(
        .Branch(EX_MEM_Branch),
        .Zero(EX_MEM_Zero),
        .Jump(EX_MEM_Jump),
        .PredictTaken(PredictTaken),
        .Flush(Flush),
        .PCSrc(PCSrc)
    );
    
    // ALU输入选择
    assign ALUInputB = ID_EX_ALUSrc ? ID_EX_SignExtImm : forwardB;
    
    // 前推数据选择
    ExecuteStage ex_forward(
        .alu_result(EX_MEM_ALUResult),
        .rs2_data(ID_EX_ReadData2),
        .wb_data(WD),
        .forward_ex(forwardB_ctrl),
        .alu_in2(forwardB)
    );
    
    assign forwardA = (forwardA_ctrl == 2'b10) ? EX_MEM_ALUResult :
                     (forwardA_ctrl == 2'b01) ? WD : ID_EX_ReadData1;
    
    // 访存阶段数据冒险处理
    wire [31:0] mem_data;
    
    MemoryStage mem_forward(
        .alu_result(EX_MEM_ALUResult),
        .rs2_data(EX_MEM_WriteData),
        .wb_data(WD),
        .forward_mem(2'b00), // 示例值
        .mem_data(mem_data)
    );
    
    // 分支预测选择PC
    assign nextPC = Flush ? (PCSrc ? EX_MEM_BranchTarget : EX_MEM_JumpTarget) :
                   PredictTaken ? PredictTarget : PCPlus4;
    
    // ========== 流水线寄存器更新 ==========
    
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            // 重置所有流水线寄存器
            PC <= 32'b0;
            
            IF_ID_PCPlus4 <= 32'b0;
            IF_ID_Instr <= 32'b0;
            
            ID_EX_PCPlus4 <= 32'b0;
            ID_EX_ReadData1 <= 32'b0;
            ID_EX_ReadData2 <= 32'b0;
            ID_EX_SignExtImm <= 32'b0;
            ID_EX_rs <= 5'b0;
            ID_EX_rt <= 5'b0;
            ID_EX_rd <= 5'b0;
            ID_EX_Funct <= 6'b0;
            ID_EX_RegWrite <= 1'b0;
            ID_EX_MemToReg <= 1'b0;
            ID_EX_Branch <= 1'b0;
            ID_EX_MemRead <= 1'b0;
            ID_EX_MemWrite <= 1'b0;
            ID_EX_RegDst <= 1'b0;
            ID_EX_ALUSrc <= 1'b0;
            ID_EX_ALUControl <= 3'b0;
            ID_EX_Jump <= 1'b0;
            
            EX_MEM_ALUResult <= 32'b0;
            EX_MEM_WriteData <= 32'b0;
            EX_MEM_WriteReg <= 5'b0;
            EX_MEM_RegWrite <= 1'b0;
            EX_MEM_MemToReg <= 1'b0;
            EX_MEM_MemRead <= 1'b0;
            EX_MEM_MemWrite <= 1'b0;
            EX_MEM_Branch <= 1'b0;
            EX_MEM_Zero <= 1'b0;
            EX_MEM_BranchTarget <= 32'b0;
            EX_MEM_Jump <= 1'b0;
            EX_MEM_JumpTarget <= 32'b0;
            
            MEM_WB_ReadData <= 32'b0;
            MEM_WB_ALUResult <= 32'b0;
            MEM_WB_WriteReg <= 5'b0;
            MEM_WB_RegWrite <= 1'b0;
            MEM_WB_MemToReg <= 1'b0;
            
        end else if (Stall) begin
            // 冒险暂停：插入气泡
            PC <= PC; // 保持PC不变
            IF_ID_Instr <= IF_ID_Instr; // 保持指令不变
            ID_EX_RegWrite <= 1'b0; // 插入气泡（NOP）
            ID_EX_MemRead <= 1'b0;
            ID_EX_MemWrite <= 1'b0;
            ID_EX_Branch <= 1'b0;
            ID_EX_Jump <= 1'b0;
            
        end else if (Flush) begin
            // 控制冒险：冲刷流水线
            PC <= nextPC;
            IF_ID_Instr <= 32'h00000000; // NOP
            IF_ID_PCPlus4 <= 32'b0;
            
        end else begin
            // ===== IF阶段 =====
            PC <= nextPC;
            
            // ===== IF/ID寄存器 =====
            IF_ID_PCPlus4 <= PCPlus4;
            IF_ID_Instr <= instr;
            
            // ===== ID/EX寄存器 =====
            ID_EX_PCPlus4 <= IF_ID_PCPlus4;
            ID_EX_ReadData1 <= RD1;
            ID_EX_ReadData2 <= RD2;
            ID_EX_SignExtImm <= SignExtImm;
            ID_EX_rs <= IF_ID_Instr[25:21];
            ID_EX_rt <= IF_ID_Instr[20:16];
            ID_EX_rd <= IF_ID_Instr[15:11];
            ID_EX_Funct <= IF_ID_Instr[5:0];
            
            // 控制信号传递
            ID_EX_RegWrite <= RegWrite;
            ID_EX_MemToReg <= MemToReg;
            ID_EX_Branch <= Branch;
            ID_EX_MemRead <= MemRead;
            ID_EX_MemWrite <= MemWrite;
            ID_EX_RegDst <= RegDst;
            ID_EX_ALUSrc <= ALUSrc;
            ID_EX_ALUControl <= ALUControl;
            ID_EX_Jump <= Jump;
            
            // ===== EX/MEM寄存器 =====
            EX_MEM_ALUResult <= ALUResult;
            EX_MEM_WriteData <= forwardB;
            EX_MEM_WriteReg <= ID_EX_RegDst ? ID_EX_rd : ID_EX_rt;
            EX_MEM_RegWrite <= ID_EX_RegWrite;
            EX_MEM_MemToReg <= ID_EX_MemToReg;
            EX_MEM_MemRead <= ID_EX_MemRead;
            EX_MEM_MemWrite <= ID_EX_MemWrite;
            EX_MEM_Branch <= ID_EX_Branch;
            EX_MEM_Zero <= Zero;
            EX_MEM_BranchTarget <= BranchTarget;
            EX_MEM_Jump <= ID_EX_Jump;
            EX_MEM_JumpTarget <= PCJump;
            
            // ===== MEM/WB寄存器 =====
            MEM_WB_ReadData <= readData;
            MEM_WB_ALUResult <= EX_MEM_ALUResult;
            MEM_WB_WriteReg <= EX_MEM_WriteReg;
            MEM_WB_RegWrite <= EX_MEM_RegWrite;
            MEM_WB_MemToReg <= EX_MEM_MemToReg;
        end
    end
    
endmodule