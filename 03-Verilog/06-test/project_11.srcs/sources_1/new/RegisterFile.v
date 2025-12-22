module RegisterFile(
    input wire clk,
    input wire rst,
    input wire RegWrite,
    input wire [4:0] ReadReg1,
    input wire [4:0] ReadReg2,
    input wire [4:0] WriteReg,
    input wire [31:0] WriteData,
    output wire [31:0] ReadData1,
    output wire [31:0] ReadData2
);

reg [31:0] registers [0:31];

integer i;
always @(negedge clk or posedge rst) begin
    if (rst) begin
        for (i = 0; i < 32; i = i + 1)
            registers[i] <= 32'h00000000;
    end else if (RegWrite && WriteReg != 5'b00000) begin
        registers[WriteReg] <= WriteData;
    end
end

assign ReadData1 = (ReadReg1 == 5'b00000) ? 32'h00000000 : registers[ReadReg1];
assign ReadData2 = (ReadReg2 == 5'b00000) ? 32'h00000000 : registers[ReadReg2];

endmodule

