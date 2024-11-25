#include "common.h" // Header que provavelmente contém funções auxiliares, como addrparse e logexit

#include <stdlib.h>      // Biblioteca padrão para alocação de memória e controle de processos
#include <stdio.h>       // Biblioteca para funções de entrada e saída
#include <string.h>      // Biblioteca para manipulação de strings
#include <unistd.h>      // Biblioteca para funções POSIX, como close()

#include <sys/types.h>   // Definições de tipos para sistemas POSIX
#include <sys/socket.h>  // Biblioteca para criação e manipulação de sockets
#include <arpa/inet.h>   // Biblioteca para funções de conversão de endereços de rede

// Função usage: exibe mensagem de uso e encerra o programa se os argumentos forem inválidos
void usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]); // Exibe a forma correta de executar o programa
    printf("example: %s 127.0.0.1 51511\n", argv[0]);          // Exemplo de execução com IP e porta
    exit(EXIT_FAILURE);                                       // Encerra o programa com erro
}


#define BUFSZ 1024 // Define o tamanho do buffer usado para enviar e receber mensagens


void imprime_possible_moves(struct action action, const char *direcoes[], char *possible_moves[]) {
    int primeira = 1; // Flag para controlar a vírgula antes dos movimentos
    if(action.type != ACTION_HINT){
        printf("Possible moves:");
    }

    for (int i = 0; i < 100 && action.moves[i] != 0; i++) { // Itera até o final dos movimentos válidos
        if (!primeira) {
            printf(", "); // Adiciona a vírgula após o primeiro movimento
        }
        possible_moves[i] = (char *)direcoes[action.moves[i] - 1];
        printf(" %s", direcoes[action.moves[i] - 1]); // Imprime a direção correspondente
        primeira = 0; // Após o primeiro movimento, define a flag para 0
    }

    printf(".\n");
}

// Função para verificar a entrada de acordo com os movimentos possíveis
int verificar_possible(struct action action, char *possible_moves[], char *msg){
    for (int i = 0; i < 100 && action.moves[i] != 0; i++){
        if(strcmp(msg, possible_moves[i]) == 0){
            return 1;
        }
    }
    return 0;
}


int verificar_reset_exit(const char *win_moves[], const char *msg){
    for(int i = 0; i < 2; i++){
        if(strcmp(msg, win_moves[i]) == 0){
            return 1;
        }
    }
    return 0;
}


// Função para verificar se a entrada é um comando de movimento
int is_movement_command(const char *msg, const char *direcoes[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(msg, direcoes[i]) == 0) {
            return i + 1; // Retorna o índice ajustado para o enum (1-based)
        }
    }
    return 0; // Não é comando de movimento
}

// Função para processar comandos gerais
int process_game_command(const char *msg, const char *action_strings[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(msg, action_strings[i]) == 0) {
            return i; // Retorna o índice correspondente ao comando
        }
    }
    return -1; // Comando não encontrado
}


void preenche_moves(struct action *action){
    for(int i = 0; i < 100; i++){
        action->moves[i] = 0;
    }
}


void imprimir_matriz_descoberta(struct action *action){
    const char simbolos[] = {'#', '_', '>', 'X', '?', '+'}; // Mapeamento dos inteiros para caracteres
    
    for (int i = 0; i < action->moves[9]; i++) {
        for (int j = 0; j < action->moves[10]; j++) {
            printf("%c\t ", simbolos[action->board[i][j]]);
        }
        printf("\n");
    }
}

// Função principal
int main(int argc, char **argv) {
    // Verifica se o número de argumentos é suficiente; se não, chama a função usage
    if (argc < 3) {
        usage(argc, argv);
    }

    // Configura o endereço do servidor
    struct sockaddr_storage storage; // Estrutura para armazenar o endereço do servidor
    if (0 != addrparse(argv[1], argv[2], &storage)) { // Chama a função para configurar o endereço do servidor
        usage(argc, argv); // Se falhar, exibe a mensagem de uso e encerra o programa
    }

    // Criação do socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0); // Cria um socket TCP
    if (s == -1) {
        logexit("socket"); // Em caso de erro, exibe mensagem e encerra o programa
    }

    // Conexão com o servidor
    struct sockaddr *addr = (struct sockaddr *)(&storage); // Converte o endereço para tipo sockaddr
    if (0 != connect(s, addr, sizeof(storage))) { // Tenta estabelecer uma conexão com o servidor
        logexit("connect"); // Em caso de erro na conexão, exibe mensagem e encerra o programa
    }

    // Exibe uma mensagem de conexão estabelecida com o endereço do servidor
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); // Converte o endereço do servidor para string
    //printf("connected to %s\n", addrstr); // Exibe o endereço do servidor conectado

    // Envio de mensagem para o servidor
    
    struct action action;
    const char *direcoes[] = {"up", "right", "down", "left"}; // Mapeamento dos movimentos
    const char *action_strings[] = {                          // Mapaeamentos dos comandos do jogador
        "start",
        "move",
        "map",
        "hint",
        "update",
        "win",
        "reset",
        "exit"
    };

    char *possible_moves[4];
    const char *win_moves[] = {"exit", "reset"};

    char msg[20];

    size_t count;
    int controle = 1;

    int game_started = 0;

    int move_index = 0;
    while(controle == 1){
        printf("> ");
        scanf("%s", msg); // Lê a mensagem do usuário 

        // Verifica se o jogo já foi iniciado
        if (!game_started && strcmp(msg, "start") != 0) {
            printf("error: start the game first\n");
            continue;
        }

        if(game_started && strcmp(msg, "start") == 0){
            printf("error: game already started\n");
            continue;
        }
        // Após vencer o jogo possíveis comandos são exit e reset
        if(action.type == ACTION_WIN && game_started){
            if(verificar_reset_exit(win_moves, msg) != 1){
                printf("error: command exit or reset\n");
                continue;
            }
        }
        
        // Verifica se é um comando de movimento
        move_index = is_movement_command(msg, direcoes, 4);
        if (move_index) {
            if (verificar_possible(action, possible_moves, msg)) {
                preenche_moves(&action);
                action.type = ACTION_MOVE;
                action.moves[0] = move_index;
            } else {
                printf("error: you cannot go this way\n");
                continue;
            }
        } else {
            // Trata comandos gerais
            action.type = process_game_command(msg, action_strings, 8);
            if (action.type == -1) {
                printf("error: command not found\n");
                continue;
            }
        }

        // Envia a mensagem ao servidor
        count = send(s, &action, sizeof(action), 0); 
        if (count != sizeof(action)) {
            logexit("send");
        }
        

        // Recepção de resposta do servidor
        count = recv(s, &action, sizeof(action), MSG_WAITALL);
        if (count != sizeof(action)) {
            logexit("recv");
        }
        

        switch(action.type){
            case ACTION_START:
                game_started = 1;
                imprime_possible_moves(action, direcoes, possible_moves); // imprime possíveis direções
                break;
            case ACTION_MOVE:
                imprime_possible_moves(action, direcoes, possible_moves); // imprime possíveis direções
                break;
            case ACTION_MAP:
                imprimir_matriz_descoberta(&action);
                break;
            case ACTION_HINT:
                printf("Hint:");
                imprime_possible_moves(action, direcoes, possible_moves);
                break;
            case ACTION_WIN:
                printf("You escaped!\n");
                imprimir_matriz_descoberta(&action);
                break;
            case ACTION_RESET:
                imprime_possible_moves(action, direcoes,possible_moves); // imprime possíveis direções
                break;
            case ACTION_EXIT:
                controle = 0;
                break;
        }
        
           
        
        //limpa o buffer de mensagem
        memset(msg, 0, sizeof(msg));
    }
   
    close(s); // Fecha o socket após o término da comunicação

    exit(EXIT_SUCCESS); // Encerra o programa com sucesso
}
