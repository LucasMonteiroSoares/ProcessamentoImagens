# ══════════════════════════════════════════════════════
#  Makefile – Projeto 1: Processamento de Imagens
#  Disciplina: Computação Visual – Mackenzie
# ══════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2 \
          $(shell pkg-config --cflags sdl3 sdl3-image sdl3-ttf)
LDFLAGS = -lSDL3_image -lSDL3_ttf -lSDL3 -lm
TARGET  = programa
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c     \
          $(SRCDIR)/image.c    \
          $(SRCDIR)/histogram.c\
          $(SRCDIR)/button.c   \
          $(SRCDIR)/text.c     \
          $(SRCDIR)/window.c

OBJS    = $(SRCS:.c=.o)

# ── Regras ─────────────────────────────────────────────
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build concluído: ./$(TARGET)"

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/app.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) output_image.png
	@echo "Limpeza concluída."
