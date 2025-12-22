module sim_PipelineMIPS_CPU;

  reg CLK;
  reg RST;
  PipelineMIPS_CPU dut (
    .CLK(CLK),
    .RST(RST)
  );

  initial begin
    CLK = 1'b0;
    forever #5 CLK = ~CLK;
  end

  initial begin
    RST = 1'b1;
    repeat(4) @(posedge CLK);
    RST = 1'b0;

    repeat(200) @(posedge CLK);
  end

endmodule