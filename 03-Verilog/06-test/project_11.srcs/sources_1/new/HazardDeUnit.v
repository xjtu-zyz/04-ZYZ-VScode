`timescale 1ns / 1ps
//load-use
module HazardDeUnit(
    input  ID_EX_MemToReg,
    input [4:0] ID_EX_rt,
    input [4:0] IF_ID_rs,
    input [4:0] IF_ID_rt,
    output reg load_use
);

always @(*) begin
    
    if (ID_EX_MemToReg && ((ID_EX_rt == IF_ID_rs)||(ID_EX_rt == IF_ID_rt)) && (ID_EX_rt != 5'b00000))    
    {   
        //前一条指令是lw
        // lw的寄存器是R型指令的源寄存器
        load_use= 1'b1;  
    } 
    else load_use= 1'b0;
end

endmodule
