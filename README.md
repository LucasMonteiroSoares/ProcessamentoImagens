# Projeto 1 – Processamento de Imagens

**Disciplina:** Computação Visual  
**Professor:** André Kishimoto  

---

## O que é o projeto

É um programa de processamento de imagens feito em C usando a biblioteca SDL3. Você passa uma imagem pela linha de comando e o programa abre duas janelas: uma mostrando a imagem em escala de cinza, e outra com o histograma da imagem e um botão para equalizar.

---

## Como funciona

1. O programa carrega a imagem passada como argumento
2. Verifica se ela já é em escala de cinza. Se não for, converte usando a fórmula `Y = 0.2125*R + 0.7154*G + 0.0721*B`
3. Abre a janela principal mostrando a imagem em cinza
4. Abre a janela secundária com o histograma, a média de intensidade, o desvio padrão e um botão de equalizar
5. Ao clicar no botão "Equalizar", a imagem é equalizada e o histograma atualiza. Clicando em "Ver original" volta para a versão em cinza sem recarregar o arquivo
6. Pressionando a tecla S, a imagem atual é salva como `output_image.png`

---

## Estrutura do projeto

```
proj1/
├── Makefile
├── README.md
└── src/
    ├── app.h          # structs, enums e protótipos usados em todo o projeto
    ├── main.c         # ponto de entrada e loop de eventos
    ├── image.c        # carregamento, conversão para cinza, equalização e salvamento
    ├── histogram.c    # cálculo e desenho do histograma
    ├── button.c       # desenho e detecção de clique do botão
    ├── text.c         # renderização de texto com SDL_ttf
    └── window.c       # criação das janelas, renderização e limpeza
```

---

## Dependências

- SDL3
- SDL3_image
- SDL3_ttf

As três bibliotecas precisam ser compiladas da fonte pois ainda não estão nos repositórios do MSYS2.

### Controles

| Ação | Como fazer |
|------|------------|
| Equalizar / Ver original | Botão na janela secundária |
| Salvar imagem atual | Tecla `S` |
| Fechar o programa | Tecla `ESC` ou fechar a janela |

---

## Contribuições

| Nome | RA | O que fez |
|------|----|-----------|
| Lucas Monteiro Soares | 10417881 | Estrutura do projeto, build system e tipos globais (app.h, Makefile, .gitignore) |
| Matheus Chediac Rodrigues | 10417490 | Carregamento de imagem, conversão para escala de cinza, equalização e salvamento (image.c) |
| Leonardo Magalhães | 10417121 | Loop principal de eventos, gerenciamento de janelas e documentação (main.c, window.c, README.md) |
| Fernando Cavaleiro Paiva | 10416680 | Histograma, botão de equalização e renderização de texto (histogram.c, button.c, text.c) |

