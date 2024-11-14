#include <stdio.h>
#include <stdlib.h>

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

// Função para liberar a memória da matriz
void liberar_matriz(int **matriz, int linhas) {
    for (int i = 0; i < linhas; i++) {
        free(matriz[i]); // Libera cada linha
    }
    free(matriz); // Libera o ponteiro principal
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <nome_do_arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return EXIT_FAILURE;
    }

    int linhas, colunas;

    // Primeira leitura para contar o número de linhas e colunas
    contar_dimensoes(file, &linhas, &colunas);

    // Aloca a matriz dinamicamente com as dimensões obtidas
    int **labirinto = alocar_matriz(linhas, colunas);

    // Segunda leitura para preencher a matriz com os dados do arquivo
    preencher_matriz(file, labirinto, linhas, colunas);

    // Exibe a matriz
    printf("Labirinto (%d linhas x %d colunas):\n", linhas, colunas);
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            printf("%d ", labirinto[i][j]);
        }
        printf("\n");
    }

    // Libera a memória e fecha o arquivo
    liberar_matriz(labirinto, linhas);
    fclose(file);

    return EXIT_SUCCESS;
}
