/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

/*
 * window.c – Criação de janelas, loop de renderização e limpeza.
 */

#include "app.h"
#include <stdio.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 * Localiza um arquivo de fonte TrueType no sistema.
 * ───────────────────────────────────────────────────────────── */
static const char *find_font(void)
{
    static const char *candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/Library/Fonts/Arial.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        NULL
    };
    for (int i = 0; candidates[i]; i++) {
        SDL_IOStream *f = SDL_IOFromFile(candidates[i], "rb");
        if (f) { SDL_CloseIO(f); return candidates[i]; }
    }
    return NULL;
}

/* ─────────────────────────────────────────────────────────────
 * init_sdl
 *   Inicializa SDL, cria as duas janelas e os renderers.
 * ───────────────────────────────────────────────────────────── */
bool init_sdl(AppState *app, int img_w, int img_h)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return false;
    }

    if (!TTF_Init()) {
        SDL_Log("TTF_Init falhou: %s", SDL_GetError());
        return false;
    }

    /* ── Janela principal ── */
    app->win_main = SDL_CreateWindow(
        "Processamento de Imagens – Imagem",
        img_w, img_h,
        SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (!app->win_main) {
        SDL_Log("Falha ao criar janela principal: %s", SDL_GetError());
        return false;
    }

    /* Centraliza no monitor principal */
    SDL_SetWindowPosition(app->win_main,
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);

    app->ren_main = SDL_CreateRenderer(app->win_main, NULL);
    if (!app->ren_main) {
        SDL_Log("Falha ao criar renderer principal: %s", SDL_GetError());
        return false;
    }

    /* ── Janela secundária (filha) ── */
    /* Posiciona à direita da janela principal */
    int main_x = 0, main_y = 0;
    SDL_GetWindowPosition(app->win_main, &main_x, &main_y);

    app->win_sec = SDL_CreateWindow(
        "Histograma",
        SEC_WIN_W, SEC_WIN_H,
        SDL_WINDOW_HIGH_PIXEL_DENSITY
    );
    if (!app->win_sec) {
        SDL_Log("Falha ao criar janela secundária: %s", SDL_GetError());
        return false;
    }

    SDL_SetWindowPosition(app->win_sec,
                          main_x + img_w + 10,
                          main_y);

    /* Define a janela principal como pai */
    SDL_SetWindowParent(app->win_sec, app->win_main);

    app->ren_sec = SDL_CreateRenderer(app->win_sec, NULL);
    if (!app->ren_sec) {
        SDL_Log("Falha ao criar renderer secundário: %s", SDL_GetError());
        return false;
    }

    /* ── Fontes ── */
    const char *font_path = find_font();
    if (font_path) {
        app->font       = TTF_OpenFont(font_path, 15);
        app->font_small = TTF_OpenFont(font_path, 12);
    }
    if (!app->font)
        SDL_Log("Aviso: fonte não encontrada; texto não será exibido.");

    /* ── Botão ── */
    app->btn_rect.x = 10.0f;
    app->btn_rect.y = (float)(SEC_WIN_H - 50);
    app->btn_rect.w = (float)(SEC_WIN_W - 20);
    app->btn_rect.h = 38.0f;
    app->btn_state  = BTN_NORMAL;

    return true;
}

/* ─────────────────────────────────────────────────────────────
 * update_main_texture
 *   Recria a textura da janela principal com a superfície atual.
 * ───────────────────────────────────────────────────────────── */
void update_main_texture(AppState *app)
{
    if (app->tex_display) {
        SDL_DestroyTexture(app->tex_display);
        app->tex_display = NULL;
    }

    SDL_Surface *src = app->equalized
                     ? app->surf_equalized
                     : app->surf_original_gray;
    if (!src) return;

    app->tex_display = SDL_CreateTextureFromSurface(app->ren_main, src);
}

/* ─────────────────────────────────────────────────────────────
 * render_main  – Desenha a imagem na janela principal.
 * ───────────────────────────────────────────────────────────── */
void render_main(AppState *app)
{
    SDL_SetRenderDrawColor(app->ren_main, 30, 30, 30, 255);
    SDL_RenderClear(app->ren_main);

    if (app->tex_display) {
        SDL_FRect dst = { 0, 0, (float)app->img_w, (float)app->img_h };
        SDL_RenderTexture(app->ren_main, app->tex_display, NULL, &dst);
    }

    SDL_RenderPresent(app->ren_main);
}

/* ─────────────────────────────────────────────────────────────
 * render_secondary
 *   Desenha histograma, informações e botão na janela secundária.
 * ───────────────────────────────────────────────────────────── */
void render_secondary(AppState *app)
{
    SDL_Renderer *ren  = app->ren_sec;
    TTF_Font     *font = app->font;
    const Histogram *h = &app->hist_current;

    /* ── Fundo ── */
    SDL_SetRenderDrawColor(ren, 18, 18, 28, 255);
    SDL_RenderClear(ren);

    /* ── Título ── */
    SDL_Color white  = { 230, 230, 240, 255 };
    SDL_Color yellow = { 255, 220,  60, 255 };
    SDL_Color green  = {  80, 220, 120, 255 };
    SDL_Color red    = { 240,  80,  80, 255 };

    render_text(ren, font, "Histograma", 10, 8, white);

    /* ── Histograma ── */
    draw_histogram(ren, h, 10, 30, SEC_WIN_W - 20, 200);

    /* ── Informações de análise ── */
    float ty = 245.0f;
    const float line_h = 22.0f;

    /* Média */
    char buf[128];
    snprintf(buf, sizeof(buf), "Media de intensidade: %.1f", h->mean);
    render_text(ren, font, buf, 10, ty, white);
    ty += line_h;

    const char *brightness_str;
    SDL_Color   brightness_color;
    switch (h->brightness) {
        case BRIGHT_DARK:
            brightness_str = "  Classificacao: Escura";
            brightness_color = red;
            break;
        case BRIGHT_LIGHT:
            brightness_str = "  Classificacao: Clara";
            brightness_color = yellow;
            break;
        default:
            brightness_str = "  Classificacao: Media";
            brightness_color = green;
            break;
    }
    render_text(ren, font, brightness_str, 10, ty, brightness_color);
    ty += line_h + 4;

    /* Desvio padrão */
    snprintf(buf, sizeof(buf), "Desvio padrao: %.1f", h->stddev);
    render_text(ren, font, buf, 10, ty, white);
    ty += line_h;

    const char *contrast_str;
    SDL_Color   contrast_color;
    switch (h->contrast) {
        case CONTRAST_LOW:
            contrast_str = "  Contraste: Baixo";
            contrast_color = red;
            break;
        case CONTRAST_HIGH:
            contrast_str = "  Contraste: Alto";
            contrast_color = green;
            break;
        default:
            contrast_str = "  Contraste: Medio";
            contrast_color = yellow;
            break;
    }
    render_text(ren, font, contrast_str, 10, ty, contrast_color);
    ty += line_h + 4;

    /* Estado atual */
    const char *state_str = app->equalized
        ? "Estado: Equalizado" : "Estado: Original (cinza)";
    SDL_Color state_color = app->equalized ? yellow : white;
    render_text(ren, font, state_str, 10, ty, state_color);

    /* ── Instrução tecla S ── */
    SDL_Color gray_hint = { 140, 140, 160, 255 };
    render_text(ren, app->font_small,
                "Pressione S para salvar a imagem atual.",
                10, (float)(SEC_WIN_H - 72), gray_hint);

    /* ── Botão ── */
    draw_button(ren, font,
                &app->btn_rect,
                app->btn_state,
                app->equalized);

    SDL_RenderPresent(ren);
}

/* ─────────────────────────────────────────────────────────────
 * cleanup  – Libera todos os recursos SDL.
 * ───────────────────────────────────────────────────────────── */
void cleanup(AppState *app)
{
    if (app->tex_display)         SDL_DestroyTexture(app->tex_display);
    if (app->surf_equalized)      SDL_DestroySurface(app->surf_equalized);
    if (app->surf_original_gray)  SDL_DestroySurface(app->surf_original_gray);
    if (app->font)                TTF_CloseFont(app->font);
    if (app->font_small)          TTF_CloseFont(app->font_small);
    if (app->ren_sec)             SDL_DestroyRenderer(app->ren_sec);
    if (app->win_sec)             SDL_DestroyWindow(app->win_sec);
    if (app->ren_main)            SDL_DestroyRenderer(app->ren_main);
    if (app->win_main)            SDL_DestroyWindow(app->win_main);
    TTF_Quit();
    SDL_Quit();
}
