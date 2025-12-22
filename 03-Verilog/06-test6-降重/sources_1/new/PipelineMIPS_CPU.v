module PipelineMIPS_CPU(
    input CLK,
    input RST
);


// IF
reg [31:0] PC;
wire [31:0] PC_plus4;
wire [31:0] Instruction_IF;
reg [31:0] PC_next;
wire IF_Flush;         // 冲刷
wire IF_Stall;         // 阻塞
// IF/ID
reg [31:0] IF_ID_IR; 
reg [31:0] IF_ID_PC1; //PC1 = PC+ 4
wire ID_Flush;       // 冲刷
wire ID_Stall;       // 阻塞

// ID
wire [5:0] Op;
wire [4:0] rs_addr, rt_addr, rd_addr;
wire [15:0] imm16;
wire [25:0] j_addr;
wire [31:0] Rs_data, Rt_data;
wire [31:0] SignImm;
// 控制单元controller产生的信号量
wire RegWrite, MemToReg, MemWrite;
wire ALUSrc, RegDst, Branch, Jump;
wire [2:0] ALUControl;
// ID/EX
reg [31:0] ID_EX_IR;     
reg [31:0] ID_EX_PC1;   
reg [31:0] ID_EX_Rs;    
reg [31:0] ID_EX_Rt;   
reg [31:0] ID_EX_Imm32;  
reg [4:0] ID_EX_rs_addr;  
reg [4:0] ID_EX_rt_addr;  
// 控制信号
reg ID_EX_RegWrite, ID_EX_MemToReg, ID_EX_MemWrite;
reg ID_EX_ALUSrc, ID_EX_RegDst, ID_EX_Branch;
reg [2:0] ID_EX_ALUControl;
wire EX_Flush;          // 冲刷  

// EX
wire [31:0] ALU_A, ALU_B;
wire [31:0] ALU_A_forward, ALU_B_forward;
wire [31:0] ALU_result;
wire Zero;
wire [31:0] Branch_target;
wire [4:0] Write_reg_addr;
wire [1:0] ForwardA, ForwardB;  // 前递控制信号
// EX/MEM 
reg [31:0] EX_MEM_IR;      
reg [31:0] EX_MEM_Rt;     
reg [31:0] EX_MEM_PC2;      //PC2 = 分支目标地址
reg [31:0] EX_MEM_PC3;      //PC3 = 跳转目标地址
reg [31:0] EX_MEM_ALUOut;  
reg EX_MEM_Zero;   
// 控制信号
reg EX_MEM_RegWrite, EX_MEM_MemToReg, EX_MEM_MemWrite;
reg EX_MEM_Branch;
reg [4:0] EX_MEM_WriteReg;

// MEM
wire [31:0] Memread_data;
wire PCSrc;  // 发生branch或jump的信号
// MEM/WB
reg [31:0] MEM_WB_IR;     
reg [31:0] MEM_WB_ALUOut;  
reg [31:0] MEM_WB_MEMOut;  
reg MEM_WB_RegWrite, MEM_WB_MemToReg;
reg [4:0] MEM_WB_WriteReg;

// WB
wire [31:0] Writeback_data;




// 冒险检测
wire load_use;              // Load-Use冒险检测信号 
wire BranchTaken;           // beq分支信号
assign BranchTaken = EX_MEM_Branch & EX_MEM_Zero;  
wire Jump_taken;            // Jump跳转信号
assign Jump_taken = Jump;    
assign PCSrc = BranchTaken | Jump_taken;   // PC选择信号

// IF阶段：PC计算
assign PC_plus4 = PC + 4;
always @(*) begin
    if (PCSrc) begin
        // 分支或跳转
        if (BranchTaken)            // beq
            PC_next = EX_MEM_PC2;   // beq 目标
        else                                             // Jump
            PC_next = {IF_ID_PC1[31:28], j_addr, 2'b00}; // Jump 目标
    end 
    else PC_next = PC_plus4;         // 顺序执行
end

// IF阶段：冲刷和阻塞
assign IF_Flush = PCSrc;        // 控制冒险时冲刷
assign IF_Stall = load_use;     // Load-Use冒险时阻塞

// IF阶段：PC更新
always @(posedge CLK or posedge RST) begin
    if (RST)
        PC <= 32'h00000000;
    else if (!IF_Stall)  // 不阻塞时才更新
        PC <= PC_next;
end

// IF阶段：指令存储器模块
IMem IM1(
    .A(PC),
    .RD(Instruction_IF)
);



// IF/ID过渡阶段：冲刷和阻塞
assign ID_Flush = PCSrc;
assign ID_Stall = load_use;
// IF/ID：寄存器更新
always @(posedge CLK or posedge RST) begin
    if (RST || ID_Flush) begin
        IF_ID_IR <= 32'h00000000;  // 插入气泡(nop)
        IF_ID_PC1 <= 32'h00000000;
    end 
    else if (!ID_Stall) begin//阻塞时保持原值
        IF_ID_IR <= Instruction_IF;
        IF_ID_PC1 <= PC_plus4;
    end                     
end

// ID阶段：指令译码
assign Op = IF_ID_IR[31:26];
assign rs_addr = IF_ID_IR[25:21];
assign rt_addr = IF_ID_IR[20:16];
assign rd_addr = IF_ID_IR[15:11];
assign imm16 = IF_ID_IR[15:0];
assign j_addr = IF_ID_IR[25:0];
assign SignImm = {{16{imm16[15]}}, imm16};

// ID阶段：寄存器组模块
RegIFle RF(
    .CLK(CLK),
    .RST(RST),
    .RW(MEM_WB_RegWrite),
    .RA1(rs_addr),
    .RA2(rt_addr),
    .WR(MEM_WB_WriteReg),
    .WD(Writeback_data),
    .RD1(Rs_data),
    .RD2(Rt_data)
);
// ID阶段：控制单元模块
Controller Con(
    .Op(Op),
    .Funct(IF_ID_IR[5:0]),
    .RegWrite(RegWrite),
    .MemToReg(MemToReg),
    .MemWrite(MemWrite),
    .ALUSrc(ALUSrc),
    .RegDst(RegDst),
    .Branch(Branch),
    .Jump(Jump),
    .ALUControl(ALUControl)
);

// ID阶段：控制冒险处理模块
HazardHandleUnit HDU(
    .ID_EX_MemToReg(ID_EX_MemToReg),
    .ID_EX_rt(ID_EX_IR[20:16]),
    .IF_ID_rs(rs_addr),
    .IF_ID_rt(rt_addr),
    .load_use(load_use)
);


// ID/EX过渡阶段：冲刷处理
assign EX_Flush = load_use | BranchTaken;
// ID/EX阶段：寄存器更新
always @(posedge CLK or posedge RST) begin
    if (RST || EX_Flush) begin
        // 插入气泡
        ID_EX_IR <= 32'h00000000;
        ID_EX_PC1 <= 32'h00000000;
        ID_EX_Rs <= 32'h00000000;
        ID_EX_Rt <= 32'h00000000;
        ID_EX_Imm32 <= 32'h00000000;
        ID_EX_rs_addr <= 5'b00000;
        ID_EX_rt_addr <= 5'b00000;
        ID_EX_ALUSrc <= 1'b0;
        ID_EX_RegDst <= 1'b0;
        ID_EX_Branch <= 1'b0;
        ID_EX_ALUControl <= 3'b000;
        ID_EX_RegWrite <= 1'b0;
        ID_EX_MemToReg <= 1'b0;
        ID_EX_MemWrite <= 1'b0;
    end else begin
        ID_EX_IR <= IF_ID_IR;
        ID_EX_PC1 <= IF_ID_PC1;
        ID_EX_Rs <= Rs_data;
        ID_EX_Rt <= Rt_data;
        ID_EX_Imm32 <= SignImm;
        ID_EX_rs_addr <= rs_addr;
        ID_EX_rt_addr <= rt_addr;
        ID_EX_ALUSrc <= ALUSrc;
        ID_EX_RegDst <= RegDst;
        ID_EX_Branch <= Branch;
        ID_EX_ALUControl <= ALUControl;
        ID_EX_RegWrite <= RegWrite;
        ID_EX_MemToReg <= MemToReg;
        ID_EX_MemWrite <= MemWrite;
    end
end

// EX阶段：数据冒险解决模块
ForwardingUnit FU(
    .ID_EX_rs(ID_EX_rs_addr),
    .ID_EX_rt(ID_EX_rt_addr),
    .EX_MEM_RegWrite(EX_MEM_RegWrite),
    .EX_MEM_WriteReg(EX_MEM_WriteReg),
    .MEM_WB_RegWrite(MEM_WB_RegWrite),
    .MEM_WB_WriteReg(MEM_WB_WriteReg),
    .ForwardA(ForwardA),
    .ForwardB(ForwardB)
);

// ALU输入A的前递选择
assign ALU_A_forward = (ForwardA == 2'b10) ? EX_MEM_ALUOut :    // 从EX阶段前递
                       (ForwardA == 2'b01) ? Writeback_data :   // 从MEM阶段前递
                       ID_EX_Rs;                                // 无前递
// ALU输入B的前递选择
//考虑立即数
wire [31:0] Rt_forward;
assign Rt_forward = (ForwardB == 2'b10) ? EX_MEM_ALUOut :       // 从EX前递
                    (ForwardB == 2'b01) ? Writeback_data :      // 从MEM前递
                    ID_EX_Rt;                                   // 无前递
assign ALU_A = ALU_A_forward;
assign ALU_B = ID_EX_ALUSrc ? ID_EX_Imm32 : Rt_forward;         // 选择立即数或寄存器值

// 写回寄存器地址选择
assign Write_reg_addr = ID_EX_RegDst ? ID_EX_IR[15:11] : ID_EX_IR[20:16];
// 分支目标地址计算
assign Branch_target = ID_EX_PC1 + (ID_EX_Imm32 << 2);
//EX阶段：ALU模块
ALU alu(
    .A(ALU_A),
    .B(ALU_B),
    .ALUControl(ID_EX_ALUControl),
    .Result(ALU_result),
    .Zero(Zero)
);

// EX/MEM过渡阶段：寄存器更新
always @(posedge CLK or posedge RST) begin
    if (RST || BranchTaken) begin
        EX_MEM_IR <= 32'h00000000;
        EX_MEM_Rt <= 32'h00000000;
        EX_MEM_PC2 <= 32'h00000000;
        EX_MEM_PC3 <= 32'h00000000;
        EX_MEM_ALUOut <= 32'h00000000;
        EX_MEM_Zero <= 1'b0;
        EX_MEM_Branch <= 1'b0;
        EX_MEM_WriteReg <= 5'b00000;
        EX_MEM_RegWrite <= 1'b0;
        EX_MEM_MemToReg <= 1'b0;
        EX_MEM_MemWrite <= 1'b0;
    end else begin
        EX_MEM_IR <= ID_EX_IR;
        EX_MEM_Rt <= Rt_forward;            // 使用前递后的值
        EX_MEM_PC2 <= Branch_target;        // 分支目标地址
        EX_MEM_PC3 <= {ID_EX_PC1[31:28], ID_EX_IR[25:0], 2'b00};
        EX_MEM_ALUOut <= ALU_result;
        EX_MEM_Zero <= Zero;
        EX_MEM_Branch <= ID_EX_Branch;
        EX_MEM_WriteReg <= Write_reg_addr;
        EX_MEM_RegWrite <= ID_EX_RegWrite;
        EX_MEM_MemToReg <= ID_EX_MemToReg;
        EX_MEM_MemWrite <= ID_EX_MemWrite;
    end
end


// MEM阶段：数据存储器模块
DMem DM(
    .CLK(CLK),
    .MemWrite(EX_MEM_MemWrite),
    .A(EX_MEM_ALUOut),
    .WD(EX_MEM_Rt),
    .RD(Memread_data)
);

// MEM/WB过渡阶段：寄存器更新
always @(posedge CLK or posedge RST) begin
    if (RST) begin
        MEM_WB_IR <= 32'h00000000;
        MEM_WB_ALUOut <= 32'h00000000;
        MEM_WB_MEMOut <= 32'h00000000;
        MEM_WB_WriteReg <= 5'b00000;
        MEM_WB_RegWrite <= 1'b0;
        MEM_WB_MemToReg <= 1'b0;
    end else begin
        MEM_WB_IR <= EX_MEM_IR;
        MEM_WB_ALUOut <= EX_MEM_ALUOut;
        MEM_WB_MEMOut <= Memread_data;
        MEM_WB_WriteReg <= EX_MEM_WriteReg;
        MEM_WB_RegWrite <= EX_MEM_RegWrite;
        MEM_WB_MemToReg <= EX_MEM_MemToReg;
    end
end

// WB阶段：写回数据选择
assign Writeback_data = MEM_WB_MemToReg ? MEM_WB_MEMOut : MEM_WB_ALUOut;

endmodule



