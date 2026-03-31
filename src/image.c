/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

/*
 * image.c – Carregamento, conversão para cinza, equalização e salvamento.
 */

#include "app.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 * load_image
 *   Carrega um arquivo de imagem usando SDL_image.
 *   Converte para RGBA8888 para facilitar o acesso a pixels.
 *   Retorna NULL em caso de erro.
 * ───────────────────────────────────────────────────────────── */
SDL_Surface *load_image(const char *path)
{
    if (!path || path[0] == '\0') {
        SDL_Log("Erro: caminho da imagem não fornecido.");
        return NULL;
    }

    SDL_Surface *raw = IMG_Load(path);
    if (!raw) {
        SDL_Log("Erro ao carregar imagem '%s': %s", path, SDL_GetError());
        return NULL;
    }

    /* Converte para RGBA8888 para acesso uniforme a pixels */
    SDL_Surface *converted = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(raw);

    if (!converted) {
        SDL_Log("Erro ao converter formato de pixel: %s", SDL_GetError());
        return NULL;
    }

    SDL_Log("Imagem carregada: %s (%dx%d)", path,
            converted->w, converted->h);
    return converted;
}

/* ─────────────────────────────────────────────────────────────
 * is_grayscale
 *   Verifica se todos os pixels têm R == G == B.
 *   Amostra até 10000 pixels para acelerar em imagens grandes.
 * ───────────────────────────────────────────────────────────── */
bool is_grayscale(SDL_Surface *surf)
{
    if (!surf) return false;

    const int total  = surf->w * surf->h;
    const int step   = (total > 10000) ? total / 10000 : 1;
    const uint32_t *pixels = (const uint32_t *)surf->pixels;
    /* SDL_PIXELFORMAT_RGBA8888: R<<24 | G<<16 | B<<8 | A */

    for (int i = 0; i < total; i += step) {
        uint32_t p = pixels[i];
        uint8_t r = (p >> 24) & 0xFF;
        uint8_t g = (p >> 16) & 0xFF;
        uint8_t b = (p >>  8) & 0xFF;
        if (r != g || g != b) return false;
    }
    return true;
}

/* ─────────────────────────────────────────────────────────────
 * convert_to_grayscale
 *   Aplica  Y = 0.2125*R + 0.7154*G + 0.0721*B
 *   Retorna nova superfície; a original não é modificada.
 * ───────────────────────────────────────────────────────────── */
SDL_Surface *convert_to_grayscale(SDL_Surface *surf)
{
    if (!surf) return NULL;

    SDL_Surface *gray = SDL_CreateSurface(surf->w, surf->h,
                                          SDL_PIXELFORMAT_RGBA8888);
    if (!gray) {
        SDL_Log("Erro ao criar superfície cinza: %s", SDL_GetError());
        return NULL;
    }

    const uint32_t *src = (const uint32_t *)surf->pixels;
    uint32_t       *dst = (uint32_t *)gray->pixels;
    const int total = surf->w * surf->h;

    for (int i = 0; i < total; i++) {
        uint32_t p = src[i];
        uint8_t r = (p >> 24) & 0xFF;
        uint8_t g = (p >> 16) & 0xFF;
        uint8_t b = (p >>  8) & 0xFF;
        uint8_t a = (p      ) & 0xFF;

        uint8_t y = (uint8_t)(0.2125 * r + 0.7154 * g + 0.0721 * b + 0.5);
        dst[i] = ((uint32_t)y << 24) | ((uint32_t)y << 16)
               | ((uint32_t)y <<  8) | (uint32_t)a;
    }

    return gray;
}

/* ─────────────────────────────────────────────────────────────
 * equalize_histogram
 *   Equalização clássica via CDF normalizada.
 *   Entrada: superfície em escala de cinza.
 *   Retorna nova superfície equalizada.
 * ───────────────────────────────────────────────────────────── */
SDL_Surface *equalize_histogram(SDL_Surface *gray_surf)
{
    if (!gray_surf) return NULL;

    const int w = gray_surf->w;
    const int h = gray_surf->h;
    const int total = w * h;
    const uint32_t *src = (const uint32_t *)gray_surf->pixels;

    /* 1. Calcula histograma */
    uint32_t hist[HIST_LEVELS];
    memset(hist, 0, sizeof(hist));
    for (int i = 0; i < total; i++) {
        uint8_t lum = (src[i] >> 24) & 0xFF;
        hist[lum]++;
    }

    /* 2. CDF */
    uint32_t cdf[HIST_LEVELS];
    cdf[0] = hist[0];
    for (int i = 1; i < HIST_LEVELS; i++)
        cdf[i] = cdf[i - 1] + hist[i];

    /* Encontra o menor valor da CDF diferente de zero */
    uint32_t cdf_min = 0;
    for (int i = 0; i < HIST_LEVELS; i++) {
        if (cdf[i] > 0) { cdf_min = cdf[i]; break; }
    }

    /* 3. Tabela de remapeamento */
    uint8_t lut[HIST_LEVELS];
    for (int i = 0; i < HIST_LEVELS; i++) {
        if (total == (int)cdf_min) {
            lut[i] = (uint8_t)i;
        } else {
            double val = ((double)(cdf[i] - cdf_min) /
                          (double)(total  - cdf_min)) * 255.0 + 0.5;
            lut[i] = (uint8_t)(val < 0 ? 0 : val > 255 ? 255 : val);
        }
    }

    /* 4. Aplica LUT */
    SDL_Surface *eq = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!eq) {
        SDL_Log("Erro ao criar superfície equalizada: %s", SDL_GetError());
        return NULL;
    }
    uint32_t *dst = (uint32_t *)eq->pixels;
    for (int i = 0; i < total; i++) {
        uint8_t  lum  = (src[i] >> 24) & 0xFF;
        uint8_t  a    = (src[i]      ) & 0xFF;
        uint8_t  newy = lut[lum];
        dst[i] = ((uint32_t)newy << 24) | ((uint32_t)newy << 16)
               | ((uint32_t)newy <<  8) | (uint32_t)a;
    }

    return eq;
}

/* ─────────────────────────────────────────────────────────────
 * save_image
 *   Salva a superfície como PNG.
 *   Sobrescreve se já existir.
 * ───────────────────────────────────────────────────────────── */
void save_image(SDL_Surface *surf, const char *path)
{
    if (!surf || !path) return;

    if (IMG_SavePNG(surf, path)) {
        SDL_Log("Imagem salva em '%s'.", path);
    } else {
        SDL_Log("Erro ao salvar imagem '%s': %s", path, SDL_GetError());
    }
}
