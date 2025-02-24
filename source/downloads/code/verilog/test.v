module test (
        input reg a,
        output reg       out
    );


endmodule //test

//~ `New testbench
`timescale  1ns / 1ps

module tb_test;

    // test Parameters
    parameter PERIOD  = 10;


    // test Inputs
    reg   a                                    = 0 ;

    // test Outputs
    wire  out                                  ;


    initial
    begin
        forever #(PERIOD/2)  clk=~clk;
    end

    initial
    begin
        #(PERIOD*2) rst_n  =  1;
    end

    test  u_test (
              .a           ( a     ),

              .out                     ( out   )
          );

    initial
    begin

        $finish;
    end

endmodule
