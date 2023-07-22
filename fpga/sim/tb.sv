
`timescale 1ns/1ns

module tb;

    reg clk;
    reg rst_n;

    reg [7:0]      prescale;
    reg            valid_in;
    reg [2:0][3:0] rgb_in;
    reg            sync_in;
    reg            sync_out;

    reg       matrix_clk;
    reg [4:0] matrix_row;
    reg [2:0] matrix_rgb_upper;
    reg [2:0] matrix_rgb_lower;
    reg       matrix_oe_n;
    reg       matrix_stb;

    initial begin
        rst_n = 1'b0;
        #50;
        rst_n = 1'b1;
    end

    initial begin
        clk = 1'b0;
        forever clk = #1 !clk;
    end

    assign prescale = 1;

    reg [31:0] cnt;

    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            valid_in <= 1'b0;
            sync_in <= 1'b0;
            cnt <= '0;
            rgb_in <= '0;
        end
        else begin
            sync_in <= 1'b0;
            if(cnt < 4096) begin
                cnt <= cnt + 1'b1;
                valid_in <= 1'b1;
                if(cnt < 16)
                    rgb_in <= {8'h00, 4'(cnt)};
                else if(cnt == 31 || cnt == 32)
                    rgb_in <= 12'h080;
                else if(cnt >= 56 & cnt < 64)
                    rgb_in <= 12'hF00;
                else if(cnt >= 4096-64 & cnt < 4096)
                    rgb_in <= 12'hFFF;
                else
                    rgb_in <= '0;
            end
            else begin
                rgb_in <= '0;
                valid_in <= 1'b0;
            end
        end
    end

    led_matrix_top #(
        .PANEL_ROWS(64),
        .PANEL_COLS(64),
        .COLOR_DEPTH(4) 
    ) dut (.*);

    initial begin
        repeat(500000) @(posedge clk);
        $finish;
    end

endmodule

