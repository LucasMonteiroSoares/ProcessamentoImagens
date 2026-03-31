/*
 * Autores:
 *   Fernando Cavaleiro Paiva     - 10416680
 *   Lucas Monteiro Soares        - 10417881
 *   Leonardo Magalhães           - 10417121
 *   Matheus Chediac Rodrigues    - 10417490
 */

#include <SDL3/SDL_main.h>
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* ── Verifica argumentos ── */
    if (argc < 2) {
        fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    /* ── Carrega imagem ── */
    SDL_Surface *raw = load_image(image_path);
    if (!raw) {
        fprintf(stderr, "Erro: não foi possível carregar '%s'.\n", image_path);
        return EXIT_FAILURE;
    }

    /* ── Converte para escala de cinza (se necessário) ── */
    SDL_Surface *gray = NULL;
    if (is_grayscale(raw)) {
        SDL_Log("Imagem já está em escala de cinza.");
        gray = raw;
        raw  = NULL;
    } else {
        SDL_Log("Imagem colorida detectada. Convertendo para escala de cinza...");
        gray = convert_to_grayscale(raw);
        SDL_DestroySurface(raw);
        raw = NULL;
        if (!gray) {
            fprintf(stderr, "Erro na conversão para escala de cinza.\n");
            return EXIT_FAILURE;
        }
    }

    /* ── Inicializa estado da aplicação ── */
    AppState app;
    memset(&app, 0, sizeof(app));

    app.surf_original_gray = gray;
    app.img_w              = gray->w;
    app.img_h              = gray->h;
    app.equalized          = false;
    app.running            = true;

    /* ── Inicializa SDL e janelas ── */
    if (!init_sdl(&app, app.img_w, app.img_h)) {
        fprintf(stderr, "Erro ao inicializar SDL.\n");
        SDL_DestroySurface(gray);
        return EXIT_FAILURE;
    }

    /* ── Textura inicial ── */
    update_main_texture(&app);

    /* ── Histograma inicial ── */
    compute_histogram(app.surf_original_gray, &app.hist_current);

    /* ── Renderização inicial ── */
    render_main(&app);
    render_secondary(&app);

    /* ════════════════════════════════════
     *  Loop principal de eventos
     * ════════════════════════════════════ */
    SDL_Event ev;
    while (app.running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {

            /* ── Fechar qualquer uma das janelas encerra o programa ── */
            case SDL_EVENT_QUIT:
                app.running = false;
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                app.running = false;
                break;

            /* ── Tecla pressionada ── */
            case SDL_EVENT_KEY_DOWN:
                if (ev.key.key == SDLK_S) {
                    /* Salva a imagem atualmente exibida */
                    SDL_Surface *to_save = app.equalized
                        ? app.surf_equalized
                        : app.surf_original_gray;
                    save_image(to_save, "output_image.png");
                }
                if (ev.key.key == SDLK_ESCAPE) {
                    app.running = false;
                }
                break;

            /* ── Mouse na janela secundária ── */
            case SDL_EVENT_MOUSE_MOTION: {
                if (ev.motion.windowID ==
                        SDL_GetWindowID(app.win_sec)) {
                    float mx = ev.motion.x;
                    float my = ev.motion.y;
                    bool over = point_in_rect(mx, my, &app.btn_rect);
                    ButtonState new_state = over ? BTN_HOVER : BTN_NORMAL;
                    if (new_state != app.btn_state) {
                        app.btn_state = new_state;
                        render_secondary(&app);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (ev.button.windowID ==
                        SDL_GetWindowID(app.win_sec)) {
                    float mx = ev.button.x;
                    float my = ev.button.y;
                    if (point_in_rect(mx, my, &app.btn_rect)) {
                        app.btn_state = BTN_PRESSED;
                        render_secondary(&app);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (ev.button.windowID ==
                        SDL_GetWindowID(app.win_sec)) {
                    float mx = ev.button.x;
                    float my = ev.button.y;
                    if (point_in_rect(mx, my, &app.btn_rect)) {
                        /* Alterna equalização */
                        if (!app.equalized) {
                            /* Equalizar */
                            if (!app.surf_equalized) {
                                app.surf_equalized =
                                    equalize_histogram(app.surf_original_gray);
                            }
                            if (app.surf_equalized) {
                                app.equalized = true;
                                compute_histogram(app.surf_equalized,
                                                  &app.hist_current);
                                update_main_texture(&app);
                                render_main(&app);
                            }
                        } else {
                            /* Reverter para original */
                            app.equalized = false;
                            compute_histogram(app.surf_original_gray,
                                              &app.hist_current);
                            update_main_texture(&app);
                            render_main(&app);
                        }
                        /* Volta ao estado hover (cursor ainda está em cima) */
                        app.btn_state = BTN_HOVER;
                        render_secondary(&app);
                    } else {
                        app.btn_state = BTN_NORMAL;
                        render_secondary(&app);
                    }
                }
                break;
            }

            /* ── Reexibir ao reexpor janelas ── */
            case SDL_EVENT_WINDOW_EXPOSED:
                if (ev.window.windowID ==
                        SDL_GetWindowID(app.win_main)) {
                    render_main(&app);
                } else {
                    render_secondary(&app);
                }
                break;

            default:
                break;
            }
        }

        /* Pequeno atraso para não consumir 100% da CPU */
        SDL_Delay(8);
    }

    /* ── Limpeza ── */
    cleanup(&app);
    return EXIT_SUCCESS;
}
