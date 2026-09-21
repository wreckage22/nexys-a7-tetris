# FPGA Hardware-Accelerated Tetris

A complete hardware/software co-design implementing Tetris on a Xilinx Artix-7 FPGA (Digilent Nexys A7). Features a MicroBlaze soft-core processor, a custom SystemVerilog VGA display pipeline, dual-port BRAM memory mapping, and software-level rendering optimizations.

---

## Technical Highlights

* **Hardware-Software Co-Design:** Integrated a 32-bit MicroBlaze soft-core processor with custom SystemVerilog RTL modules over AXI interconnects using Xilinx Vivado & Vitis.
* **Custom VGA Controller:** Built a 640x480 @ 60Hz display driver from scratch, implementing pipelined register stages to compensate for 1-cycle BRAM read latency.
* **Zero-Contention Memory Architecture:** Configured Dual-Port Block RAM (BRAM) to allow parallel access—Port A dedicated to MicroBlaze game updates, Port B dedicated to VGA frame scanning.
* **Performance Optimization:** Implemented **Delta Rendering (Shadow Buffering)** in C to eliminate AXI bus bottlenecking and screen tearing, cutting BRAM writes per frame from 200 down to active tile changes.
* **Input State Machine:** Designed DAS (Delayed Auto-Shift) and ARR (Auto-Repeat Rate) timing logic to handle rapid GPIO button polling smoothly.

---

## VGA Controller Specifications & Timing

* **Pixel Clock:** 25.2 MHz (generated via Clocking Wizard for 640x480 @ 60Hz VESA DMT standard).
* **Frame Dimensions:** 800 x 525 total pixel cycles including front/back porch and sync intervals.
* **Grid Mapping:** 10x20 game board where each tile maps to a 20x20 pixel display block.
* **Pipelining:** To align video sync signals (`hsync`, `vsync`, `active_video`) with BRAM's 1-clock latency, control paths are registered (`_d`) to eliminate boundary color bleeding and frame misalignment.

---

## Repository Structure

* `/rtl` — SystemVerilog sources (VGA controller, top-level wrapper) and constraints (`.xdc`).
* `/sw` — MicroBlaze C code (`main.c`, hardware header definitions).
* `/assets` — Block diagrams and board demonstration media.
