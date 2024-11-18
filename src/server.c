#include "common.h" // Header que provavelmente contém funções utilitárias, como logexit

#include <stdio.h>      // Biblioteca padrão de entrada e saída
#include <stdlib.h>     // Biblioteca para funções gerais, como alocação de memória e controle de processo
#include <string.h>     // Biblioteca para manipulação de strings
#include <unistd.h>     // Biblioteca para funções POSIX, como close()

#include <sys/socket.h> // Biblioteca para criação e manipulação de sockets
#include <sys/types.h>  // Biblioteca com definições de tipos usadas em sistemas POSIX

#define BUFSZ 1024      // Define o tamanho do buffer usado para comunicação

struct pos {
    int x;
    int y;
};

int definir_entrada_labirinto(int **labirinto, struct pos *pos, int linhas, int colunas){
    for(int i = 0; i < linhas; i++){
        for(int j = 0; j < colunas; j++){
            if(labirinto[i][j] == 2){
                pos->x = i;
                pos->y = j;
                return 1;
            }
        }
    }
    return 0;
}

// Função para verificar movimentos válidos
void verificar_movimentos(int **labirinto, int linhas, int colunas, struct pos pos, struct action *action) {
    int index = 0;

    // Verifica "Cima" (1)
    if (pos.x > 0 && 
        (labirinto[pos.x - 1][pos.y] == 1 || labirinto[pos.x - 1][pos.y] == 3)) {
        action->moves[index++] = 1;
    }

    // Verifica "Direita" (2)
    if (pos.y < colunas - 1 && 
        (labirinto[pos.x][pos.y + 1] == 1 || labirinto[pos.x][pos.y + 1] == 3)) {
        action->moves[index++] = 2;
    }

    // Verifica "Baixo" (3)
    if (pos.x < linhas - 1 && 
        (labirinto[pos.x + 1][pos.y] == 1 || labirinto[pos.x + 1][pos.y] == 3)) {
        action->moves[index++] = 3;
    }

    // Verifica "Esquerda" (4)
    if (pos.y > 0 && 
        (labirinto[pos.x][pos.y - 1] == 1 || labirinto[pos.x][pos.y - 1] == 3)) {
        action->moves[index++] = 4;
    }

    // Preenche o restante do vetor com 0
    for (int i = index; i < 100; i++) {
        action->moves[i] = 0;
    }
}

// Função usage: exibe uma mensagem de uso e encerra o programa se os argumentos forem inválidos
void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port> <-i> <input/in.txt>\n", argv[0]); // Exibe a forma correta de executar o programa
    printf("example: %s v4 51511 -i input/in.txt\n", argv[0]);             // Exemplo de execução
    exit(EXIT_FAILURE);                                   // Encerra o programa com erro
}

// Função principal
int main(int argc, char **argv) {
    // Verifica se o número de argumentos é suficiente; se não, chama a função usage
    if (argc < 5) {
        usage(argc, argv);
    }

    // Abertura do arquivo
    FILE *file = fopen(argv[4], "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    int linhas, colunas;

    // Primeira leitura para contar o número de linhas e colunas
    contar_dimensoes(file, &linhas, &colunas);

    // Aloca a matriz dinamicamente com as dimensões obtidas
    int **labirinto = alocar_matriz(linhas, colunas);

    // Segunda leitura para preencher a matriz com os dados do arquivo
    preencher_matriz(file, labirinto, linhas, colunas);

    // Exibe a matriz
    exibir_matriz(labirinto, linhas, colunas);

    // Inicializa o endereço do servidor
    struct sockaddr_storage storage; // Estrutura para armazenar o endereço
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) { // Chama função para configurar o endereço
        usage(argc, argv); // Se falhar, exibe a mensagem de uso e encerra
    }

    // Criação do socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0); // Cria um socket TCP
    if (s == -1) {
        logexit("socket"); // Em caso de erro, exibe mensagem e encerra o programa
    }

    // Configuração de opções do socket
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt"); // Em caso de erro na configuração, exibe mensagem e encerra o programa
    }

    // Associa o socket ao endereço especificado
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind"); // Em caso de erro, exibe mensagem e encerra o programa
    }

    // Coloca o socket em modo de escuta para aceitar conexões
    if (0 != listen(s, 10)) { // Configura o socket para aceitar até 10 conexões pendentes
        logexit("listen"); // Em caso de erro, exibe mensagem e encerra o programa
    }

    // Exibe o endereço ao qual o servidor está associado e aguarda conexões
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); // Converte o endereço para string
    printf("bound to %s, waiting connections\n", addrstr); // Exibe a mensagem

    // Define a posição de entrada do labirinto
    struct pos pos;
    definir_entrada_labirinto(labirinto, &pos, linhas, colunas);

    //printf("Linhas: %d  Colunas: %d\n", pos.x, pos.y);

    // Loop principal para aceitar e tratar conexões
    while (1) {
        struct sockaddr_storage cstorage;               // Armazena o endereço do cliente
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage); // Converte para sockaddr
        socklen_t caddrlen = sizeof(cstorage);

        // Aceita uma nova conexão do cliente
        int csock = accept(s, caddr, &caddrlen); // Cria um novo socket para comunicação com o cliente
        if (csock == -1) {
            logexit("accept"); // Em caso de erro, exibe mensagem e encerra o programa
        }

        // Exibe o endereço do cliente conectado
        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ); // Converte o endereço do cliente para string
        printf("client connected\n"); // Exibe o endereço do cliente

        // Recebe dados do cliente pela estrutura action
        struct action action;
        size_t count = recv(csock, &action, sizeof(action), MSG_WAITALL); // Recebe dados do cliente

        verificar_movimentos(labirinto, linhas, colunas, pos, &action);

        switch (action.type) {
            case ACTION_START:
                printf("starting new game\n");
                count = send(csock, &action, sizeof(action), 0); // Envia a resposta para o cliente
                break;
            case ACTION_MOVE:
                printf("Player moved.\n");
                break;
            case ACTION_MAP:
                printf("Sending map to client.\n");
                break;
            case ACTION_HINT:
                printf("Providing hint to client.\n");
                break;
            case ACTION_UPDATE:
                printf("Updating game state.\n");
                break;
            case ACTION_WIN:
                printf("Player won the game.\n");
                break;
            case ACTION_RESET:
                printf("Game reset.\n");
                break;
            case ACTION_EXIT:
                printf("Client exited the game.\n");
                break;
            default:
                printf("Unknown action type: %d\n", action.type);
        }       

        
        

        // Prepara e envia uma resposta ao cliente
        count = send(csock, &action, sizeof(action), 0); // Envia a resposta para o cliente
        if (count != sizeof(action)) {
            logexit("send"); // Em caso de erro ao enviar, exibe mensagem e encerra o programa
        }
        close(csock); // Fecha o socket da conexão com o cliente
    }

    close(s);
    fclose(file);
    // Libera a memória e fecha o arquivo
    liberar_matriz(labirinto, linhas);
   
    exit(EXIT_SUCCESS); // Encerra o programa com sucesso (nunca chega a este ponto devido ao loop)
}
