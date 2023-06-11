
module led_matrix_framebuffer #(
    parameter PANEL_ROWS = 64,
    parameter PANEL_COLS = 64,
    parameter COLOR_DEPTH = 8
)(
    input clk,
    input rst_n,

    input                                 sync_in,
    input                                 valid_in,
    input  [3*COLOR_DEPTH-1:0]            rgb_in,
    output                                frame_sync,
    output reg [$clog2(PANEL_ROWS/2)-1:0] matrix_row,

    input                           line_sync,
    input  [$clog2(PANEL_COLS)-1:0] frame_column,
    output reg [3*COLOR_DEPTH-1:0]  rgb_out_upper,
    output reg [3*COLOR_DEPTH-1:0]  rgb_out_lower

);

localparam FRAME_AW = $clog2(PANEL_ROWS*PANEL_COLS);
localparam BUFFER_AW = $clog2(3*PANEL_ROWS*PANEL_COLS/2);

reg [2:0]                      wr_sel;
reg                            wr_sync;
reg [2:0]                      wr_sel_frwd;
reg [2:0]                      wr_sel_back;
reg [2:0]                      rd_sel;
reg [2:0]                      frame_sel;
reg [$clog2(PANEL_ROWS/2)-1:0] frame_row;

reg [FRAME_AW-1:0]  wr_addr;
reg [FRAME_AW-2:0]  rd_addr;
reg [BUFFER_AW-1:0] wr_offset;
reg [BUFFER_AW-1:0] rd_offset;

reg [3*COLOR_DEPTH-1:0] frame_upper [(3*PANEL_ROWS*PANEL_COLS/2)-1:0];
reg [3*COLOR_DEPTH-1:0] frame_lower [(3*PANEL_ROWS*PANEL_COLS/2)-1:0];
reg [3*COLOR_DEPTH-1:0] frame_data_upper;
reg [3*COLOR_DEPTH-1:0] frame_data_lower;

reg [1:0] mem_wr_en;
reg [BUFFER_AW-1:0] mem_wr_addr;
reg [BUFFER_AW-1:0] mem_rd_addr;

//--------------------------------------------------------------------------------------------------
// Panel Row Counter
//--------------------------------------------------------------------------------------------------
always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        frame_row <= '0;
        matrix_row <= '0;
    end
    else if(line_sync) begin
        frame_row <= frame_row + 1'b1; 
        matrix_row <= frame_row;
    end
end

assign frame_sync = line_sync & (frame_row == (PANEL_ROWS/2)-1);

//--------------------------------------------------------------------------------------------------
// Addresses
//--------------------------------------------------------------------------------------------------
always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        wr_addr <= '0;
    end
    else if(sync_in) begin
        wr_addr <= '0;
    end
    else if(valid_in) begin
        wr_addr <= wr_addr + 1'b1;
    end
end

assign rd_addr = {frame_row, frame_column};

//--------------------------------------------------------------------------------------------------
// Triple-buffer Muxing
//--------------------------------------------------------------------------------------------------

assign wr_sel_frwd = {wr_sel[1], wr_sel[0], wr_sel[2]};
assign wr_sel_back = {wr_sel[0], wr_sel[2], wr_sel[1]};

assign wr_sync = sync_in | (valid_in & (wr_addr == ((PANEL_ROWS*PANEL_COLS)-1)));

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        wr_sel <= 3'b001;
        frame_sel <= 3'b000;
    end
    else if(wr_sync) begin
        frame_sel <= wr_sel;
        wr_sel <= (rd_sel == wr_sel_frwd) ? wr_sel_back : wr_sel_frwd;
    end
end

always_ff @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        rd_sel <= 3'b000;
    end
    else if(frame_sync) begin
        rd_sel <= frame_sel; 
    end
end

always_comb begin
    case(wr_sel)
        3'b100:  wr_offset = BUFFER_AW'(2*(PANEL_ROWS*PANEL_COLS/2));
        3'b010:  wr_offset = BUFFER_AW'(1*(PANEL_ROWS*PANEL_COLS/2));
        default: wr_offset = BUFFER_AW'(0*(PANEL_ROWS*PANEL_COLS/2));
    endcase
end

always_comb begin
    case(rd_sel)
        3'b100:  rd_offset = BUFFER_AW'(2*(PANEL_ROWS*PANEL_COLS/2));
        3'b010:  rd_offset = BUFFER_AW'(1*(PANEL_ROWS*PANEL_COLS/2));
        default: rd_offset = BUFFER_AW'(0*(PANEL_ROWS*PANEL_COLS/2));
    endcase
end

//--------------------------------------------------------------------------------------------------
// BRAMs
//--------------------------------------------------------------------------------------------------

assign mem_wr_en[0] = valid_in & (wr_addr[FRAME_AW-1] == 1'b0);
assign mem_wr_en[1] = valid_in & (wr_addr[FRAME_AW-1] == 1'b1);

assign mem_wr_addr = wr_addr[FRAME_AW-2:0] + wr_offset;
assign mem_rd_addr = rd_addr + rd_offset;

always_ff @(posedge clk) begin
    if(mem_wr_en[0]) begin
        frame_upper[mem_wr_addr] <= rgb_in;
    end
    frame_data_upper <= frame_upper[mem_rd_addr];
end
always_ff @(posedge clk) begin
    if(mem_wr_en[1]) begin
        frame_lower[mem_wr_addr] <= rgb_in;
    end
    frame_data_lower <= frame_lower[mem_rd_addr];
end

//--------------------------------------------------------------------------------------------------
// Mux Out
//--------------------------------------------------------------------------------------------------
assign rgb_out_upper = |rd_sel ? frame_data_upper : '0;
assign rgb_out_lower = |rd_sel ? frame_data_lower : '0;

endmodule
