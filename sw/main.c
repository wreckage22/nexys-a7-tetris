#include <stdint.h>
#include <xparameters.h>

// Hardware mapping
#ifndef BRAM_BASE_ADDR
#define BRAM_BASE_ADDR XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR
#endif

volatile uint32_t *hardware_grid = (volatile uint32_t *) BRAM_BASE_ADDR;

// GPIO button mapping
#ifndef GPIO_BTNS_BASE_ADDR
#define GPIO_BTNS_BASE_ADDR XPAR_AXI_GPIO_0_BASEADDR
#endif

volatile uint32_t *gpio_btns = (volatile uint32_t *) GPIO_BTNS_BASE_ADDR;

// Push buttons BitMasks (push_button[3:0])
#define BTN_RIGHT 	(1 << 0) // bit 0
#define BTN_LEFT 	(1 << 1) // bit 1
#define BTN_DOWN 	(1 << 2) // bit 2
#define BTN_UP		(1 << 3) // bit 3

// Button holding timing thresholds (measured in debounce ticks)
#define DAS_TICKS 1000  // Ticks before auto-repeat starts
#define ARR_TICKS 200   // Ticks between repeated movements

// Game Board Gemoetry
#define BOARD_COLS	10
#define BOARD_ROWS	20

// timing parameters
#define DROP_DELAY_CYCLES 500000
#define BUTTON_DEBOUNCE_CYCLES 500


uint8_t playfield[BOARD_ROWS][BOARD_COLS] = {0};
uint8_t shadow_buffer[BOARD_ROWS][BOARD_COLS] = {[0 ... BOARD_ROWS-1][0 ... BOARD_COLS-1] = 255};


typedef struct {
    int x;         // which column the piece's top-left corner is at (0-9)
    int y;         // which row the piece's top-left corner is at (0-19)
    int type;      // which piece it is (0-6)
    int rotation;  // which rotation state it's in (0-3)
} Tetromino;

Tetromino current_piece;

const uint8_t TETROMINO_SHAPES[7][4][4][4] = {
	    // 1: I-Piece
		{{{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}}, {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}},
	    // 2: O-Piece
	    {{{0,2,2,0}, {0,2,2,0}, {0,0,0,0}, {0,0,0,0}}, {{0,2,2,0}, {0,2,2,0}, {0,0,0,0}, {0,0,0,0}}, {{0,2,2,0}, {0,2,2,0}, {0,0,0,0}, {0,0,0,0}}, {{0,2,2,0}, {0,2,2,0}, {0,0,0,0}, {0,0,0,0}}},
	    // 3: T-Piece
	    {{{0,3,0,0}, {3,3,3,0}, {0,0,0,0}, {0,0,0,0}}, {{0,3,0,0}, {0,3,3,0}, {0,3,0,0}, {0,0,0,0}}, {{0,0,0,0}, {3,3,3,0}, {0,3,0,0}, {0,0,0,0}}, {{0,3,0,0}, {3,3,0,0}, {0,3,0,0}, {0,0,0,0}}},
	    // 4: L-Piece
	    {{{0,0,4,0}, {4,4,4,0}, {0,0,0,0}, {0,0,0,0}}, {{0,4,0,0}, {0,4,0,0}, {0,4,4,0}, {0,0,0,0}}, {{0,0,0,0}, {4,4,4,0}, {4,0,0,0}, {0,0,0,0}}, {{4,4,0,0}, {0,4,0,0}, {0,4,0,0}, {0,0,0,0}}},
	    // 5: J-Piece
	    {{{5,0,0,0}, {5,5,5,0}, {0,0,0,0}, {0,0,0,0}}, {{0,5,5,0}, {0,5,0,0}, {0,5,0,0}, {0,0,0,0}}, {{0,0,0,0}, {5,5,5,0}, {0,0,5,0}, {0,0,0,0}}, {{0,5,0,0}, {0,5,0,0}, {5,5,0,0}, {0,0,0,0}}},
	    // 6: S-Piece
	    {{{0,6,6,0}, {6,6,0,0}, {0,0,0,0}, {0,0,0,0}}, {{0,6,0,0}, {0,6,6,0}, {0,0,6,0}, {0,0,0,0}}, {{0,0,0,0}, {0,6,6,0}, {6,6,0,0}, {0,0,0,0}}, {{6,0,0,0}, {6,6,0,0}, {0,6,0,0}, {0,0,0,0}}},
	    // 7: Z-Piece
	    {{{7,7,0,0}, {0,7,7,0}, {0,0,0,0}, {0,0,0,0}}, {{0,0,7,0}, {0,7,7,0}, {0,7,0,0}, {0,0,0,0}}, {{0,0,0,0}, {7,7,0,0}, {0,7,7,0}, {0,0,0,0}}, {{0,7,0,0}, {7,7,0,0}, {7,0,0,0}, {0,0,0,0}}}

};

// random number generator, Linear Feedback Shift Register (LFSR)
uint16_t lfsr = 0xACE1; // non-zero seed

uint8_t lfsr_next() {
    // 16-bit LFSR with taps at positions 16, 14, 13, 11
    uint16_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
    lfsr = (lfsr >> 1) | (bit << 15);
    return (lfsr % 7); // returns 0-6
}


void spawn_piece(){
	current_piece.x = 3;
	current_piece.y = 0;
	current_piece.rotation = 0;
	current_piece.type = lfsr_next();
}

int check_collision (int next_x, int next_y, int next_rot){
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 4; c++){
			if(TETROMINO_SHAPES[current_piece.type][next_rot][r][c] != 0){
				int board_x = next_x + c;
				int board_y = next_y + r;
				if(board_x < 0 || board_y < 0 || board_x >= BOARD_COLS || board_y >= BOARD_ROWS) return 1;
				if(playfield[board_y][board_x] != 0) return 1;
			}
		}
	}
	return 0;
}

void lock_piece(){
	for(int r = 0; r < 4; r++){
		for(int c = 0; c < 4; c++){
			int cell_value = TETROMINO_SHAPES[current_piece.type][current_piece.rotation][r][c];
			if(cell_value != 0){
				int board_x = current_piece.x + c;
				int board_y = current_piece.y + r;
				if(board_y >= 0 && board_y < BOARD_ROWS && board_x >= 0 && board_x < BOARD_COLS)
					playfield[board_y][board_x] = cell_value;
			}
		}
	}
}

void check_line_clears(){
	for(int r = BOARD_ROWS - 1; r >= 0; r--){
		int is_full = 1;
		for(int c = 0; c < BOARD_COLS; c++){
			if(playfield[r][c] == 0) {is_full = 0; break;}
		}
		if(is_full){
			for(int drop_r = r; drop_r > 0; drop_r--){
				for(int c = 0; c < BOARD_COLS; c++){
					playfield[drop_r][c] = playfield[drop_r - 1][c];
				}
			}
			for(int c = 0; c < BOARD_COLS; c++)playfield[0][c] = 0;
			r++;
		}
	}
}

void render_frame() {
	for(int r = 0; r < BOARD_ROWS; r++){
		for(int c = 0; c < BOARD_COLS; c++){
			uint8_t final_id = playfield[r][c];

            // Overlay the moving piece
			int piece_r = r - current_piece.y;
			int piece_c = c - current_piece.x;
			if(piece_r >= 0 && piece_r < 4 && piece_c >= 0 && piece_c < 4){
				uint8_t active_pixel = TETROMINO_SHAPES[current_piece.type][current_piece.rotation][piece_r][piece_c];
				if (active_pixel != 0) final_id = active_pixel;
			}

			// DELTA RENDER: Only write to hardware BRAM if the tile actually changed
			if (final_id != shadow_buffer[r][c]) {
				hardware_grid[(r * BOARD_COLS) + c] = final_id;
				shadow_buffer[r][c] = final_id;
			}
		}
	}
}

#define DISPLAY_COLS 12
#define DISPLAY_ROWS 22

/* would currupt the current game layout
void render_perimeter() {
    for (int r = 0; r < DISPLAY_ROWS; r++) {
        for (int c = 0; c < DISPLAY_COLS; c++) {
            // Check if we are on the outer edge (top, bottom, left, or right border)
            if (r == 0 || r == DISPLAY_ROWS - 1 || c == 0 || c == DISPLAY_COLS - 1) {
                // Write a static border ID (e.g., color/type 8) to BRAM
                hardware_grid[(r * DISPLAY_COLS) + c] = 8;
            }
        }
    }
}
*/

int main() {
	gpio_btns[1] = 0xFFFFFFFF; // set GPIO channel 1 as input

    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) playfield[r][c] = 0;
    }
	spawn_piece();
	volatile uint32_t gravity_counter = 0;
	volatile uint32_t btn_counter = 0;

	uint32_t prev_btn_state = 0;
	uint8_t needs_render = 1;

	uint32_t left_held_ticks = 0, right_held_ticks = 0, down_held_ticks = 0;
	uint8_t left_repeating = 0, right_repeating = 0, down_repeating = 0;

	while(1) { // grand loop
		if (needs_render){
			render_frame();
			needs_render = 0;
		}

		// button polling (rising edge)
		btn_counter++;
		if(btn_counter > BUTTON_DEBOUNCE_CYCLES){
			btn_counter = 0;
			uint32_t current_btn_state = *gpio_btns;
			uint32_t btn_pressed = current_btn_state & ~prev_btn_state;

			// --- LEFT BUTTON LOGIC ---
			if (current_btn_state & BTN_LEFT) {
				if (btn_pressed & BTN_LEFT) {
					// 1. Initial press
					if(!check_collision(current_piece.x - 1, current_piece.y, current_piece.rotation)){
						current_piece.x--;
						needs_render = 1;
					}
					left_held_ticks = 0;
					left_repeating = 0;
				} else {
					// 2. Button is being held
					left_held_ticks++;
					if (!left_repeating && left_held_ticks >= DAS_TICKS) {
						// Reached DAS threshold, start repeating
						left_repeating = 1;
						left_held_ticks = 0;
					} else if (left_repeating && left_held_ticks >= ARR_TICKS) {
						// 3. Fire ARR repeated movement
						if(!check_collision(current_piece.x - 1, current_piece.y, current_piece.rotation)){
							current_piece.x--;
							needs_render = 1;
						}
						left_held_ticks = 0;
					}
				}
			} else {
				// Reset state when button is released
				left_held_ticks = 0;
				left_repeating = 0;
			}

			// --- RIGHT BUTTON LOGIC ---
			if (current_btn_state & BTN_RIGHT) {
				if (btn_pressed & BTN_RIGHT) {
					if(!check_collision(current_piece.x + 1, current_piece.y, current_piece.rotation)){
						current_piece.x++;
						needs_render = 1;
					}
					right_held_ticks = 0;
					right_repeating = 0;
				} else {
					right_held_ticks++;
					if (!right_repeating && right_held_ticks >= DAS_TICKS) {
						right_repeating = 1;
						right_held_ticks = 0;
					} else if (right_repeating && right_held_ticks >= ARR_TICKS) {
						if(!check_collision(current_piece.x + 1, current_piece.y, current_piece.rotation)){
							current_piece.x++;
							needs_render = 1;
						}
						right_held_ticks = 0;
					}
				}
			} else {
				right_held_ticks = 0;
				right_repeating = 0;
			}

			// --- DOWN BUTTON LOGIC (Soft Drop) ---
			if (current_btn_state & BTN_DOWN) {
				if (btn_pressed & BTN_DOWN) {
					if(!check_collision(current_piece.x, current_piece.y + 1, current_piece.rotation)){
						current_piece.y++;
						needs_render = 1;
					}
					down_held_ticks = 0;
					down_repeating = 0;
				} else {
					down_held_ticks++;
					if (!down_repeating && down_held_ticks >= DAS_TICKS) {
						down_repeating = 1;
						down_held_ticks = 0;
					} else if (down_repeating && down_held_ticks >= (ARR_TICKS / 2)) {
						// ARR is usually faster for soft-dropping
						if(!check_collision(current_piece.x, current_piece.y + 1, current_piece.rotation)){
							current_piece.y++;
							needs_render = 1;
						}
						down_held_ticks = 0;
					}
				}
			} else {
				down_held_ticks = 0;
				down_repeating = 0;
			}

			// --- UP BUTTON LOGIC (Rotate) ---
			// Rotation strictly stays on rising-edge only
			if(btn_pressed & BTN_UP){
				if(!check_collision(current_piece.x, current_piece.y, (current_piece.rotation + 1) % 4)){
					current_piece.rotation = (current_piece.rotation + 1) % 4;
					needs_render = 1;
				}
			}

			prev_btn_state = current_btn_state;
		}

		gravity_counter++;
		if(gravity_counter > DROP_DELAY_CYCLES){
			gravity_counter = 0;
			if(!check_collision(current_piece.x, current_piece.y + 1, current_piece.rotation)){
				current_piece.y++;
			}
			else {
				lock_piece();
				check_line_clears();
				spawn_piece();
			}
			if(check_collision(current_piece.x, current_piece.y, current_piece.rotation)){
				for(int r = 0; r < BOARD_ROWS; r++){
					for (int c = 0; c < BOARD_COLS; c++) playfield[r][c] = 0;
				}
			}
			needs_render = 1;
		}
	}
	return 0;
}

