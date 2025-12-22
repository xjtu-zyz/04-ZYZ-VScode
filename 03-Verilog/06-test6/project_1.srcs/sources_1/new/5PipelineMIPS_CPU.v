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
    reg [31:0] PC;
    wire [31:0] nextPC;
    
    wire [31:0] instr;
    IMem imem(.A(PC), .RD(instr));
    
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
    
    wire [31:0] readData;
    DMem dmem(
        .CLK(clk),
        .WE(EX_MEM_MemWrite),
        .A(EX_MEM_ALUResult),
        .WD(EX_MEM_WriteData),
        .RD(readData)
    );
    
    assign WD = MEM_WB_MemToReg ? MEM_WB_ReadData : MEM_WB_ALUResult;
    wire [31:0] SignExtImm = {{16{IF_ID_Instr[15]}}, IF_ID_Instr[15:0]};
    wire [31:0] BranchTarget = ID_EX_PCPlus4 + (ID_EX_SignExtImm << 2);
    wire [31:0] PCPlus4 = PC + 4;
    wire [31:0] PCJump = {PCPlus4[31:28], IF_ID_Instr[25:0], 2'b00};
    
    // 冒险处理单元
    wire Stall, Flush;
    wire [1:0] forwardA_ctrl, forwardB_ctrl;

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
    
    // 使用修正后的冒险检测单元
    HazardDetectionUnit hazard_unit(
        .ID_EX_MemRead(ID_EX_MemRead),
        .ID_EX_rt(ID_EX_rt),
        .IF_ID_rs(IF_ID_Instr[25:21]),
        .IF_ID_rt(IF_ID_Instr[20:16]),
        .Stall(Stall)
    );
    
    BranchPredictor branch_predictor(
        .pc_IF(PC),
        .instr_IF(instr),
        .predict_taken(PredictTaken),
        .predict_target(PredictTarget)
    );
    
    // 使用修正后的控制冒险单元
    ControlHazardUnit control_hazard(
        .Branch(EX_MEM_Branch),
        .Zero(EX_MEM_Zero),
        .Jump(EX_MEM_Jump),
        .PredictTaken(PredictTaken),
        .Flush(Flush),
        .PCSrc(PCSrc)
    );
    
    assign ALUInputB = ID_EX_ALUSrc ? ID_EX_SignExtImm : forwardB;
    assign forwardA = (forwardA_ctrl == 2'b10) ? EX_MEM_ALUResult :
                     (forwardA_ctrl == 2'b01) ? WD : ID_EX_ReadData1;
    
    wire [31:0] mem_data;
    MemoryStage mem_forward(
        .alu_result(EX_MEM_ALUResult),
        .rs2_data(EX_MEM_WriteData),
        .wb_data(WD),
        .forward_mem(2'b00),
        .mem_data(mem_data)
    );
    
    // 修正PC选择逻辑：优先响应跳转/分支目标，再考虑预测和默认递增
    assign nextPC = (Flush) ? 
                    (Jump ? EX_MEM_JumpTarget : (PCSrc ? EX_MEM_BranchTarget : PCPlus4)) :
                    (PredictTaken ? PredictTarget : PCPlus4);
    
    // ========== 流水线寄存器更新（核心修正） ==========
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            // 初始化所有寄存器（保持不变）
            PC <= 32'b0;
            IF_ID_PCPlus4 <= 32'b0;
            IF_ID_Instr <= 32'b0;
            // ... 其他寄存器初始化省略
        end else if (Stall) begin
            // 1. Load-use冒险：暂停流水线
            PC <= PC;  // PC保持不变（不取下一条指令）
            IF_ID_Instr <= IF_ID_Instr;  // IF/ID保持当前指令
            // ID/EX插入气泡（控制信号清零，避免错误执行）
            ID_EX_RegWrite <= 1'b0;
            ID_EX_MemRead <= 1'b0;
            ID_EX_MemWrite <= 1'b0;
            ID_EX_Branch <= 1'b0;
            ID_EX_Jump <= 1'b0;
            // 其他ID/EX字段保持不变（不影响，因控制信号已清零）
        end else if (Flush) begin
            // 2. 控制冒险：冲刷流水线
            PC <= nextPC;  // PC跳转到目标地址
            // 冲刷IF/ID：插入NOP（避免错误指令进入ID阶段）
            IF_ID_Instr <= 32'h00000000;  // NOP指令
            IF_ID_PCPlus4 <= 32'b0;
            // 冲刷ID/EX：插入NOP（控制信号清零）
            ID_EX_RegWrite <= 1'b0;
            ID_EX_MemToReg <= 1'b0;
            ID_EX_Branch <= 1'b0;
            ID_EX_MemRead <= 1'b0;
            ID_EX_MemWrite <= 1'b0;
            ID_EX_RegDst <= 1'b0;
            ID_EX_ALUSrc <= 1'b0;
            ID_EX_ALUControl <= 3'b0;
            ID_EX_Jump <= 1'b0;
            // EX/MEM及以后阶段不受影响（已进入执行阶段，需完成但不写回）
        end else begin
            // 3. 正常流水线流转
            PC <= nextPC;
            
            // IF/ID寄存器更新
            IF_ID_PCPlus4 <= PCPlus4;
            IF_ID_Instr <= instr;  // 取新指令
            
            // ID/EX寄存器更新（传递译码结果和控制信号）
            ID_EX_PCPlus4 <= IF_ID_PCPlus4;
            ID_EX_ReadData1 <= RD1;
            ID_EX_ReadData2 <= RD2;
            ID_EX_SignExtImm <= SignExtImm;
            ID_EX_rs <= IF_ID_Instr[25:21];
            ID_EX_rt <= IF_ID_Instr[20:16];
            ID_EX_rd <= IF_ID_Instr[15:11];
            ID_EX_Funct <= IF_ID_Instr[5:0];
            ID_EX_RegWrite <= RegWrite;
            ID_EX_MemToReg <= MemToReg;
            ID_EX_Branch <= Branch;
            ID_EX_MemRead <= MemRead;
            ID_EX_MemWrite <= MemWrite;
            ID_EX_RegDst <= RegDst;
            ID_EX_ALUSrc <= ALUSrc;
            ID_EX_ALUControl <= ALUControl;
            ID_EX_Jump <= Jump;
            
            // EX/MEM寄存器更新（传递执行结果）
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
            EX_MEM_JumpTarget <= PCJump;  // 传递跳转目标地址
            
            // MEM/WB寄存器更新（传递访存结果）
            MEM_WB_ReadData <= readData;
            MEM_WB_ALUResult <= EX_MEM_ALUResult;
            MEM_WB_WriteReg <= EX_MEM_WriteReg;
            MEM_WB_RegWrite <= EX_MEM_RegWrite;
            MEM_WB_MemToReg <= EX_MEM_MemToReg;
        end
    end
    
endmodule