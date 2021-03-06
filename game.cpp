#include <SDL2/SDL.h>

#include "game.h"
#include "dimensions.h"
#include "board.h"
#include "render.h"
#include "menu.h"
#include "sound.h"

board_t *board;
point_t board_offset;
bool suicidal_place;
uint_t pieces_encircled;
point_t piece_selected;
bool is_piece_selected;

void game_init() {
    pieces_encircled = 0;
    board = board_create(menu_board_size());
    board_offset = {
        (int)(WINDOW_WIDTH - board->size * BOARD_CELL_SIZE) / 2,
        (int)(WINDOW_HEIGHT - board->size * BOARD_CELL_SIZE) / 2 + 20
    };
    suicidal_place = false;
    is_piece_selected = false;
}

void game_deinit() {
    board_delete(board);
}

bool game_inited() { return board; }
bool game_started() { return game_inited() && board->player1_pieces->length; }
bool game_ended() { return game_started() && pieces_encircled; }

point_t board_position(SDL_MouseButtonEvent mouse) {
    return {
        // (Relative position from board offset / Cell size) = Position of the click in the 0..<board.size grid
        (mouse.x - board_offset.x) / BOARD_CELL_SIZE,
        (mouse.y - board_offset.y) / BOARD_CELL_SIZE
    };
}

SDL_Rect menu_btn = {WINDOW_WIDTH - 120, 550, 100, 30};
SDL_Rect new_game_btn = {30, 550, 100, 30};

void handle_mouse_click(SDL_MouseButtonEvent mouse) {
    if (pieces_encircled && menu_button_pressed(mouse, new_game_btn)) {
        game_deinit();
        game_init();
        return;
    }

    if (menu_button_pressed(mouse, menu_btn)) {
        menu_visible(true);
        return;
    }

    if (pieces_encircled) return;

    bool valid_placement = false;

    if (board->moves < board->size * 2) {
        // Initial placing
        valid_placement = board_place_piece(board, board_position(mouse), menu_prevent_suicide());
    } else {
        // Moves
        if (!is_piece_selected) {
            point_t pos = board_position(mouse);
            if (list_contains(board_current_player_pieces(board), pos)) {
                piece_selected = pos;
                is_piece_selected = true;
            }
            return;
        }
        
        is_piece_selected = false;
        valid_placement = board_move_piece(board, piece_selected, board_position(mouse));
    }

    if (!valid_placement) return;

    if (!menu_prevent_suicide() && board->moves <= board->size * 2) {
        pieces_encircled = board_player_defeated(board, board_opponent(board));
        suicidal_place = pieces_encircled ? true : false;
    }

    if (!suicidal_place)
        pieces_encircled = board_player_defeated(board, board_current_player(board));

    if (menu_sound()) {
        sound_play_place_piece();
        if (pieces_encircled)
            sound_play_tada();
    }
}

void render_turn_info(board_t *board, const color_scheme_t &color_scheme) {
    const char *const place = "has to place";
    const char *const move = "has to move";
    static char won[19] = "WON WITH * POINTS!";

    const char *msg;
    int player;
    
    if (!pieces_encircled) {
        msg = (board->moves < board->size * 2) ? place : move;
        player = board_current_player(board);
    } else {
        won[9] = pieces_encircled + '0';
        msg = won;
        player = suicidal_place ? board_current_player(board) : board_opponent(board);
    }
    
    render_circle(12, {30, 30}, color_scheme.player_piece_colors[player]);
    render_text(msg, 14, {50, 20}, {0, 0, 0, 0});

    render_text("No. of moves: ", 14, {WINDOW_WIDTH - 150, 20}, {0, 0, 0, 0});

    char moves[5] = {
        (char)((board->moves / 1000) % 10 + '0'),
        (char)((board->moves / 100) % 10 + '0'),
        (char)((board->moves / 10) % 10 + '0'),
        (char)(board->moves % 10 + '0'),
        '\0'
    };

    render_text(moves, 14, {WINDOW_WIDTH - 55, 20}, {0, 0, 0, 0});
}

void game_loop(SDL_Event window_event) {
    render_clear(menu_color_scheme().background);
    render_logo();

    render_board(board, board_offset, menu_color_scheme());

    render_button(menu_btn, "Menu", menu_color_scheme().buttons_background, {255, 255, 255});

    if (pieces_encircled)
        render_button(new_game_btn, "New Game", menu_color_scheme().buttons_background, {255, 255, 255});

    if (is_piece_selected)
        render_board_piece_selector(board, board_offset, piece_selected, menu_color_scheme());

    render_turn_info(board, menu_color_scheme());

    if (window_event.type == SDL_MOUSEBUTTONDOWN)
        handle_mouse_click(window_event.button);

    render_present();
}
