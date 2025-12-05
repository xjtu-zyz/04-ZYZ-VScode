// module RegFile(
//     input clk,
//     input RegWrite,
//     input [4:0] readReg1, readReg2, writeReg,
//     input [31:0] writeData,
//     output [31:0] readData1, readData2
// );
//     reg [31:0] registers [0:31];
//     integer i;
    
//     initial begin
//         for (i = 0; i < 32; i = i + 1)
//             registers[i] = 32'b0;
//     end
    
//     // ¶Á²Ù×÷ - ×éºÏÂß¼­
//     assign readData1 = (readReg1 != 0) ? registers[readReg1] : 32'b0;
//     assign readData2 = (readReg2 != 0) ? registers[readReg2] : 32'b0;
    
//     // Ð´²Ù×÷ - Ê±ÐòÂß¼­
//     always @(posedge clk) begin
//         if (RegWrite && writeReg != 0) begin
//             registers[writeReg] <= writeData;
//         end
//     end
// endmodule
`define DATA_WIDTH 32

module RegFile #(
    parameter ADDR_SIZE = 5
)(
    input CLK,
    input WE3,
    input [ADDR_SIZE-1:0] RA1, RA2, WA3,
    input [`DATA_WIDTH-1:0] WD,
    output [`DATA_WIDTH-1:0] RD1, RD2
);

    // ????? - 32?32????
    reg [`DATA_WIDTH-1:0] rf[0:(2 ** ADDR_SIZE)-1];//2**5=2^5=32
    
    //???? - ?????
    always @(posedge CLK) begin
        if (WE3) rf[WA3] <= WD;
    end
    
    //???? - ?????
    assign RD1 = (RA1 != 0) ? rf[RA1] : 0;
    assign RD2 = (RA2 != 0) ? rf[RA2] : 0;

endmodule