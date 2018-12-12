#ifndef CUGO_BOARD_H
#define CUGO_BOARD_H

#include "point.h"
#include "list_point.h"

enum player_e {
    PLAYER1 = 0,
    PLAYER2 = 1
};

struct board_t {
    uint_t length;
    list_point_t *player1_pieces;
    list_point_t *player2_pieces;
    uint_t moves;
};

board_t* board_create(uint_t);

void board_delete(board_t*);

player_e board_current_player(board_t*);

player_e board_opponent(board_t *);

bool board_place_piece(board_t*, player_e, point_t);

void board_move_piece(board_t*, player_e, point_t, point_t);

int board_player_defeated(board_t*, player_e);

#endif // CUGO_BOARD_H
