`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/03/2026 02:31:01 PM
// Design Name: 
// Module Name: vga_driver
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module vga_driver(
    input logic clk,      //25.clk
    input logic [31:0] ram_data,
    output logic [3:0] vga_red, 
    output logic [3:0] vga_green, 
    output logic [3:0] vga_blue,
    output logic hsync, vsync,
    output logic ram_en, 
    output logic [31:0] ram_addr
    );
    
    // 640x480 (pixel) @ 60Hz timing
    localparam H_VISIBLE    = 640;
    localparam H_F_PORCH    = 16;
    localparam H_SYNC       = 96;
    localparam H_B_PORCH    = 48;
    localparam H_TOTAL      = 800;
    
    localparam V_VISIBLE    = 480;
    localparam V_F_PORCH    = 10;
    localparam V_SYNC       = 2;
    localparam V_B_PORCH    = 33; 
    localparam V_TOTAL      = 525;   
    
    // Tetris board geometry 10x20 (block)
    localparam BOARD_COLS   = 10;
    localparam BOARD_ROWS   = 20;
    localparam BLOCK_SIZE   = 20; //20 pixels per block width
    localparam BOARD_TOP    = 40;
    localparam BOARD_BOTTOM = BOARD_TOP + (BOARD_ROWS * BLOCK_SIZE);
    localparam BOARD_LEFT   = 220;
    localparam BOARD_RIGHT  = BOARD_LEFT + (BLOCK_SIZE * BOARD_COLS);
    localparam BORDER_SIZE = 10;
    
    logic [9:0] h_count = 0;
    logic [9:0] v_count = 0;
    logic on_video;
    logic on_board;
    logic on_border;
    logic on_border_d;
    
    //counters
    always_ff @(posedge clk) begin
        if(h_count == H_TOTAL - 1) begin 
            h_count <= 0;
            if(v_count == V_TOTAL - 1) begin
                v_count <= 0;
            end else begin 
                v_count <= v_count + 1;
            end  
        end else begin
            h_count <= h_count + 1;
        end
    end
    
    // on visible display or game display or other
    assign on_video = (h_count < H_VISIBLE) && (v_count < V_VISIBLE);
    assign on_board = on_video && 
                                    (h_count >= BOARD_LEFT) && (h_count < BOARD_RIGHT) && 
                                    (v_count >= BOARD_TOP)  && (v_count < BOARD_BOTTOM);
    assign on_border = on_video && !on_board &&
                   (h_count >= (BOARD_LEFT  - BORDER_SIZE)) &&
                   (h_count <  (BOARD_RIGHT + BORDER_SIZE)) &&
                   (v_count >= (BOARD_TOP   - BORDER_SIZE)) &&
                   (v_count <  (BOARD_BOTTOM + BORDER_SIZE));
    
    // game grid and addressing 
    logic [4:0] tile_col; // 5 bits for col index 0-9
    logic [5:0] tile_row; // 6 vits for row index 0-19 
    logic [9:0] tile_index; // 9 bits for tile index 0-199
    
    assign tile_col = h_count >= BOARD_LEFT ? (h_count - BOARD_LEFT) / BLOCK_SIZE : 5'b0;
    assign tile_row = v_count >= BOARD_TOP  ? (v_count - BOARD_TOP) / BLOCK_SIZE : 6'b0;
    assign tile_index = (tile_row * BOARD_COLS) + tile_col;
   
    
    assign ram_en = 1'b1; // always enabled i guess idk 
    
    // driving signals and delaying signals 
    logic on_board_d;
    logic on_video_d;
    
    always_ff @(posedge clk) begin
        if(on_board)    ram_addr <= {20'b0, tile_index, 2'b0};
        else            ram_addr <= 32'b0;
        
        hsync <= (h_count >= (H_VISIBLE + H_F_PORCH)) && (h_count < (H_VISIBLE + H_F_PORCH + H_SYNC)) ? 1'b0 : 1'b1;
        vsync <= (v_count >= (V_VISIBLE + V_F_PORCH)) && (v_count < (V_VISIBLE + V_F_PORCH + V_SYNC)) ? 1'b0 : 1'b1;
        on_board_d <= on_board; // delayed to match the 1-cycle BRAM read latency 
        on_video_d <= on_video;
        on_border_d <= on_border;
    end
    
    always_comb begin 
        if(!on_video_d) begin
            vga_red = 4'h0; vga_green = 4'h0; vga_blue = 4'h0;
        end else if(on_border_d) begin
            vga_red = 4'hF; vga_green = 4'hF; vga_blue = 4'hF; // white border
        end else if(!on_board_d) begin
            vga_red = 4'h0; vga_green = 4'h0; vga_blue = 4'h0;
        end else begin  
            case (ram_data[3:0])
                4'h0: begin vga_red = 4'h0; vga_green = 4'h0; vga_blue = 4'h0; end
                4'h1: begin vga_red = 4'h0; vga_green = 4'hF; vga_blue = 4'hF; end
                4'h2: begin vga_red = 4'hF; vga_green = 4'hF; vga_blue = 4'h0; end
                4'h3: begin vga_red = 4'hA; vga_green = 4'h0; vga_blue = 4'hF; end
                4'h4: begin vga_red = 4'hF; vga_green = 4'h7; vga_blue = 4'h0; end
                4'h5: begin vga_red = 4'h0; vga_green = 4'h0; vga_blue = 4'hF; end
                4'h6: begin vga_red = 4'h0; vga_green = 4'hF; vga_blue = 4'h0; end
                4'h7: begin vga_red = 4'hF; vga_green = 4'h0; vga_blue = 4'h0; end
                default: begin vga_red = 4'hF; vga_green = 4'hF; vga_blue = 4'hF; end
            endcase 
        end
    end
               
endmodule

