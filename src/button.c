/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

/*
 * button.c – Desenho e teste de colisão do botão de equalização.
 */

#include "app.h"
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 * point_in_rect  – testa se (px,py) está dentro de r.
 * ───────────────────────────────────────────────────────────── */
bool point_in_rect(float px, float py, const SDL_FRect *r)
{
    return (px >= r->x && px <= r->x + r->w &&
            py >= r->y && py <= r->y + r->h);
}

/* ─────────────────────────────────────────────────────────────
 * draw_button
 *   Desenha o botão com primitivas SDL.
 *   Estado BTN_NORMAL  → azul médio     (#1a6bb5)
 *   Estado BTN_HOVER   → azul claro     (#3b9fe8)
 *   Estado BTN_PRESSED → azul escuro    (#0d3d6e)
 * ───────────────────────────────────────────────────────────── */
void draw_button(SDL_Renderer *ren, TTF_Font *font,
                 const SDL_FRect *rect, ButtonState state, bool equalized)
{
    if (!ren || !rect) return;

    /* Cor de fundo conforme estado */
    switch (state) {
        case BTN_HOVER:
            SDL_SetRenderDrawColor(ren, 59, 159, 232, 255);  /* azul claro  */
            break;
        case BTN_PRESSED:
            SDL_SetRenderDrawColor(ren, 13,  61, 110, 255);  /* azul escuro */
            break;
        default:
            SDL_SetRenderDrawColor(ren, 26, 107, 181, 255);  /* azul médio  */
            break;
    }
    SDL_RenderFillRect(ren, rect);

    /* Borda */
    SDL_SetRenderDrawColor(ren, 200, 220, 255, 255);
    SDL_RenderRect(ren, rect);

    /* Sombra inferior (destaque 3-D simples) */
    if (state != BTN_PRESSED) {
        SDL_SetRenderDrawColor(ren, 10, 40, 80, 200);
        SDL_FRect shadow = { rect->x + 2, rect->y + rect->h,
                              rect->w,   3 };
        SDL_RenderFillRect(ren, &shadow);
    }

    /* Texto centralizado */
    if (font) {
        const char *label = equalized ? "Ver original" : "Equalizar";
        SDL_Color white = { 255, 255, 255, 255 };

        /* Mede o texto para centralizar */
        int tw = 0, th = 0;
        TTF_GetStringSize(font, label, 0, &tw, &th);

        float tx = rect->x + (rect->w - (float)tw) * 0.5f;
        float ty = rect->y + (rect->h - (float)th) * 0.5f;

        /* Renderiza superfície de texto e converte para textura */
        SDL_Surface *surf = TTF_RenderText_Blended(font, label, 0, white);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
            SDL_DestroySurface(surf);
            if (tex) {
                SDL_FRect dst = { tx, ty, (float)tw, (float)th };
                SDL_RenderTexture(ren, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
        }
    }
}
