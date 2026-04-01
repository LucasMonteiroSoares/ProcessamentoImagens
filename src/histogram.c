/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

/*
 * histogram.c – Cálculo e renderização do histograma.
 */

#include "app.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────────────────────────────────────
 * compute_histogram
 *   Calcula frequências, média, desvio padrão e classificações.
 * ───────────────────────────────────────────────────────────── */
void compute_histogram(SDL_Surface *gray_surf, Histogram *hist)
{
    if (!gray_surf || !hist) return;

    memset(hist, 0, sizeof(*hist));

    const uint32_t *pixels = (const uint32_t *)gray_surf->pixels;
    const int total = gray_surf->w * gray_surf->h;

    /* Frequências */
    for (int i = 0; i < total; i++) {
        uint8_t lum = (pixels[i] >> 24) & 0xFF;
        hist->freq[lum]++;
    }

    /* Maior frequência */
    hist->max_freq = 0;
    for (int i = 0; i < HIST_LEVELS; i++)
        if (hist->freq[i] > hist->max_freq)
            hist->max_freq = hist->freq[i];

    /* Média */
    double sum = 0.0;
    for (int i = 0; i < HIST_LEVELS; i++)
        sum += (double)i * hist->freq[i];
    hist->mean = sum / total;

    /* Desvio padrão */
    double var = 0.0;
    for (int i = 0; i < HIST_LEVELS; i++) {
        double diff = (double)i - hist->mean;
        var += diff * diff * hist->freq[i];
    }
    hist->stddev = sqrt(var / total);

    /* Classificação de brilho: media < 85 → escura, > 170 → clara */
    if (hist->mean < 85.0)
        hist->brightness = BRIGHT_DARK;
    else if (hist->mean > 170.0)
        hist->brightness = BRIGHT_LIGHT;
    else
        hist->brightness = BRIGHT_MEDIUM;

    /* Classificação de contraste: desvio < 40 → baixo, > 70 → alto */
    if (hist->stddev < 40.0)
        hist->contrast = CONTRAST_LOW;
    else if (hist->stddev > 70.0)
        hist->contrast = CONTRAST_HIGH;
    else
        hist->contrast = CONTRAST_MEDIUM;
}

/* ─────────────────────────────────────────────────────────────
 * draw_histogram
 *   Desenha o histograma como barras verticais dentro da região
 *   (x, y, w, h) usando o renderer fornecido.
 * ───────────────────────────────────────────────────────────── */
void draw_histogram(SDL_Renderer *ren, const Histogram *hist,
                    int x, int y, int w, int h)
{
    if (!ren || !hist || hist->max_freq == 0) return;

    /* Fundo do histograma */
    SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
    SDL_FRect bg = { (float)x, (float)y, (float)w, (float)h };
    SDL_RenderFillRect(ren, &bg);

    /* Borda */
    SDL_SetRenderDrawColor(ren, 120, 120, 140, 255);
    SDL_RenderRect(ren, &bg);

    /* Barras – cada barra tem largura fracionária */
    const float bar_w = (float)w / (float)HIST_LEVELS;
    const float scale = (float)(h - 4) / (float)hist->max_freq;

    for (int i = 0; i < HIST_LEVELS; i++) {
        if (hist->freq[i] == 0) continue;

        float bar_h = hist->freq[i] * scale;
        if (bar_h < 1.0f) bar_h = 1.0f;

        /* Cor degradê: barras mais claras para níveis mais altos */
        uint8_t shade = (uint8_t)i;
        SDL_SetRenderDrawColor(ren, shade, shade, (uint8_t)(shade / 2 + 128), 255);

        SDL_FRect bar = {
            (float)x + i * bar_w,
            (float)(y + h) - bar_h - 2.0f,
            bar_w > 1.0f ? bar_w : 1.0f,
            bar_h
        };
        SDL_RenderFillRect(ren, &bar);
    }

    /* Linha de base */
    SDL_SetRenderDrawColor(ren, 180, 180, 200, 255);
    SDL_RenderLine(ren,
        (float)x,     (float)(y + h - 2),
        (float)(x+w), (float)(y + h - 2));
}
