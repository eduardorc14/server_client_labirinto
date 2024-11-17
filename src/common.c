#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void logexit(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage) {
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    struct in_addr inaddr4; // 32-bit IP address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}

// Funções para leitura do arquivo


// Função para contar linhas e colunas do arquivo
void contar_dimensoes(FILE *file, int *linhas, int *colunas) {
    *linhas = 0;
    *colunas = 0;
    int temp_colunas = 0;
    int valor;

    while (fscanf(file, "%d", &valor) == 1) {
        temp_colunas++;
        if (fgetc(file) == '\n') { // Fim da linha encontrado
            (*linhas)++;
            if (*colunas == 0) { // Define as colunas apenas na primeira linha
                *colunas = temp_colunas;
            } else if (temp_colunas != *colunas) {
                fprintf(stderr, "Erro: número irregular de colunas na linha %d\n", *linhas + 1);
                exit(EXIT_FAILURE);
            }
            temp_colunas = 0; // Reinicia a contagem para a próxima linha
        }
    }
    if (temp_colunas > 0) {
        (*linhas)++; // Conta a última linha, se não terminar com '\n'
    }
    rewind(file); // Volta o ponteiro do arquivo para o início
}

// Função para alocar uma matriz de inteiros
int **alocar_matriz(int linhas, int colunas) {
    int **matriz = (int **)malloc(linhas * sizeof(int *));
    if (matriz == NULL) {
        perror("Erro ao alocar memória para as linhas");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < linhas; i++) {
        matriz[i] = (int *)malloc(colunas * sizeof(int));
        if (matriz[i] == NULL) {
            perror("Erro ao alocar memória para as colunas");
            exit(EXIT_FAILURE);
        }
    }
    return matriz;
}

// Função para preencher a matriz com os dados do arquivo
void preencher_matriz(FILE *file, int **matriz, int linhas, int colunas) {
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            if (fscanf(file, "%d", &matriz[i][j]) != 1) {
                fprintf(stderr, "Erro ao ler o valor na posição %d,%d\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
    }
}


void exibir_matriz(int **labirinto, int linhas, int colunas){
    printf("Labirinto (%d linhas x %d colunas):\n", linhas, colunas);
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            printf("%d ", labirinto[i][j]);
        }
        printf("\n");
    }
}


// Função para liberar a memória da matriz
void liberar_matriz(int **matriz, int linhas) {
    for (int i = 0; i < linhas; i++) {
        free(matriz[i]); // Libera cada linha
    }
    free(matriz); // Libera o ponteiro principal
}