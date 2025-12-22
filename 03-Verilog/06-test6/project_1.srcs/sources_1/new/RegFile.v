
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

    reg [`DATA_WIDTH-1:0] rf[0:(2 ** ADDR_SIZE)-1];//2**5=2^5=32
    
    always @(posedge CLK) begin
        if (WE3) rf[WA3] <= WD;
    end
    
    assign RD1 = (RA1 != 0) ? rf[RA1] : 0;
    assign RD2 = (RA2 != 0) ? rf[RA2] : 0;

endmodule