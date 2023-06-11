
module led_matrix_bcm #(
    parameter COLOR_DEPTH = 0
)(

    input                      clk,
    input                      rst_n,
    input                      frame_sync,
    input  [3*COLOR_DEPTH-1:0] frame_rgb_upper,
    input  [3*COLOR_DEPTH-1:0] frame_rgb_lower,
    output reg [2:0]           matrix_rgb_upper,
    output reg [2:0]           matrix_rgb_lower,
    output                     image_sync
    
);

reg [COLOR_DEPTH-1:0] bcm_count;
reg [COLOR_DEPTH-1:0] bcm_count_rev;
reg [COLOR_DEPTH-1:0] bcm_tick;
reg [COLOR_DEPTH-1:0] bcm_tick_rev;

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        bcm_count <= '0;
    end
    else begin
        if(frame_sync)
            bcm_count <= bcm_count + 1'b1;
    end
end

assign bcm_tick_rev = bcm_count_rev & ~(bcm_count_rev - 1'b1);

always_comb begin
    for(int i = 0; i < COLOR_DEPTH; i++) begin
        bcm_count_rev[COLOR_DEPTH-i-1] = bcm_count[i];
        bcm_tick[COLOR_DEPTH-i-1] = bcm_tick_rev[i];
    end
end

always_comb begin
    for(int i = 0; i < 3; i++) begin
        matrix_rgb_upper[i] = |(bcm_tick & frame_rgb_upper[i*COLOR_DEPTH +: COLOR_DEPTH]);
        matrix_rgb_lower[i] = |(bcm_tick & frame_rgb_lower[i*COLOR_DEPTH +: COLOR_DEPTH]);
    end
end

assign image_sync = frame_sync & (bcm_count == '1);

endmodule
