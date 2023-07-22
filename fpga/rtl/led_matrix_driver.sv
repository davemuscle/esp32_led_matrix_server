
module led_matrix_driver #(
    parameter PANEL_COLS    = 0,
    parameter OUTPUT_CYCLES = 5
)(

    input clk,
    input rst_n,
    input prescaler_enable,
    input prescaler_bypass,

    output [$clog2(PANEL_COLS)-1:0] frame_column,
    output                          line_sync,

    output matrix_clk,
    output matrix_oe_n,
    output matrix_stb

);

typedef enum {
    RESET, CLOCK, LATCH, DELAY1, SWITCH, DELAY2, ENABLE
} state_t;

state_t state, state_next;

reg                             clock_done;
reg                             clock_toggle;
reg                             enable;
reg [$clog2(PANEL_COLS)-1:0]    column;
reg                             column_enable;
reg [$clog2(OUTPUT_CYCLES)-1:0] oe_cycles;

//--------------------------------------------------------------------------------------------------
// Enable
//--------------------------------------------------------------------------------------------------

assign enable = prescaler_enable | prescaler_bypass;

//--------------------------------------------------------------------------------------------------
// StateMachine
//--------------------------------------------------------------------------------------------------

assign clock_done = (prescaler_bypass | clock_toggle) & (column == (PANEL_COLS-1));

always_comb begin
    if(enable) begin
        case(state)
            RESET    : state_next = CLOCK;
            CLOCK    : state_next = clock_done ? LATCH : CLOCK;
            LATCH    : state_next = DELAY1;
            DELAY1   : state_next = SWITCH;
            SWITCH   : state_next = DELAY2;
            DELAY2   : state_next = ENABLE;
            ENABLE   : state_next = CLOCK;
        endcase
    end
    else begin
        state_next = state;
    end
end

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        state <= RESET;
    end
    else begin
        state <= state_next;
    end
end

`ifndef SYNTHESIS
    bit [255:0] state_string;
    always_comb 
        case(state)
            RESET    : state_string = "RESET";
            CLOCK    : state_string = "CLOCK";
            LATCH    : state_string = "LATCH";
            ENABLE   : state_string = "ENABLE";
            SWITCH   : state_string = "SWITCH";
            DELAY1   : state_string = "DELAY1";
            DELAY2   : state_string = "DELAY2";
        endcase
`endif

//--------------------------------------------------------------------------------------------------
// Clock Toggle
//--------------------------------------------------------------------------------------------------

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        clock_toggle <= 1'b0;
    end
    else begin
        if(prescaler_bypass | (state != CLOCK))
            clock_toggle <= 1'b0;
        else if(enable & (state == CLOCK))
            clock_toggle <= !clock_toggle;
    end
end

//--------------------------------------------------------------------------------------------------
// Bit Counter
//--------------------------------------------------------------------------------------------------

assign column_enable = enable & (state == CLOCK) & (prescaler_bypass | clock_toggle);

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        column <= '0;
    end
    else begin
        if(state == LATCH)
            column <= '0;
        else if(column_enable)
            column <= column + 1'b1;
    end
end

//--------------------------------------------------------------------------------------------------
// Interface Outputs
//--------------------------------------------------------------------------------------------------
assign frame_column = column;
assign line_sync = state == SWITCH & state_next != SWITCH;

//--------------------------------------------------------------------------------------------------
// Panel Outputs
//--------------------------------------------------------------------------------------------------

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        oe_cycles <= '0;
    end
    else begin
        if(state != CLOCK) begin
            oe_cycles <= '0;
        end
        else if(enable & (state == CLOCK) & !clock_toggle & (oe_cycles != $clog2(OUTPUT_CYCLES)'(OUTPUT_CYCLES-1))) begin
            oe_cycles <= oe_cycles + 1'b1;
        end
    end
end

assign matrix_clk  = (prescaler_bypass ? !clk : clock_toggle) & state == CLOCK;
assign matrix_oe_n = !(state == CLOCK & (oe_cycles < OUTPUT_CYCLES-1));
assign matrix_stb  = state == LATCH;

endmodule
