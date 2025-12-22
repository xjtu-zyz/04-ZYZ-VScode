`timescale 1ns / 1ps

module FiveStageCPU(
    input wire clk,
    input wire rst
);


// FI
reg [31:0] PC;
wire [31:0] PC_plus_4;
wire [31:0] Instruction_FI;
reg [31:0] PC_next;
wire FI_Flush;  // 冲刷
wire FI_Stall;  // 阻塞

// FI/ID
reg [31:0] FI_ID_IR; 
reg [31:0] FI_ID_NPC1; 
wire ID_Flush;       
wire ID_Stall;         

// ID
wire [5:0] opcode;
wire [4:0] rs_addr, rt_addr, rd_addr;
wire [15:0] imm16;
wire [25:0] j_addr;
wire [31:0] Rs_data, Rt_data;
wire [31:0] SignImm;
wire RegWrite_ID, MemToReg_ID, MemWrite_ID;
wire ALUSrc_ID, RegDst_ID, Branch_ID, Jump_ID;
wire [3:0] ALUControl_ID;

// ID/EX
reg [31:0] ID_EX_IR;     
reg [31:0] ID_EX_NPC1;   
reg [31:0] ID_EX_Rs;    
reg [31:0] ID_EX_Rt;   
reg [31:0] ID_EX_Imm32;  
reg [4:0] ID_EX_rs_addr;  //前递判断
reg [4:0] ID_EX_rt_addr;  
// 控制信号
reg ID_EX_RegWrite, ID_EX_MemToReg, ID_EX_MemWrite;
reg ID_EX_ALUSrc, ID_EX_RegDst, ID_EX_Branch;
reg [3:0] ID_EX_ALUControl;
wire EX_Flush;  

// EX
wire [31:0] ALU_input_A, ALU_input_B;
wire [31:0] ALU_input_A_forward, ALU_input_B_forward;
wire [31:0] ALU_result;
wire Zero;
wire [31:0] Branch_target;
wire [4:0] Write_reg_addr;
wire [1:0] ForwardA, ForwardB;  // 前递控制信号

// EX/MA 
reg [31:0] EX_MA_IR;      
reg [31:0] EX_MA_Rt;     
reg [31:0] EX_MA_NPC2;     // 分支指令
reg [31:0] EX_MA_NPC3;     // 转移指令
reg [31:0] EX_MA_ALUOut;  
reg EX_MA_Zero;   

reg EX_MA_RegWrite, EX_MA_MemToReg, EX_MA_MemWrite;
reg EX_MA_Branch;
reg [4:0] EX_MA_WriteReg;

// MA
wire [31:0] Mem_read_data;
wire PCSrc;  // 分支是否发生


// MA/WB
reg [31:0] MA_WB_IR;     
reg [31:0] MA_WB_ALUOut;  
reg [31:0] MA_WB_MEMOut;  

reg MA_WB_RegWrite, MA_WB_MemToReg;
reg [4:0] MA_WB_WriteReg;

// WB
wire [31:0] Write_back_data;

// 冒险检测
wire load_use_hazard;

// FI
assign PC_plus_4 = PC + 4;

wire BranchTaken;
assign BranchTaken = EX_MA_Branch & EX_MA_Zero;  

wire Jump_taken;
assign Jump_taken = Jump_ID;    


assign PCSrc = BranchTaken | Jump_taken;   


always @(*) begin
    if (PCSrc) begin
        // 发生控制转移
        if (BranchTaken) begin
            PC_next = EX_MA_NPC2;  // 分支目标
        end else begin
            PC_next = {FI_ID_NPC1[31:28], j_addr, 2'b00}; // Jump 目标
        end
    end else begin
        // 顺序执行
        PC_next = PC_plus_4;
    end
end

// FI级冲刷和阻塞
assign FI_Flush = PCSrc;

assign FI_Stall = load_use_hazard;     // Load-Use冒险时阻塞

// 指令存储器
IM IM1(
    .Address(PC),
    .Instruction(Instruction_FI)
);

// PC更新
always @(posedge clk or posedge rst) begin
    if (rst)
        PC <= 32'h00000000;
    else if (!FI_Stall)  // 不阻塞时才更新
        PC <= PC_next;
end

// FI/ID 
assign ID_Flush = PCSrc;
assign ID_Stall = load_use_hazard;

always @(posedge clk or posedge rst) begin
    if (rst || ID_Flush) begin
        FI_ID_IR <= 32'h00000000;  // 插入气泡(nop)
        FI_ID_NPC1 <= 32'h00000000;
    end else if (!ID_Stall) begin//阻塞时保持原值
        FI_ID_IR <= Instruction_FI;
        FI_ID_NPC1 <= PC_plus_4;
    end
    // Stall时保持原值
end

// ID
assign opcode = FI_ID_IR[31:26];
assign rs_addr = FI_ID_IR[25:21];
assign rt_addr = FI_ID_IR[20:16];
assign rd_addr = FI_ID_IR[15:11];
assign imm16 = FI_ID_IR[15:0];
assign j_addr = FI_ID_IR[25:0];

assign SignImm = {{16{imm16[15]}}, imm16};

// 寄存器文件
RegisterFile RF(
    .clk(clk),
    .rst(rst),
    .RegWrite(MA_WB_RegWrite),
    .ReadReg1(rs_addr),
    .ReadReg2(rt_addr),
    .WriteReg(MA_WB_WriteReg),
    .WriteData(Write_back_data),
    .ReadData1(Rs_data),
    .ReadData2(Rt_data)
);

ControlUnit CU(
    .opcode(opcode),
    .funct(FI_ID_IR[5:0]),
    .RegWrite(RegWrite_ID),
    .MemToReg(MemToReg_ID),
    .MemWrite(MemWrite_ID),
    .ALUSrc(ALUSrc_ID),
    .RegDst(RegDst_ID),
    .Branch(Branch_ID),
    .Jump(Jump_ID),
    .ALUControl(ALUControl_ID)
);

// 冒险检测
HazardDeUnit HDU(
    .ID_EX_MemToReg(ID_EX_MemToReg),
    .ID_EX_rt(ID_EX_IR[20:16]),
    .FI_ID_rs(rs_addr),
    .FI_ID_rt(rt_addr),
    .load_use_hazard(load_use_hazard)
);

// ID/EX
assign EX_Flush = load_use_hazard | BranchTaken;

always @(posedge clk or posedge rst) begin
    if (rst || EX_Flush) begin
        // 插入气泡
        ID_EX_IR <= 32'h00000000;
        ID_EX_NPC1 <= 32'h00000000;
        ID_EX_Rs <= 32'h00000000;
        ID_EX_Rt <= 32'h00000000;
        ID_EX_Imm32 <= 32'h00000000;
        ID_EX_rs_addr <= 5'b00000;
        ID_EX_rt_addr <= 5'b00000;
        ID_EX_RegWrite <= 1'b0;
        ID_EX_MemToReg <= 1'b0;
        ID_EX_MemWrite <= 1'b0;
        ID_EX_ALUSrc <= 1'b0;
        ID_EX_RegDst <= 1'b0;
        ID_EX_Branch <= 1'b0;
        ID_EX_ALUControl <= 4'b0000;
    end else begin
        ID_EX_IR <= FI_ID_IR;
        ID_EX_NPC1 <= FI_ID_NPC1;
        ID_EX_Rs <= Rs_data;
        ID_EX_Rt <= Rt_data;
        ID_EX_Imm32 <= SignImm;
        ID_EX_rs_addr <= rs_addr;
        ID_EX_rt_addr <= rt_addr;
        ID_EX_RegWrite <= RegWrite_ID;
        ID_EX_MemToReg <= MemToReg_ID;
        ID_EX_MemWrite <= MemWrite_ID;
        ID_EX_ALUSrc <= ALUSrc_ID;
        ID_EX_RegDst <= RegDst_ID;
        ID_EX_Branch <= Branch_ID;
        ID_EX_ALUControl <= ALUControl_ID;
    end
end

// EX

// 前递单元 - 解决数据冒险
ForwardingUnit FU(
    .ID_EX_rs(ID_EX_rs_addr),
    .ID_EX_rt(ID_EX_rt_addr),
    .EX_MA_RegWrite(EX_MA_RegWrite),
    .EX_MA_WriteReg(EX_MA_WriteReg),
    .MA_WB_RegWrite(MA_WB_RegWrite),
    .MA_WB_WriteReg(MA_WB_WriteReg),
    .ForwardA(ForwardA),
    .ForwardB(ForwardB)
);

// ALU输入A的前递选择
assign ALU_input_A_forward = (ForwardA == 2'b10) ? EX_MA_ALUOut :     // EX前递
                              (ForwardA == 2'b01) ? Write_back_data :  // MEM前递
                              ID_EX_Rs;                                // 无前递

// ALU输入B的前递选择(考虑立即数)
wire [31:0] Rt_forward;
assign Rt_forward = (ForwardB == 2'b10) ? EX_MA_ALUOut :     // EX前递
                    (ForwardB == 2'b01) ? Write_back_data :  // MEM前递
                    ID_EX_Rt;                                // 无前递

assign ALU_input_A = ALU_input_A_forward;
assign ALU_input_B = ID_EX_ALUSrc ? ID_EX_Imm32 : Rt_forward;

// 写回寄存器地址选择
assign Write_reg_addr = ID_EX_RegDst ? ID_EX_IR[15:11] : ID_EX_IR[20:16];

assign Branch_target = ID_EX_NPC1 + (ID_EX_Imm32 << 2);

ALU alu(
    .A(ALU_input_A),
    .B(ALU_input_B),
    .ALUControl(ID_EX_ALUControl),
    .Result(ALU_result),
    .Zero(Zero)
);

// EX/MA
always @(posedge clk or posedge rst) begin
    if (rst || BranchTaken) begin
        EX_MA_IR <= 32'h00000000;
        EX_MA_Rt <= 32'h00000000;
        EX_MA_NPC2 <= 32'h00000000;
        EX_MA_NPC3 <= 32'h00000000;
        EX_MA_ALUOut <= 32'h00000000;
        EX_MA_Zero <= 1'b0;
        EX_MA_RegWrite <= 1'b0;
        EX_MA_MemToReg <= 1'b0;
        EX_MA_MemWrite <= 1'b0;
        EX_MA_Branch <= 1'b0;
        EX_MA_WriteReg <= 5'b00000;
    end else begin
        EX_MA_IR <= ID_EX_IR;
        EX_MA_Rt <= Rt_forward;  // 使用前递后的值
        EX_MA_NPC2 <= Branch_target;
        EX_MA_NPC3 <= {ID_EX_NPC1[31:28], ID_EX_IR[25:0], 2'b00};
        EX_MA_ALUOut <= ALU_result;
        EX_MA_Zero <= Zero;
        EX_MA_RegWrite <= ID_EX_RegWrite;
        EX_MA_MemToReg <= ID_EX_MemToReg;
        EX_MA_MemWrite <= ID_EX_MemWrite;
        EX_MA_Branch <= ID_EX_Branch;
        EX_MA_WriteReg <= Write_reg_addr;
    end
end


// MA
DataMemory DM(
    .clk(clk),
    .MemWrite(EX_MA_MemWrite),
    .Address(EX_MA_ALUOut),
    .WriteData(EX_MA_Rt),
    .ReadData(Mem_read_data)
);

// MA/WB
always @(posedge clk or posedge rst) begin
    if (rst) begin
        MA_WB_IR <= 32'h00000000;
        MA_WB_ALUOut <= 32'h00000000;
        MA_WB_MEMOut <= 32'h00000000;
        MA_WB_RegWrite <= 1'b0;
        MA_WB_MemToReg <= 1'b0;
        MA_WB_WriteReg <= 5'b00000;
    end else begin
        MA_WB_IR <= EX_MA_IR;
        MA_WB_ALUOut <= EX_MA_ALUOut;
        MA_WB_MEMOut <= Mem_read_data;
        MA_WB_RegWrite <= EX_MA_RegWrite;
        MA_WB_MemToReg <= EX_MA_MemToReg;
        MA_WB_WriteReg <= EX_MA_WriteReg;
    end
end

// WB
assign Write_back_data = MA_WB_MemToReg ? MA_WB_MEMOut : MA_WB_ALUOut;

endmodule