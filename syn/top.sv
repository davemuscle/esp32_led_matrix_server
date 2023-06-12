
module top (
    input  reg        deca_max10_clk1_50,
    output reg [7:0]  deca_led,
    input  reg [1:0]  deca_key,
    input  reg [1:0]  deca_sw,
    inout  reg [43:0] deca_gpio0,
    inout  reg [24:0] deca_gpio1
);

    reg prescaler_enable;
    reg prescaler_bypass;

    reg clk;
    reg rst_n;

    reg sync_in;
    reg valid_in;
    reg [11:0] rgb_in;
    reg sync_out;

    reg       matrix_clk;
    reg [2:0] matrix_rgb_upper;
    reg [2:0] matrix_rgb_lower;
    reg [4:0] matrix_row;
    reg       matrix_oe_n;
    reg       matrix_stb;

    reg uart_tx, uart_rx;
    reg uart_valid, uart_ready;
    reg [7:0] uart_data;

    assign clk = deca_max10_clk1_50;

    cdc_reset_sync
    u_reset_sync (
        .clk   (clk),
        .arst_n(deca_key[0]),
        .rst_n (rst_n)
    );

    led_matrix_top #(
        .PANEL_ROWS(64),
        .PANEL_COLS(64),
        .COLOR_DEPTH(4)
    )
    u_led_matrix_top (
        .*, .prescale('d1)
    );

    reg [2:0] cnt;

    assign sync_in = 1'b0;
    assign uart_ready = 1'b1;

    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            cnt <= '0;
            rgb_in <= '0;
            valid_in <= 1'b0;
        end
        else begin
            valid_in <= (uart_valid & (cnt == 'd2));
            if(uart_valid) begin
                cnt <= (cnt == 'd2) ? '0 : cnt + 1'b1;
                rgb_in[cnt * 4 +: 4] <= uart_data[7:4];
            end
        end
    end


    uart #(
        .UART_CLKRATE_KHZ(50000),
        .UART_BAUDRATE_HZ(115200),
        .UART_STOP_BITS  (1)
    ) u_uart (
        .clk     (clk),
        .rst_n   (rst_n),
        .tx_valid(1'b0),
        .tx_ready(),
        .tx_data (),
        .rx_valid(uart_valid),
        .rx_ready(uart_ready),
        .rx_data (uart_data),
        .uart_tx (uart_tx),
        .uart_rx (uart_rx)
    );

    reg [7:0] z;
    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            z <= '1;
        end
        else begin
            if(uart_valid & uart_ready) begin
                z <= ~uart_data;
            end
        end
    end

    assign deca_led = z;

    assign deca_gpio0[0] = matrix_clk;

    // green and blue are swapped on the matrix?
    assign deca_gpio0 [2]  = matrix_rgb_upper[0];
    assign deca_gpio0 [4]  = matrix_rgb_upper[2];
    assign deca_gpio0 [6]  = matrix_rgb_upper[1];
    assign deca_gpio0 [8]  = matrix_rgb_lower[0];
    assign deca_gpio0 [10] = matrix_rgb_lower[2];
    assign deca_gpio0 [12] = matrix_rgb_lower[1];

    assign deca_gpio0 [14] = matrix_row[0];
    assign deca_gpio0 [16] = matrix_row[1];
    assign deca_gpio0 [18] = matrix_row[2];
    assign deca_gpio0 [20] = matrix_row[3];
    assign deca_gpio0 [22] = matrix_row[4];

    assign deca_gpio0[42] = matrix_oe_n;
    assign deca_gpio0[43] = matrix_stb;

    assign deca_gpio1[0] = uart_tx;
    assign uart_rx = deca_gpio1[1];
    
endmodule
