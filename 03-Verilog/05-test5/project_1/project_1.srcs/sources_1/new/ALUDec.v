module ALUDec(
    input [5:0] Funct,
    input [1:0] ALUOp,
    output reg [2:0] ALUControl
);
    always @(*) begin
        case(ALUOp)
            2'b00: ALUControl = 3'b010; // add (lw/sw/addi)
            2'b01: ALUControl = 3'b110; // sub (beq)
            2'b11: ALUControl = 3'b100; // lui      //???
            default: begin // R-type    ALUOp=10
                case(Funct)
                    6'b100000: ALUControl = 3'b010; // addu
                    6'b100010: ALUControl = 3'b110; // subu
                    6'b100101: ALUControl = 3'b001; // or
                    6'b100100: ALUControl = 3'b000; // and
                    6'b101010: ALUControl = 3'b111; // slt 
                    //6'b000000: ALUControl = 3'b000; // nop 
                    default:   ALUControl = 3'b010; // add
                endcase
            end
        endcase
    end
endmodule