#include "common.h" // Header que provavelmente contém funções utilitárias, como logexit

#include <stdio.h>      // Biblioteca padrão de entrada e saída
#include <stdlib.h>     // Biblioteca para funções gerais, como alocação de memória e controle de processo
#include <string.h>     // Biblioteca para manipulação de strings
#include <unistd.h>     // Biblioteca para funções POSIX, como close()

#include <sys/socket.h> // Biblioteca para criação e manipulação de sockets
#include <sys/types.h>  // Biblioteca com definições de tipos usadas em sistemas POSIX

#define BUFSZ 1024      // Define o tamanho do buffer usado para comunicação


int definir_entrada_labirinto(Labirinto *labirinto){
    for(int i = 0; i < labirinto->linhas; i++){
        for(int j = 0; j < labirinto->colunas; j++){
            if(labirinto->labirinto_completo[i][j] == 2){
                labirinto->jogador_x = i;
                labirinto->jogador_y = j;
                return 1;
            }
        }
    }
    return 0;
}

// Função para verificar movimentos válidos
void verificar_movimentos(Labirinto *labirinto, struct action *action) {
    int index = 0;

    // Verifica "Cima" (1)
    if (labirinto->jogador_x > 0 && 
        (labirinto->labirinto_completo[labirinto->jogador_x - 1][labirinto->jogador_y] == 1 || labirinto->labirinto_completo[labirinto->jogador_x - 1][labirinto->jogador_y] == 3)) {
        action->moves[index++] = 1;
    }

    // Verifica "Direita" (2)
    if (labirinto->jogador_y < labirinto->colunas - 1 && 
        (labirinto->labirinto_completo[labirinto->jogador_x][labirinto->jogador_y + 1] == 1 || labirinto->labirinto_completo[labirinto->jogador_x][labirinto->jogador_y + 1] == 3)) {
        action->moves[index++] = 2;
    }

    // Verifica "Baixo" (3)
    if (labirinto->jogador_x < labirinto->linhas - 1 && 
        (labirinto->labirinto_completo[labirinto->jogador_x + 1][labirinto->jogador_y] == 1 || labirinto->labirinto_completo[labirinto->jogador_x + 1][labirinto->jogador_y] == 3)) {
        action->moves[index++] = 3;
    }

    // Verifica "Esquerda" (4)
    if (labirinto->jogador_y > 0 && 
        (labirinto->labirinto_completo[labirinto->jogador_x][labirinto->jogador_y - 1] == 1 || labirinto->labirinto_completo[labirinto->jogador_x][labirinto->jogador_y - 1] == 3)) {
        action->moves[index++] = 4;
    }

    // Preenche o restante do vetor com 0
    for (int i = index; i < 100; i++) {
        action->moves[i] = 0;
    }
}


// Função para movimentar no tabuleiro
int fazer_movimentos(Labirinto *labirinto, struct action *action){
    int x = labirinto->jogador_x;
    int y = labirinto->jogador_y;

    int novo_x = x, novo_y = y;

    printf("Move no Labirinto: %d\n", action->moves[0]);

    switch (action->moves[0]) {
        case 1: novo_x = x - 1; break; // Cima
        case 2: novo_y = y + 1; break; // Direita
        case 3: novo_x = x + 1; break; // Baixo
        case 4: novo_y = y - 1; break; // Esquerda
        default: return 0; // Movimento inválido
    }

    printf("Move: (%d,%d)\n", novo_x,novo_y);

    if (labirinto->labirinto_completo[novo_x][novo_y] == 0) {
        return 0; // Movimento inválido (muro)
    }

    if (labirinto->labirinto_completo[novo_x][novo_y] == 3) {
        return 2; // Jogador chegou na saída
    }



    // Atualiza a posição
    if(labirinto->labirinto_completo[x][y] != 2){
        labirinto->labirinto_completo[x][y] = 1; // Marca a posição anterior como livre
    }
    labirinto->labirinto_completo[novo_x][novo_y] = 5; // Marca a nova posição como o jogador
    labirinto->jogador_x = novo_x;
    labirinto->jogador_y = novo_y;

    // Atualiza o raio de visibilidade ao redor do jogador
    for (int i = novo_x - 1; i <= novo_x + 1; i++) {
        for (int j = novo_y - 1; j <= novo_y + 1; j++) {
            // Verifica se a célula está dentro dos limites do labirinto
            if (i >= 0 && i < labirinto->linhas && j >= 0 && j < labirinto->colunas) {
                labirinto->labirinto_descoberto[i][j] = labirinto->labirinto_completo[i][j];
            }
        }
    }

    return 1; // Movimento bem-sucedido
}
void  map_descoberto(Labirinto *labirinto, struct action *action){
    action->moves[9] = labirinto->linhas;
    action->moves[10] = labirinto->colunas;

    for(int i = 0; i < labirinto->linhas; i++){
        for(int j = 0; j < labirinto->colunas; j++){
            action->board[i][j] = labirinto->labirinto_descoberto[i][j];
        }
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

    Labirinto labirinto;
    // Primeira leitura para contar o número de linhas e colunas
    contar_dimensoes(file, &labirinto);

    // Aloca a matriz dinamicamente com as dimensões obtidas
    alocar_matriz(&labirinto);

    // Segunda leitura para preencher a matriz com os dados do arquivo
    preencher_matriz(file,&labirinto);

    // Exibe a matriz
    exibir_matriz(&labirinto);

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

    definir_entrada_labirinto(&labirinto);

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

    // Loop principal para aceitar e tratar conexões
    while (1) {
        // Recebe dados do cliente pela estrutura action
        struct action action;
        size_t count = recv(csock, &action, sizeof(action), MSG_WAITALL); // Recebe dados do cliente
        if (count != sizeof(action)) {
            logexit("recv");
        }
        printf("Recebido Servidor: type=%d, moves[0]=%d\n", action.type, action.moves[0]);

        switch (action.type) {
            case ACTION_START:
                printf("starting new game\n");
                verificar_movimentos(&labirinto, &action);
                break;
            case ACTION_MOVE:
                printf("Player moved.\n");
                fazer_movimentos(&labirinto, &action);
                verificar_movimentos(&labirinto, &action);
                break;
            case ACTION_MAP:
                printf("Map:\n");
                map_descoberto(&labirinto, &action);
                break;
            case ACTION_EXIT:
                printf("client disconnected\n");
                break;
            default:
                printf("Unknown action type: %d\n", action.type);
        }  


        count = send(csock, &action, sizeof(action), 0); // Envia a resposta para o cliente
        if (count != sizeof(action)) {
            logexit("send");
        } 
        printf("Enviado Servidor: type=%d, moves[0]=%d\n", action.type, action.moves[0]);
    }



    close(csock); // Fecha o socket da conexão com o cliente
    close(s);
    fclose(file);
    // Libera a memória e fecha o arquivo
    liberar_matriz(&labirinto);
   
    exit(EXIT_SUCCESS); // Encerra o programa com sucesso (nunca chega a este ponto devido ao loop)
}
