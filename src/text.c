/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

/*
 * text.c – Utilitário para renderizar texto com SDL_ttf 3.
 */

#include "app.h"

void render_text(SDL_Renderer *ren, TTF_Font *font,
                 const char *text, float x, float y, SDL_Color color)
{
    if (!ren || !font || !text) return;

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (tex) {
        float w = (float)surf->w;
        float h = (float)surf->h;
        SDL_FRect dst = { x, y, w, h };
        SDL_RenderTexture(ren, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surf);
}
