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

void game_init() {
    pieces_encircled = 0;
    board = board_create(menu_board_size());
    board_offset = {
        (int)(WINDOW_WIDTH - board->size * BOARD_CELL_SIZE) / 2,
        (int)(WINDOW_HEIGHT - board->size * BOARD_CELL_SIZE) / 2 + 20
    };
}

void game_deinit() {
    board_delete(board);
}

bool game_inited() { return board; }
bool game_started() { return game_inited() && board->player1_pieces->length; }
bool game_ended() { return game_started() && pieces_encircled; }

point_t board_position(SDL_MouseButtonEvent mouse) {
    // Ot
    return {
        // (Relative position from board offset / Cell size) = Position of the click in the 0..<board.size grid
        (mouse.x - board_offset.x) / BOARD_CELL_SIZE,
        (mouse.y - board_offset.y) / BOARD_CELL_SIZE
    };
}

void perform_ai_move() {
    if (pieces_encircled) return;
    
    if (board->moves < board->size * 2) {
        point_t piece;
        do {
            piece.x = rand() % board->size;
            piece.y = rand() % board->size;
        } while (!board_place_piece(board, piece, true));
        if (menu_sound())
            sound_play_place_piece();
    } else {
        point_t piece, new_pos;
        list_point_t *potential_moves;
        list_init(potential_moves);

        do {
            uint_t index = rand() % board->size;
            list_node_point_t *node = board_current_player_pieces(board)->first;
            for (; index > 0; index--) node = node->next;
            piece = node->value;
            board_potential_moves(board, piece, potential_moves);
        } while (!potential_moves->length);

        uint_t index = rand() % potential_moves->length;
        list_node_point_t *node = potential_moves->first;
        for (; index > 0; index--) node = node->next;
        new_pos = node->value;
        board_move_piece(board, piece, new_pos);
        
        if (menu_sound())
            sound_play_place_piece();
    }
    
    pieces_encircled = board_player_defeated(board, board_current_player(board));

    if (pieces_encircled && menu_sound())
        sound_play_tada();
}

point_t piece;
bool piece_selected = false;

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

    if (board->moves < board->size * 2) {
        // Initial placing
        if (board_place_piece(board, board_position(mouse), menu_prevent_suicide()) && menu_sound())
            sound_play_place_piece();
    } else {
        // Moves
        if (!piece_selected) {
            point_t pos = board_position(mouse);
            if (list_contains(board_current_player_pieces(board), pos)) {
                piece = pos;
                piece_selected = true;
            }
            return;
        }
        
        piece_selected = false;
        
        if(board_move_piece(board, piece, board_position(mouse)) && menu_sound())
             sound_play_place_piece();
    }

    if (board->moves <= board->size * 2 && !menu_prevent_suicide()) {
        pieces_encircled = board_player_defeated(board, board_opponent(board));
        suicidal_place = pieces_encircled ? true : false;
    }

    if (!suicidal_place)
        pieces_encircled = board_player_defeated(board, board_current_player(board));

    if (pieces_encircled && menu_sound())
        sound_play_tada();
    
    perform_ai_move();
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

    if (piece_selected)
        render_board_piece_selector(board, piece, board_offset, menu_color_scheme());

    render_turn_info(board, menu_color_scheme());

    if (window_event.type == SDL_MOUSEBUTTONDOWN)
        handle_mouse_click(window_event.button);

    render_present();
}
