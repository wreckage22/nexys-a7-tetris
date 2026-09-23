//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2022.1 (lin64) Build 3526262 Mon Apr 18 15:47:01 MDT 2022
//Date        : Sun Sep 20 15:23:15 2026
//Host        : edalinux17 running 64-bit Rocky Linux release 8.10 (Green Obsidian)
//Command     : generate_target tetris_v2_block_wrapper.bd
//Design      : tetris_v2_block_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module tetris_v2_block_wrapper
   (BRAM_PORTB_0_addr,
    BRAM_PORTB_0_clk,
    BRAM_PORTB_0_din,
    BRAM_PORTB_0_dout,
    BRAM_PORTB_0_en,
    BRAM_PORTB_0_rst,
    BRAM_PORTB_0_we,
    clk_100MHz,
    clk_out2_0,
    push_buttons,
    reset_rtl_0);
  input [31:0]BRAM_PORTB_0_addr;
  input BRAM_PORTB_0_clk;
  input [31:0]BRAM_PORTB_0_din;
  output [31:0]BRAM_PORTB_0_dout;
  input BRAM_PORTB_0_en;
  input BRAM_PORTB_0_rst;
  input [3:0]BRAM_PORTB_0_we;
  input clk_100MHz;
  output clk_out2_0;
  input [3:0]push_buttons;
  input reset_rtl_0;

  wire [31:0]BRAM_PORTB_0_addr;
  wire BRAM_PORTB_0_clk;
  wire [31:0]BRAM_PORTB_0_din;
  wire [31:0]BRAM_PORTB_0_dout;
  wire BRAM_PORTB_0_en;
  wire BRAM_PORTB_0_rst;
  wire [3:0]BRAM_PORTB_0_we;
  wire clk_100MHz;
  wire clk_out2_0;
  wire [3:0]push_buttons;
  wire reset_rtl_0;

  tetris_v2_block tetris_v2_block_i
       (.BRAM_PORTB_0_addr(BRAM_PORTB_0_addr),
        .BRAM_PORTB_0_clk(BRAM_PORTB_0_clk),
        .BRAM_PORTB_0_din(BRAM_PORTB_0_din),
        .BRAM_PORTB_0_dout(BRAM_PORTB_0_dout),
        .BRAM_PORTB_0_en(BRAM_PORTB_0_en),
        .BRAM_PORTB_0_rst(BRAM_PORTB_0_rst),
        .BRAM_PORTB_0_we(BRAM_PORTB_0_we),
        .clk_100MHz(clk_100MHz),
        .clk_out2_0(clk_out2_0),
        .push_buttons(push_buttons),
        .reset_rtl_0(reset_rtl_0));
endmodule
