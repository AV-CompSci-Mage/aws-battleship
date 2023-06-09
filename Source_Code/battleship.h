// This is an implementation of the board game Battleship.
// this is coded in C using a CC3200 microcontroller.
#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <stdint.h>

enum GamePhase { STARTUP, SHIP_PLACEMENT, PLAYER_1_TURN, PLAYER_2_TURN, GAME_OVER };
enum TileState { EMPTY, SHIP_TILE, HIT_TILE, MISS_TILE };
enum ShipType { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, PATROL_BOAT, SHIP_NONE };
enum ShipOrientation { HORIZONTAL, VERTICAL };
enum ShotResult { UNKNOWN_SHOT, MISS_SHOT, HIT_SHOT, INVALID_SHOT, SUNK_SHOT };

enum InputState { NONE, UP, DOWN, LEFT, RIGHT, SELECT, BACK, ROTATE };

struct Position {
    int x;
    int y;
};

struct Ship {
    enum ShipType type;
    int length;
    int hits;
    enum ShipOrientation orientation;
    // this is the position of the top left of the ship
    struct Position position;
    unsigned char is_sunk;
};

struct Tile {
    enum TileState state;
    struct Ship* ship;
};


struct GameState {
    enum GamePhase game_phase;
    unsigned char ship_placement_done;
    unsigned char shot_fired;
    struct Position shot_location;
    enum ShotResult shot_result;
    enum ShipType ship_sunk;
    struct Position ship_sunk_location[5];
    unsigned char defeated;
    unsigned char ready_to_transition;
};


//* global variables *//
#define BOARD_SIZE 8
#define NUM_SHIPS 5


extern struct GameState game_state;
extern struct GameState opponents_game_state;
extern struct Tile board[BOARD_SIZE][BOARD_SIZE];
extern enum ShotResult opponents_board[BOARD_SIZE][BOARD_SIZE];
extern struct Ship ships[NUM_SHIPS];
extern unsigned char is_phase_transition;
extern unsigned char need_to_draw;
extern unsigned char polling_allowed;

extern int ship_lengths[NUM_SHIPS];
extern int PLAYER_NUM;

extern struct Position cursor_position;
extern int cursor_ship_index;

extern enum InputState input_state;
extern unsigned char new_input;

extern int num_ships_sunk;
extern int ship_placement_ready;

//* AWS Communication Definitions *//
#define MAX_RECV_BUFF_SIZE  8192

extern unsigned char message_received;
extern unsigned char message_to_send;

extern int opponent_status_version; // used to track when opponent's board is updated
extern int our_status_version; // used to track when our board is updated

extern char message_buff[MAX_RECV_BUFF_SIZE + 1];


//* Draw Definitions *//
#define X_MAX 128
#define Y_MAX 128
#define GRID_SIZE 128 / BOARD_SIZE
#define GRID_DOT_SIZE 2
#define GRID_OFFSET GRID_SIZE / 2

// 2 byte color definitions
// #define WHITE 0xFFFF
// #define BLACK 0x0000
// #define BLUE  0x001F
// #define RED   0xF800
// #define GREEN 0x07E0
#define GREY  0x8410
#define DARK_GREY 0x4208
#define LIGHT_GREY 0xC618
#define DARKER_GREY 0x2104
#define NEAR_BLACK 0x0841

#define EMPTY_COLOR DARKER_GREY
#define SHIP_COLOR 0xFFFF
#define SHIP_PLACEMENT_COLOR 0xFFE0 // yellow
#define SHIP_PLACEMENT_ERROR_COLOR 0xF800 // red
#define HIT_COLOR 0xFA00 // red
#define MISS_COLOR 0xFFFF // white
#define SUNK_COLOR  0x8800 //dark red
#define CURSOR_COLOR 0x001F // blue
#define CURSOR_RADIUS 2

#define SHIP_MARGIN 2


#define TURN_DELAY 30000000


//* functions *//
// primary game functions
extern void init_game();
extern void draw_game();
extern void update_game();

//* Input functions *//
// used to receive input from the user
extern void pass_input(enum InputState input);

//* AWS Communication functions *//
// used for inter-board communication
extern void receive_message();
extern void send_message();

extern void indicate_ready_to_transition();
extern void shoot_at_opponent(struct Position position);
extern enum ShotResult receive_shot(struct Position position);
extern void indicate_sunken_ship(struct Ship* ship);
extern void indicate_victory();

//* Phases *//
// transition function
extern void transition_phase(enum GamePhase next_phase);

// startup phase
extern void draw_startup_phase();
extern void update_startup_phase();

// ship placement phase
extern void draw_ship_placement_phase();
extern void update_ship_placement_phase();

// your turn phase
extern void draw_your_turn_phase();
extern void update_your_turn_phase();

// opponent's turn phase
extern void draw_opponents_turn_phase();
extern void update_opponents_turn_phase();

// game over phase
extern void draw_game_over_phase();
extern void update_game_over_phase();

//* helper functions *//

// returns 1 if the position is within the board, 0 otherwise
extern unsigned char is_valid_position(struct Position position);

// returns a shotResult based on the position on the board
extern enum ShotResult process_shot(struct Position position);

// draws all placed ships on the board
extern void draw_ships();

// draws the ship on the board
extern void draw_ship(struct Ship* ship, unsigned int color);

// checks if the position is in bounds and that it will not collide with another ship
extern unsigned char can_be_placed_here(struct Ship* ship, struct Position position);

// places the ship on the board, updates the board matrix, updates the ship's position
extern void place_ship(struct Ship* ship, struct Position position);

// update cursor position
extern void move_cursor(int dx, int dy);

// draws the cursor

extern void draw_cursor();

extern void draw_ship_at_cursor(struct Ship* ship);

extern void draw_our_board_dots();
extern void draw_opponents_board_dots();

extern void draw_message_box(const char* message);



#endif // BATTLESHIP_H
