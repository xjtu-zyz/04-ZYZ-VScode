module RegFile(
    input CLK,
    input RST,
    input RW,               //RegWrite
    input [4:0] RA1,
    input [4:0] RA2,
    input [4:0] WR,         //WriteReg
    input [31:0] WD,
    output [31:0] RD1,
    output [31:0] RD2
);

reg [31:0] register [0:31];

integer i;
always @(negedge CLK or posedge RST) begin
    if (RST) begin
        for (i = 0; i < 32; i = i + 1)
            register[i] <= 32'h00000000;
    end 
    else if (RW && WR != 5'b00000) begin
        register[WR] <= WD;
    end
end

assign RD1 = (RA1 == 5'b00000) ? 32'h00000000 : register[RA1];
assign RD2 = (RA2 == 5'b00000) ? 32'h00000000 : register[RA2];

endmodule