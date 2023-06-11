
module led_matrix_top #(
    parameter PANEL_ROWS  = 0,
    parameter PANEL_COLS  = 0,
    parameter COLOR_DEPTH = 0
)(
    input clk,
    input rst_n,
    input [7:0] prescale,

    // user interface
    input                         valid_in,
    input                         sync_in,
    input  [2:0][COLOR_DEPTH-1:0] rgb_in,
    output                        sync_out,

    // matrix interface
    output                            matrix_clk,
    output                            matrix_stb,
    output                            matrix_oe_n,
    output [$clog2(PANEL_ROWS/2)-1:0] matrix_row,
    output [2:0]                      matrix_rgb_upper,
    output [2:0]                      matrix_rgb_lower

);

    reg                          prescaler_enable;
    reg                          prescaler_bypass;
    reg                          line_sync;
    reg [$clog2(PANEL_COLS)-1:0] frame_column;
    reg                          frame_sync;
    reg [3*COLOR_DEPTH-1:0]      frame_rgb_upper;
    reg [3*COLOR_DEPTH-1:0]      frame_rgb_lower;

    led_matrix_prescaler #(
        .WIDTH(8)
    ) u_prescaler (
        .clk             (clk),
        .rst_n           (rst_n),
        .prescale        (prescale),
        .prescaler_enable(prescaler_enable),
        .prescaler_bypass(prescaler_bypass)
    );

    led_matrix_framebuffer #(
        .PANEL_COLS (PANEL_COLS),
        .PANEL_ROWS (PANEL_ROWS),
        .COLOR_DEPTH(COLOR_DEPTH)
    ) u_framebuffer (
        .clk          (clk),
        .rst_n        (rst_n),
        .sync_in      (sync_in),
        .valid_in     (valid_in),
        .rgb_in       (rgb_in),
        .frame_sync   (frame_sync),
        .line_sync    (line_sync),
        .matrix_row   (matrix_row),
        .frame_column (frame_column),
        .rgb_out_upper(frame_rgb_upper),
        .rgb_out_lower(frame_rgb_lower)
    );

    led_matrix_driver #(
        .PANEL_COLS(PANEL_COLS)
    ) u_driver (
        .clk             (clk),
        .rst_n           (rst_n),
        .prescaler_enable(prescaler_enable),
        .prescaler_bypass(prescaler_bypass),
        .frame_column    (frame_column),
        .line_sync       (line_sync),
        .matrix_clk      (matrix_clk),
        .matrix_oe_n     (matrix_oe_n),
        .matrix_stb      (matrix_stb)
    );

    led_matrix_bcm #(
        .COLOR_DEPTH(COLOR_DEPTH)
    ) u_bcm (
        .clk             (clk),
        .rst_n           (rst_n),
        .frame_sync      (frame_sync),
        .image_sync      (sync_out),
        .frame_rgb_upper (frame_rgb_upper),
        .frame_rgb_lower (frame_rgb_lower),
        .matrix_rgb_upper(matrix_rgb_upper),
        .matrix_rgb_lower(matrix_rgb_lower)
    );

endmodule
