#ifndef APP_H
#define APP_H

/*
 * Projeto 1 – Processamento de Imagens
 * Disciplina: Computação Visual
 * Universidade Presbiteriana Mackenzie
 * Professor: André Kishimoto
 */

#include <SDL3/SDL.h>

#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>

/* ── Dimensões da janela secundária ─────────────────────────── */
#define SEC_WIN_W  420
#define SEC_WIN_H  560

/* ── Histograma ──────────────────────────────────────────────── */
#define HIST_LEVELS 256

/* ── Classificação de intensidade ───────────────────────────── */
typedef enum {
    BRIGHT_DARK   = 0,
    BRIGHT_MEDIUM = 1,
    BRIGHT_LIGHT  = 2
} BrightnessClass;

typedef enum {
    CONTRAST_LOW    = 0,
    CONTRAST_MEDIUM = 1,
    CONTRAST_HIGH   = 2
} ContrastClass;

/* ── Estado do botão ─────────────────────────────────────────── */
typedef enum {
    BTN_NORMAL = 0,
    BTN_HOVER  = 1,
    BTN_PRESSED= 2
} ButtonState;

/* ── Dados do histograma ─────────────────────────────────────── */
typedef struct {
    uint32_t        freq[HIST_LEVELS];   /* frequência de cada nível         */
    uint32_t        max_freq;            /* maior frequência (para escala)   */
    double          mean;                /* média                            */
    double          stddev;             /* desvio padrão                    */
    BrightnessClass brightness;
    ContrastClass   contrast;
} Histogram;

/* ── Estado global da aplicação ─────────────────────────────── */
typedef struct {
    /* SDL handles */
    SDL_Window   *win_main;
    SDL_Renderer *ren_main;
    SDL_Window   *win_sec;
    SDL_Renderer *ren_sec;
    TTF_Font     *font;
    TTF_Font     *font_small;

    /* Superfícies / texturas */
    SDL_Surface  *surf_original_gray;   /* escala de cinza original         */
    SDL_Surface  *surf_equalized;       /* após equalização                 */
    SDL_Texture  *tex_display;          /* textura exibida na janela princ. */

    /* Histogramas */
    Histogram     hist_current;         /* do estado atual (gray ou eq.)    */

    /* Flags */
    bool          equalized;            /* true => mostrando versão eq.     */
    bool          running;

    /* Botão na janela secundária */
    SDL_FRect     btn_rect;
    ButtonState   btn_state;

    /* Dimensões da imagem */
    int           img_w;
    int           img_h;
} AppState;

/* ── Protótipos ──────────────────────────────────────────────── */

/* image.c */
SDL_Surface *load_image(const char *path);
bool         is_grayscale(SDL_Surface *surf);
SDL_Surface *convert_to_grayscale(SDL_Surface *surf);
SDL_Surface *equalize_histogram(SDL_Surface *gray_surf);
void         save_image(SDL_Surface *surf, const char *path);

/* histogram.c */
void compute_histogram(SDL_Surface *gray_surf, Histogram *hist);
void draw_histogram(SDL_Renderer *ren, const Histogram *hist,
                    int x, int y, int w, int h);

/* button.c */
void draw_button(SDL_Renderer *ren, TTF_Font *font,
                 const SDL_FRect *rect, ButtonState state, bool equalized);
bool point_in_rect(float px, float py, const SDL_FRect *r);

/* text.c */
void render_text(SDL_Renderer *ren, TTF_Font *font,
                 const char *text, float x, float y, SDL_Color color);

/* window.c */
bool init_sdl(AppState *app, int img_w, int img_h);
void update_main_texture(AppState *app);
void render_main(AppState *app);
void render_secondary(AppState *app);
void cleanup(AppState *app);

#endif /* APP_H */
