#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "prcm.h"
#include "utils.h"
#include "systick.h"
#include "rom_map.h"
#include "uart.h"
#include "gpio.h"
#include "timer_if.h"
#include "timer.h"


#include "battleship.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"
#include "glcdfont.h"
#include "OLED_Utils.h"


#define PLAYER 2

// global variables
struct GameState game_state = 
{ 
    STARTUP, 
    0, 
    0, 
    {0, 0}, 
    UNKNOWN_SHOT, 
    SHIP_NONE, 
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}, 
    0 
};

struct GameState opponents_game_state = 
{ 
    STARTUP, 
    0, 
    0, 
    {0, 0}, 
    UNKNOWN_SHOT, 
    SHIP_NONE, 
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}, 
    0 
};

struct Tile board[BOARD_SIZE][BOARD_SIZE];
enum ShotResult opponents_board[BOARD_SIZE][BOARD_SIZE];
struct Ship ships[NUM_SHIPS];
unsigned char is_phase_transition = 0;
unsigned char is_update_phase_transition = 0;
unsigned char need_to_draw = 0;
unsigned char polling_allowed = 0;

int ship_lengths[NUM_SHIPS] = { 2, 3, 3, 4, 5 };
int PLAYER_NUM = 1;

struct Position cursor_position = { 0, 0 };
int cursor_ship_index = 0;

enum InputState input_state = NONE;
unsigned char new_input = 0; 

int num_ships_sunk = 0;
int ship_placement_ready = 0;

unsigned char message_received = 0;
unsigned char message_to_send = 0;

int opponent_status_version = 0;
int our_status_version = 0;

char message_buff[MAX_RECV_BUFF_SIZE + 1];


const char* GamePhase_strings[] = {"STARTUP", "SHIP_PLACEMENT", "PLAYER_1_TURN", "PLAYER_2_TURN", "GAME_OVER"};
const char* ShotResult_strings[] = {"UNKNOWN_SHOT", "MISS_SHOT", "HIT_SHOT", "INVALID_SHOT", "SUNK_SHOT"};
const char* ShipType_strings[] = {"CARRIER", "BATTLESHIP", "CRUISER", "SUBMARINE", "PATROL_BOAT", "NONE"};

// primary game functions
void init_game() {
    // initialize the game state
    transition_phase(STARTUP);
    int i = 0;
    int j = 0;
    // initialize the board
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            struct Tile tile = { EMPTY, 0 };
            board[i][j] = tile;
        }
    }

    // initialize the ships
    for (i = 0; i < NUM_SHIPS; i++) {
        ships[i].type = NUM_SHIPS - i - 1;
        int length = 0;
        switch(ships[i].type){
            case CARRIER:
                length = 5;
                break;
            case BATTLESHIP:
                length = 4;
                break;
            case CRUISER: 
                length = 3;
                break;
            case SUBMARINE:
                length = 3;
                break;
            case PATROL_BOAT:
                length = 2;
                break;
        }
        ships[i].length = length;
        ships[i].hits = 0;
        ships[i].orientation = HORIZONTAL;
        ships[i].position.x = -1;
        ships[i].position.y = -1;
        ships[i].is_sunk = 0;
    }

    // reset the shot board
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            opponents_board[i][j] = UNKNOWN_SHOT;
        }
    }

    // initialize the player number
    PLAYER_NUM = PLAYER;     //TODO: arbitrate for player number with opponent
    num_ships_sunk = 0;
    cursor_ship_index = NUM_SHIPS - 1;
    ship_placement_ready = 0;

    game_state = (struct GameState) { STARTUP, 0, 0, {0, 0}, UNKNOWN_SHOT, SHIP_NONE, {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}, 0 };

    message_to_send = 1;
}

void draw_game() {
    switch (game_state.game_phase) {
        case STARTUP:
            draw_startup_phase();
            break;
        case SHIP_PLACEMENT:
            draw_ship_placement_phase();
            break;
        case PLAYER_1_TURN:
            if(PLAYER_NUM == 1)
                draw_your_turn_phase();
            else
                draw_opponents_turn_phase();
            break;
        case PLAYER_2_TURN:
            if(PLAYER_NUM == 2)
                draw_your_turn_phase();
            else
                draw_opponents_turn_phase();
            break;
        case GAME_OVER:
            draw_game_over_phase();
            break;
    }
}

void update_game() {
    switch (game_state.game_phase) {
        case STARTUP:
            update_startup_phase();
            break;
        case SHIP_PLACEMENT:
            update_ship_placement_phase();
            break;
        case PLAYER_1_TURN:
            if(PLAYER_NUM == 1)
                update_your_turn_phase();
            else
                update_opponents_turn_phase();
            break;
        case PLAYER_2_TURN:
            if(PLAYER_NUM == 2)
                update_your_turn_phase();
            else
                update_opponents_turn_phase();
            break;
        case GAME_OVER:
            update_game_over_phase();
            break;
    }
}

// Input functions
void pass_input(enum InputState input) {
    input_state = input;
    new_input = 1;
}



void receive_message(){
    // TODO: parse message and update game state
    

}

const char* json_format_string = "{\"state\":{\"desired\":{\"%s\":{\"version\": %d,\"current_phase\": \"%s\",\"ship_placement_done\": \"%s\",\"shot_fired\": \"%s\",\"shot_location\": [%d,%d],\"shot_result\": \"%s\",\"ship_sunk\": \"%s\",\"ship_sunk_locations\": [[%d,%d],[%d,%d],[%d,%d],[%d,%d],[%d,%d]],\"defeated\": \"%s\"}}}}\0";

void send_message(){
    sprintf(message_buff, 
    json_format_string, PLAYER == 1 ? "player1" : "player2", our_status_version++, 
    GamePhase_strings[game_state.game_phase], 
    game_state.ship_placement_done ? "true" : "false", 
    game_state.shot_fired ? "true" : "false",
    game_state.shot_location.x, game_state.shot_location.y, 
    ShotResult_strings[game_state.shot_result],
    ShipType_strings[game_state.ship_sunk], 
    game_state.ship_sunk_location[0].x, game_state.ship_sunk_location[0].y,
    game_state.ship_sunk_location[1].x, game_state.ship_sunk_location[1].y,
    game_state.ship_sunk_location[2].x, game_state.ship_sunk_location[2].y,
    game_state.ship_sunk_location[3].x, game_state.ship_sunk_location[3].y,
    game_state.ship_sunk_location[4].x, game_state.ship_sunk_location[4].y,
    game_state.defeated ? "true" : "false"
    );
    Report("Sending message: %s\n", message_buff);
}

void indicate_ready_to_transition(){
    game_state.ready_to_transition = 1;
    message_to_send = 1;
}

void shoot_at_opponent(struct Position position){
    game_state.shot_fired = 1;
    game_state.shot_location = position;
    message_to_send = 1;
}

enum ShotResult receive_shot(struct Position position){
    game_state.shot_result = process_shot(position);
    Report("Shot result: %s\n", ShotResult_strings[game_state.shot_result]);
    if(num_ships_sunk >= NUM_SHIPS){
        indicate_victory();
    }
    message_to_send = 1;
    return game_state.shot_result;
}

void indicate_sunken_ship(struct Ship* ship){
    game_state.ship_sunk = ship->type;
    int i = 0;
    if(ship->orientation == HORIZONTAL){
        for(i = 0; i < ship->length; i++){
        game_state.ship_sunk_location[i] = (struct Position){ship->position.x + i, ship->position.y};
        }
    } else {
        for(i = 0; i < ship->length; i++){
        game_state.ship_sunk_location[i] = (struct Position){ship->position.x, ship->position.y + i};
        }
    }
    message_to_send = 1;
}


void indicate_victory(){
    game_state.defeated = 1;
    message_to_send = 1;
    transition_phase(GAME_OVER);
}


// Phases
void transition_phase(enum GamePhase next_phase) {
    game_state.game_phase = next_phase;
    message_to_send = 1;
    is_phase_transition = 1;
    is_update_phase_transition = 1;
}

// startup phase
void draw_startup_phase() {
    if(is_phase_transition) {
        fillScreen(BLACK);
        draw_message_box("Connecting...");
        if(PLAYER == 1){
            drawString("Player 1", 0, 8, WHITE, BLACK, 1);
        } else {
            drawString("Player 2", 0, 8, WHITE, BLACK, 1);
        }
        is_phase_transition = 0; // reset the transition flag
    }
}

void update_startup_phase() {
    if(new_input){
        new_input = 0;
        // we don't do anything with inputs during the startup phase
    }
}

unsigned char is_local_placement_over = 0;

// ship placement phase
void draw_ship_placement_phase() {
    if(is_phase_transition) {
        need_to_draw = 1;
        is_phase_transition = 0; // reset the transition flag
    }
    if(need_to_draw){
        if(is_local_placement_over){
            draw_ships();
            // draw a black square in the middle of the screen
            fillRect(0, Y_MAX/2 - 16, X_MAX, 24, BLACK);
            drawRect(0, Y_MAX/2 - 16, X_MAX, 24, WHITE);

            // center the text in the middle of the screen
            draw_message_box("Waiting for opponent");
            need_to_draw = 0;
            return;
        }
        fillScreen(BLACK);
        draw_our_board_dots();
        draw_ships();
        struct Ship* ship = &ships[cursor_ship_index];
        draw_ship_at_cursor(ship);
        draw_cursor();

        need_to_draw = 0;
    }
}

void update_ship_placement_phase() {
    struct Ship* ship = &ships[cursor_ship_index];
    if(is_update_phase_transition) {
        is_local_placement_over = 0;
        is_update_phase_transition = 0;
        polling_allowed = 0;
    }
    if(new_input){
        if(!is_local_placement_over){
            // move the cursor based on the input
            if(input_state == LEFT){
                move_cursor(-1, 0);
                need_to_draw = 1;
            } else if(input_state == RIGHT){
                move_cursor(1, 0);
                need_to_draw = 1;
            } else if(input_state == UP){
                move_cursor(0, -1);
                need_to_draw = 1;
            } else if(input_state == DOWN){
                move_cursor(0, 1);
                need_to_draw = 1;
            } else if (input_state == ROTATE){
                if(ship->orientation == HORIZONTAL){
                    ship->orientation = VERTICAL;
                } else {
                    ship->orientation = HORIZONTAL;
                }
                need_to_draw = 1;
            } else if(input_state == SELECT){
                if(can_be_placed_here(ship, cursor_position)){
                    place_ship(ship, cursor_position);
                    cursor_ship_index--;
                    if(cursor_ship_index == -1){
                        ship_placement_ready++;
                        is_local_placement_over = 1;
                        game_state.ship_placement_done = 1;
                        message_to_send = 1;
                        polling_allowed = 1;
                    }
                    need_to_draw = 1;
                }
            }
        }
        new_input = 0;
    }
    if(message_received){
        message_received = 0;
    }
    if(opponents_game_state.ship_placement_done && game_state.ship_placement_done){
        transition_phase(PLAYER_1_TURN);
    }
}

unsigned char has_shot = 0;

// your turn phase
void draw_your_turn_phase() {
    if(is_phase_transition) {
        fillScreen(BLACK);
        draw_opponents_board_dots();
        draw_cursor();
        draw_message_box("Your turn");
        is_phase_transition = 0; // reset the transition flag
    }
    if(need_to_draw){
        fillScreen(BLACK);
        draw_opponents_board_dots();
        draw_cursor();
        if(has_shot) {
            // draw a border around the whole screen indicating that we are waiting for the opponent
            drawRect(0,0,128,128,CYAN);
        }
        need_to_draw = 0;
    }
}

void update_your_turn_phase() {
    if(is_update_phase_transition) {
        has_shot = 0;
        cursor_position.x = 1;
        cursor_position.y = 1;
        game_state.shot_fired = 0;
        message_to_send = 1;
        game_state.ready_to_transition = 0;

        polling_allowed = 0;

        is_update_phase_transition = 0;
    }
    if(new_input){
        if(!has_shot){
        // move the cursor based on the input
            if(input_state == LEFT){
                move_cursor(-1, 0);
                need_to_draw = 1;
            } else if(input_state == RIGHT){
                move_cursor(1, 0);
                need_to_draw = 1;
            } else if(input_state == UP){
                move_cursor(0, -1);
                need_to_draw = 1;
            } else if(input_state == DOWN){
                move_cursor(0, 1);
                need_to_draw = 1;
            } else if(input_state == SELECT){
                shoot_at_opponent(cursor_position);
                need_to_draw = 1;
                has_shot = 1;
                polling_allowed = 1;
            }
        }
        new_input = 0;
    }
    if(message_received){
        if(opponents_game_state.defeated){
            transition_phase(GAME_OVER);
        } else {
            if(opponents_game_state.shot_result != UNKNOWN_SHOT){
                if(opponents_game_state.shot_result == INVALID_SHOT){
                    has_shot = 0;
                    need_to_draw = 1;
                    message_to_send = 1;
                    game_state.shot_fired = 0;
                    polling_allowed = 0;
                    draw_message_box("Invalid shot");
                } else {
                    opponents_board[game_state.shot_location.x][game_state.shot_location.  y] = opponents_game_state.shot_result;
                    indicate_ready_to_transition();
                    game_state.shot_fired = 0;
                }
            }
        }

        if(opponents_game_state.ship_sunk != SHIP_NONE){
            int l = 0;
            int length = 0;
            switch(opponents_game_state.ship_sunk){
                case CARRIER:
                    length = 5;
                    break;
                case BATTLESHIP:
                    length = 4;
                    break;
                case CRUISER:
                    length = 3;
                    break;
                case SUBMARINE:
                    length = 3;
                    break;
                case PATROL_BOAT:
                    length = 2;
                    break;
                default:
                    length = 2;
            }
            for(l = 0; l < length; l++){
                int x = opponents_game_state.ship_sunk_location[l].x;
                int y = opponents_game_state.ship_sunk_location[l].y;
                opponents_board[x][y] = SUNK_SHOT;
            }
        }
        message_received = 0;
    }
    if(game_state.ready_to_transition){
        fillScreen(BLACK);
        draw_opponents_board_dots();
        if(opponents_game_state.shot_result == HIT_SHOT){
            // draw a red rectangle around the whole board
            drawRect(0,0,128,128,RED);
        } else if (opponents_game_state.shot_result == MISS_SHOT){
            // draw a grey rectangle around the whole board
            drawRect(0,0,128,128,GREY);
        }
        if(opponents_game_state.ship_sunk != SHIP_NONE){
            char ship_sunk_string[64];
            sprintf(ship_sunk_string, "sunk %s!", ShipType_strings[opponents_game_state.ship_sunk]);
            draw_message_box(ship_sunk_string);
        }
        MAP_UtilsDelay(TURN_DELAY);
        if(game_state.game_phase == PLAYER_1_TURN ){
            transition_phase(PLAYER_2_TURN);
        } else {
            transition_phase(PLAYER_1_TURN);
        }
    }
}

// opponent's turn phase
void draw_opponents_turn_phase() {
    if(is_phase_transition) {
        fillScreen(BLACK);
        draw_ships();
        draw_our_board_dots();
        draw_message_box("Opponent's turn");
        is_phase_transition = 0; // reset the transition flag
    }
    if(need_to_draw){
        fillScreen(BLACK);
        draw_ships();
        draw_our_board_dots();
        need_to_draw = 0;
    }
}

void update_opponents_turn_phase() {
    if(is_update_phase_transition){
        game_state.ship_sunk = SHIP_NONE;
        game_state.shot_result = UNKNOWN_SHOT;
        message_to_send = 1;
        is_update_phase_transition = 0;
        polling_allowed = 1; // always allow polling in this phase
    }
    if(new_input){
        // we don't do shit
        new_input = 0;
    }
    // if message_received need to draw
    if(message_received){
        if(opponents_game_state.shot_fired){
            receive_shot(opponents_game_state.shot_location);
            indicate_ready_to_transition();
        }
        if(opponents_game_state.game_phase != game_state.game_phase){
            if(opponents_game_state.game_phase == GAME_OVER){
                transition_phase(GAME_OVER);
            } else {
                fillScreen(BLACK);
                draw_ships();
                draw_our_board_dots();
                if(game_state.shot_result == HIT_SHOT){
                    // draw a red rectangle around the whole board
                    drawRect(0,0,128,128,RED);
                } else if (game_state.shot_result == MISS_SHOT){
                    // draw a blue rectangle around the whole board
                    drawRect(0,0,128,128,GREY);
                }
                if(game_state.ship_sunk != SHIP_NONE){
                    char ship_sunk_string[64];
                    sprintf(ship_sunk_string, "%s sunk!", ShipType_strings[game_state.ship_sunk]);
                    draw_message_box(ship_sunk_string);
                }
                // delay
                MAP_UtilsDelay(TURN_DELAY);
                transition_phase(opponents_game_state.game_phase);
            }
        }
        message_received = 0;
    }
}

// game over phase
void draw_game_over_phase() {
    if(is_phase_transition) {
        fillScreen(BLACK);
        // draw the game over text (win or lose)
        if(game_state.defeated){
            draw_message_box("You Lose");
        } else {
            draw_message_box("You Win!");
        }
        is_phase_transition = 0; // reset the transition flag
    }
}

void update_game_over_phase() {
    if(is_update_phase_transition){
        polling_allowed = 0;
        is_update_phase_transition = 0;
    }
    if(new_input){
        // if any new input we reset the game and continue
        init_game();
        new_input = 0;
    }
}

// helper functions
unsigned char is_valid_position(struct Position position) {
    if (position.x < 0 || position.x >= BOARD_SIZE || position.y < 0 || position.y >= BOARD_SIZE) {
        return 0;
    }
    return 1;
}

// this returns the result of the shot on our board
enum ShotResult process_shot(struct Position position) {
    if (!is_valid_position(position)) {
        return INVALID_SHOT;
    }
     if (board[position.x][position.y].state == MISS_TILE || board[position.x][position.y].state == HIT_TILE) {
        return INVALID_SHOT;
    }
    if (board[position.x][position.y].state == EMPTY) {
        return MISS_SHOT;
    }
    else {
        board[position.x][position.y].state = HIT_TILE;
        if(board[position.x][position.y].ship != 0) {
            board[position.x][position.y].ship->hits++;
            if(board[position.x][position.y].ship->hits == board[position.x][position.y].ship->length){
                board[position.x][position.y].ship->is_sunk = 1;
                indicate_sunken_ship(board[position.x][position.y].ship);
            }
        }
        return HIT_SHOT;
    }
}

 void draw_ships(){
     int i = 0;
     for(i = 0; i < NUM_SHIPS; i++){
        if(!is_valid_position(ships[i].position)){
            continue;
        }
        if(ships[i].is_sunk){
            draw_ship(&ships[i], GREY);
        } else {
            draw_ship(&ships[i], SHIP_COLOR);
        }
    }
 }

void draw_ship(struct Ship* ship, unsigned int color){
    int i = 0;
    if(ship->orientation == HORIZONTAL){
        int left_corner_x = ship->position.x * GRID_SIZE + SHIP_MARGIN;
        int left_corner_y = ship->position.y * GRID_SIZE + SHIP_MARGIN;
        int width = ship->length * GRID_SIZE - SHIP_MARGIN * 2;
        int length = GRID_SIZE - SHIP_MARGIN * 2;
        fillRect(left_corner_x, left_corner_y, width, length, color);
    } else if(ship->orientation == VERTICAL){
        int left_corner_x = ship->position.x * GRID_SIZE + SHIP_MARGIN;
        int left_corner_y = ship->position.y * GRID_SIZE + SHIP_MARGIN;
        int width = GRID_SIZE - SHIP_MARGIN * 2;
        int length = ship->length * GRID_SIZE - SHIP_MARGIN * 2;
        fillRect(left_corner_x, left_corner_y, width, length, color);
    }
}

// checks if the position is in bounds and that it will not collide with another ship
unsigned char can_be_placed_here(struct Ship* ship, struct Position position){
    if(!is_valid_position(position)){
        return 0;
    }
    int i = 0;
    if(ship->orientation == HORIZONTAL){
        if(position.x + ship->length > BOARD_SIZE){
            return 0;
        }
        for(i = 0; i < ship->length; i++){
            if(board[position.x + i][position.y].state != EMPTY){
                return 0;
            }
        }
    } else if(ship->orientation == VERTICAL){
        if(position.y + ship->length > BOARD_SIZE){
            return 0;
        }
        for(i = 0; i < ship->length; i++){
            if(board[position.x][position.y + i].state != EMPTY){
                return 0;
            }
        }
    }
    return 1;
}

// places the ship on the board, updates the board matrix, updates the ship's position
void place_ship(struct Ship* ship, struct Position position){
    int i = 0;
    if(ship->orientation == HORIZONTAL){
        for(i = 0; i < ship->length; i++){
            board[position.x + i][position.y].state = SHIP_TILE;
            board[position.x + i][position.y].ship = ship;
        }
    } else if(ship->orientation == VERTICAL){
        for(i = 0; i < ship->length; i++){
            board[position.x][position.y + i].state = SHIP_TILE;
            board[position.x][position.y + i].ship = ship;
        }
    }
    ship->position = position;
}

// moves the cursor by dx and dy, clamps the cursor to the board
void move_cursor(int dx, int dy){
    cursor_position.x += dx;
    cursor_position.y += dy;
    if(cursor_position.x < 0){
        cursor_position.x = 0;
    } else if(cursor_position.x >= BOARD_SIZE){
        cursor_position.x = BOARD_SIZE - 1;
    }
    if(cursor_position.y < 0){
        cursor_position.y = 0;
    } else if(cursor_position.y >= BOARD_SIZE){
        cursor_position.y = BOARD_SIZE - 1;
    }
}

// draws the cursor on the OLED
void draw_cursor(){
    //TODO: use draw functions to draw the cursor on the OLED
    int cursor_center_x = cursor_position.x * GRID_SIZE + GRID_OFFSET;
    int cursor_center_y = cursor_position.y * GRID_SIZE + GRID_OFFSET;
    drawCircle(cursor_center_x, cursor_center_y, CURSOR_RADIUS, CURSOR_COLOR);
    // draw 4 lines radiating from the center of the cursor. They should extend twice the radius of the cursor
    drawLine(cursor_center_x, cursor_center_y, cursor_center_x + CURSOR_RADIUS * 2, cursor_center_y, CURSOR_COLOR);
    drawLine(cursor_center_x, cursor_center_y, cursor_center_x - CURSOR_RADIUS * 2, cursor_center_y, CURSOR_COLOR);
    drawLine(cursor_center_x, cursor_center_y, cursor_center_x, cursor_center_y + CURSOR_RADIUS * 2, CURSOR_COLOR);
    drawLine(cursor_center_x, cursor_center_y, cursor_center_x, cursor_center_y - CURSOR_RADIUS * 2, CURSOR_COLOR);
}


void draw_ship_at_cursor(struct Ship* ship){
    struct Ship temp_ship = *ship;
    temp_ship.position = cursor_position;
    if(can_be_placed_here(&temp_ship, cursor_position)){
        draw_ship(&temp_ship, SHIP_PLACEMENT_COLOR);
    } else {
        draw_ship(&temp_ship, SHIP_PLACEMENT_ERROR_COLOR);
    }
}


void draw_our_board_dots(){
    int i = 0;
    int j = 0;
    for(i = 0; i < BOARD_SIZE; i++){
        for(j = 0; j < BOARD_SIZE; j++){
            int dot_center_x = i * GRID_SIZE + GRID_OFFSET;
            int dot_center_y = j * GRID_SIZE + GRID_OFFSET;
            if(board[i][j].state == EMPTY){
                drawCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, EMPTY_COLOR);
            } else if (board[i][j].state == SHIP_TILE){
                drawCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, SHIP_COLOR);
            } else if (board[i][j].state == HIT_TILE){
                drawFilledCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, HIT_COLOR);
            } else if (board[i][j].state == MISS_TILE){
                drawFilledCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, MISS_COLOR);
            }
        }
    }
}

void draw_opponents_board_dots(){
    // now draw them with the correct color
    int i = 0;
    int j = 0;
    for(i = 0; i < BOARD_SIZE; i++){
        for(j = 0; j < BOARD_SIZE; j++){
            int dot_center_x = i * GRID_SIZE + GRID_OFFSET;
            int dot_center_y = j * GRID_SIZE + GRID_OFFSET;
            if(opponents_board[i][j] == UNKNOWN_SHOT){
                drawCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, EMPTY_COLOR);
            } else if(opponents_board[i][j] == MISS_SHOT){
                drawFilledCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, MISS_COLOR);
            } else if(opponents_board[i][j] == HIT_SHOT){
                drawFilledCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, HIT_COLOR);
            } else if(opponents_board[i][j] == SUNK_SHOT){
                drawCircle(dot_center_x, dot_center_y, GRID_DOT_SIZE, SUNK_COLOR);
            }
        }
    }
}


#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8


void draw_message_box(const char* message){
     // draw a black square in the middle of the screen
    fillRect(0, Y_MAX/2 - 16, X_MAX, 24, BLACK);
    drawRect(0, Y_MAX/2 - 16, X_MAX, 24, WHITE);

    // calculate the length of the message
    int i = 0;
    while(message[i] != '\0'){
        i++;
    }
    // center the text in the middle of the screen
    drawString(message, X_MAX/2 - i * (CHAR_WIDTH/2), Y_MAX/2 - CHAR_HEIGHT, WHITE, BLACK, 1);
}
