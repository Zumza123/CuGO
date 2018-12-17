#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "dimensions.h"
#include "board.h"
#include "list_point.h"

SDL_Window* render_create_window() {
    return SDL_CreateWindow(
        "CuGO Game",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI
    );
}

SDL_Renderer* render_create_renderer(SDL_Window *window) {
    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
    return renderer;
}

void SDL_SetRenderDrawColor(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void render_clear(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color);
    SDL_RenderClear(renderer);
}

void render_text(SDL_Renderer *renderer, char *text, uint_t size, SDL_Rect rect, SDL_Color color) {
    TTF_Font *sans = TTF_OpenFont("sans.ttf", size);
    SDL_Surface *surface = TTF_RenderText_Solid(sans, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(sans);
}

void render_logo(SDL_Renderer *renderer) {
    render_text(renderer, "CuGO", 14, {330, 0, 150, 100}, {0, 0, 0});
}

void render_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius) {
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        SDL_Point points[] = {
            {cx + x, cy + y},
            {cx - x, cy + y},

            {cx + y, cy + x},
            {cx - y, cy + x},

            {cx - x, cy - y},
            {cx + x, cy - y},

            {cx - y, cy - x},
            {cx + y, cy - x}
        };

        SDL_RenderDrawLines(renderer, points, sizeof(points) / sizeof(SDL_Point));

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }

        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void render_board_cell(SDL_Renderer *renderer, int x_board, int y_board, SDL_Color color) {
    int sq_stg, sq_sus;
    sq_stg=BOARD_OFFSET_X+x_board*BOARD_CELL_SIZE;
    sq_sus=BOARD_OFFSET_Y+y_board*BOARD_CELL_SIZE;
    SDL_SetRenderDrawColor(renderer, color);
    SDL_Rect rect = {sq_stg, sq_sus, BOARD_CELL_SIZE, BOARD_CELL_SIZE};
    SDL_RenderFillRect(renderer, &rect);
}

void render_board_grid(SDL_Renderer *renderer, board_t *board) {
    for(int x_board=0; x_board<board->length; x_board++) {
        for(int y_board=0; y_board<board->length; y_board++) {
                if((x_board+y_board) % 2 == 0)
                    render_board_cell(renderer, x_board, y_board, {255, 204, 102, 255});
                else
                    render_board_cell(renderer, x_board, y_board, {153, 102, 0, 255});
        }
    }
}

void render_board_piece(SDL_Renderer *renderer, point_t pos) {
    int left, top;
    left=BOARD_OFFSET_X+BOARD_CELL_SIZE*pos.x;
    top=BOARD_OFFSET_Y+BOARD_CELL_SIZE*pos.y;
    left=left+BOARD_CELL_SIZE/2;
    top=top+BOARD_CELL_SIZE/2;
    render_filled_circle(renderer, left, top, BOARD_CELL_SIZE/3);
}

void render_board_pieces(SDL_Renderer *renderer, list_point_t *list, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color);
    for (list_node_point_t *node = list->first; node; node = node->next)
        render_board_piece(renderer, node->value);
}

void render_board(SDL_Renderer* renderer, board_t *board, const SDL_Color colors[2]) {
    render_board_grid(renderer, board);
    render_board_pieces(renderer, board->player1_pieces, colors[0]);
    render_board_pieces(renderer, board->player2_pieces, colors[1]);
}
