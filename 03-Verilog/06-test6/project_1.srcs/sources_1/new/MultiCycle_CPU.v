module MultiCycleCPU (
    input clk,
    input reset
);
    // 定义状态
    parameter FETCH = 3'b000;
    parameter DECODE = 3'b001;
    parameter EXECUTE = 3'b010;
    parameter MEM_ACCESS = 3'b011;
    parameter WRITE_BACK = 3'b100;

    reg [2:0] state, next_state;
    
    // 内部信号
    reg [31:0] PC;
    wire [31:0] PC_out, Instruction;
    reg [31:0] IR; // 指令寄存器
    reg [31:0] MDR; // 存储器数据寄存器
    reg [31:0] ALUOut; // ALU输出寄存器
    reg [31:0] A, B; // 寄存器读取值
    
    wire [31:0] ReadData1, ReadData2, ALUResult, MemReadData;
    wire [31:0] SignExtImm;  // 改为wire类型
    reg [31:0] ALUInputA, ALUInputB;
    reg [4:0] WriteReg;
    wire [2:0] ALUControl;
    wire Zero;
    
    // 控制信号
    wire RegWrite, RegDst, ALUSrc, Branch, MemWrite, MemToReg, Jump, PCSrc;
    wire [1:0] ALUOp;
    
    // PC计算相关信号
    wire [31:0] PCPlus4, PCBranch, PCJump;
    reg PCWrite;    //PC写使能
    wire [31:0] next_PC;  // 重命名避免冲突
    
    // 实例化程序计数器
    PC PC_unit (
        .clk(clk),
        .reset(reset),
        .nextPC(PC),
        .currentPC(PC_out)
    );
    
    // 实例化指令存储器
    IMem IM (
        .A(PC_out),
        .RD(Instruction)
    );
        wire [31:0] WriteData;
    // 寄存器组
    RegFile RF(
        .CLK(clk),
        .WE3(RegWrite),
        .RA1(IR[25:21]),
        .RA2(IR[20:16]),
        .WA3(WriteReg),
        .WD(WriteData),
        .RD1(ReadData1),
        .RD2(ReadData2)
    );
    
    // 实例化数据存储器
    DMem DM (
        .CLK(clk),
        .WE(MemWrite),
        .A(ALUOut),
        .WD(B),
        .RD(MemReadData)
    );
    
    // 控制单元
    Controller CU(
        .Op(IR[31:26]),
        .Funct(IR[5:0]),
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
    
    // ALU
    ALU alu (
        .A(ALUInputA),
        .B(ALUInputB),
        .OP(ALUControl),
        .F(ALUResult),
        .ZF(Zero)
    );
    
    // 符号扩展 - 使用assign而不是reg
    assign SignExtImm = {{16{IR[15]}}, IR[15:0]};
    
    //PC计算
   assign PCPlus4 = PC_out + 4;
   assign PCBranch = PCPlus4 + (SignExtImm << 2);
    assign PCJump = {PCPlus4[31:28], IR[25:0], 2'b00};
    
    // 下一条PC选择 - 使用重命名的信号
//    assign next_PC =(Jump) ? PCJump :
//                    (PCSrc ? PCBranch : PCPlus4);
    
    // 写回数据选择

    assign WriteData = MemToReg ? MDR : ALUOut;
    
    // 状态寄存器
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state <= FETCH;
            IR <= 32'b0;
            PC <= 0; 
            MDR <= 32'b0;
            ALUOut <= 32'b0;
        end else begin
            state <= next_state;
            
            // 在FETCH阶段更新IR
            if (state == FETCH) begin
                IR <= Instruction;
                    PC<=(Jump) ? {PCPlus4[31:28], IR[25:0], 2'b00} :
                         (PCSrc ? PC_out + 4+ ({{16{IR[15]}}, IR[15:0]} << 2) : PC_out + 4);
            end
            
            // 在DECODE阶段读取寄存器值
            if (state == DECODE) begin
                A <= ReadData1;
                B <= ReadData2;
             
            end
            
            // 在EXECUTE阶段保存ALU结果
            if (state == EXECUTE) begin
                ALUOut <= ALUResult;
               
            end
            
            // 在MEM_ACCESS阶段保存存储器数据
            if (state == MEM_ACCESS) begin
                MDR <= MemReadData;
            end
            //这几步都没有成功实现，于是我最后选择不输出观察
        end
    end
    
    // 下一状态逻辑
    always @(*) begin
        case (state)
            FETCH: next_state = DECODE;
            
            DECODE: begin
                case (IR[31:26])
                    6'b000000: next_state = EXECUTE; // R-type
                    6'b100011: next_state = EXECUTE; // LW
                    6'b101011: next_state = EXECUTE; // SW
                    6'b000100: next_state = EXECUTE; // BEQ
                    6'b001000: next_state = EXECUTE; // ADDI
                    6'b000010: next_state = FETCH;   // J (跳转后返回FETCH)
                    default: next_state = FETCH;
                endcase
            end
            
            EXECUTE: begin
                case (IR[31:26])
                    6'b000000: next_state = WRITE_BACK; // R-type
                    6'b100011: next_state = MEM_ACCESS; // LW
                    6'b101011: next_state = MEM_ACCESS; // SW
                    6'b000100: next_state = FETCH;      // BEQ(完成分支后返回FETCH)
                    6'b001000: next_state = WRITE_BACK; // ADDI
                    default: next_state = FETCH;
                endcase
            end
            
            MEM_ACCESS: begin
                case (IR[31:26])
                    6'b100011: next_state = WRITE_BACK; // LW
                    6'b101011: next_state = FETCH;      // SW(完成存储后返回FETCH)
                    default: next_state = FETCH;
                endcase
            end
            
            WRITE_BACK: next_state = FETCH;
            
            default: next_state = FETCH;
        endcase
    end
    
    // ALU输入选择
    always @(*) begin
        case (state)
            FETCH: begin
                ALUInputA = PC_out;
                ALUInputB = 32'd4;
            end
            EXECUTE: begin
                ALUInputA = A;
                // 使用控制器输出的ALUSrc信号
                ALUInputB = ALUSrc ? SignExtImm : B;
            end
            default: begin
                ALUInputA = A;
                ALUInputB = B;
            end
        endcase
    end
    
    // 写寄存器选择
    always @(*) begin
        WriteReg = RegDst ? IR[15:11] : IR[20:16];
    end
    
    // // PC写使能控制
    // always @(*) begin
    //     // 在FETCH状态总是更新PC，或者在分支/跳转时更新
    //     PCWrite = (state == FETCH) || 
    //              (state == EXECUTE && IR[31:26] == 6'b000100 && Zero) || // BEQ
    //              (state == DECODE && IR[31:26] == 6'b000010); // J
    // end

endmodule