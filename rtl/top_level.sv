`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/03/2026 02:33:22 PM
// Design Name: 
// Module Name: top_level
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


module top_level(
    input logic clk_100MHz,
    input logic [3:0] push_buttons,
    input logic reset,
    output logic [3:0] vga_red, vga_green, vga_blue,
    output logic hsync, vsync
    );
    
    logic pixel_clk;
    logic [31:0] ram_data_i;
    logic [31:0] ram_addr;
    logic ram_en;
    tetris_v2_block_wrapper my_tetris_block
       (.BRAM_PORTB_0_addr          (ram_addr),
        .BRAM_PORTB_0_clk           (pixel_clk),
        .BRAM_PORTB_0_din           (32'b0),        // read only, no data-in
        .BRAM_PORTB_0_dout          (ram_data_i),
        .BRAM_PORTB_0_en            (ram_en),
        .BRAM_PORTB_0_rst           (1'b0),
        .BRAM_PORTB_0_we            (4'b0),
        .clk_100MHz                 (clk_100MHz),
        .clk_out2_0                 (pixel_clk),
        .push_buttons               (push_buttons),
        .reset_rtl_0                (~reset)
        );
    
    
    
    vga_driver  my_vga_driver(
    .clk                            (pixel_clk),
    .ram_data                       (ram_data_i),
    .vga_red                        (vga_red), 
    .vga_green                      (vga_green), 
    .vga_blue                       (vga_blue),
    .hsync                          (hsync), 
    .vsync                          (vsync),
    .ram_en                         (ram_en), 
    .ram_addr                       (ram_addr)
    );
        
endmodule

