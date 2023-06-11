
module led_matrix_prescaler #(
    parameter WIDTH = 0
)(
    input              clk,
    input              rst_n,
    input  [WIDTH-1:0] prescale,
    output             prescaler_enable,
    output             prescaler_bypass
);

reg enable;
reg [WIDTH-1:0] cnt;

assign enable = cnt == prescale;

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        cnt <= '0;
    end
    else begin
        if(enable)
            cnt <= '0;
        else
            cnt <= cnt + 1'b1;
    end
end

assign prescaler_enable = enable;
assign prescaler_bypass = prescale == '0;

endmodule
