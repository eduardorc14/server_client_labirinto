#pragma once

#include<stdio.h>

#include <stdlib.h>

#include <arpa/inet.h>

struct action {         // Estrutura da mensagem de comunicação entre o servidor e cliente
    int type;
    int moves[100];
    int board[10][10];
};


typedef enum {
    ACTION_START = 0,
    ACTION_MOVE,
    ACTION_MAP,
    ACTION_HINT,
    ACTION_UPDATE,
    ACTION_WIN,
    ACTION_RESET,
    ACTION_EXIT
} ActionType;

typedef enum {
    up = 1,
    right =2,
    down = 3,
    left = 4

} MoveType;

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);

void contar_dimensoes(FILE *file, int *linhas, int *colunas);

int **alocar_matriz(int linhas, int colunas);

void preencher_matriz(FILE *file, int **matriz, int linhas, int colunas);

void exibir_matriz(int **labirinto, int linhas, int colunas);

void liberar_matriz(int **matriz, int linhas);
