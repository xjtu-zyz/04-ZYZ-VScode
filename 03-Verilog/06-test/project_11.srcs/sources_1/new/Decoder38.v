module Decoder38(ALUControl,EN);
    input [2:0] ALUControl;
    output reg[8:0]EN;
    integer i;
    
    always @(ALUControl)begin
        EN = 8'b0000_0000;
        for (i = 0;i< 8; i = i+1)
            if(ALUControl == i) EN[i] = 1;
            else EN[i] = 0;
     end
endmodule
