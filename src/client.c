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


void imprime_possible_moves(struct action action, const char *direcoes[]) {
    int primeira = 1; // Flag para controlar a vírgula antes dos movimentos
    printf("Possible moves:");

    for (int i = 0; i < 100 && action.moves[i] != 0; i++) { // Itera até o final dos movimentos válidos
        if (!primeira) {
            printf(", "); // Adiciona a vírgula após o primeiro movimento
        }
        printf(" %s", direcoes[action.moves[i] - 1]); // Imprime a direção correspondente
        primeira = 0; // Após o primeiro movimento, define a flag para 0
    }

    printf(".\n");
}

// Função para mapear o input para o comando númerico
void convert_string_enum(const char *strings[], struct action *action, const char *msg, int size){
    for(int i = 0; i < size; i++){
        if(strcmp(msg, strings[i]) == 0){
            action->type = (ActionType)i;
        }
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
    printf("connected to %s\n", addrstr); // Exibe o endereço do servidor conectado

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

    char msg[20];

    size_t count;

    while(1){
        printf("> ");
        scanf("%s", msg); // Lê a mensagem do usuário 

        convert_string_enum(action_strings, &action, msg, sizeof(action_strings) / sizeof(action_strings[0]));


        count = send(s, &action, sizeof(action), 0); // Envia a mensagem ao servidor
        if (count != sizeof(action)){
            logexit("send"); // Em caso de erro no envio, exibe mensagem e encerra o programa
        }

        // Recepção de resposta do servidor
        count = recv(s, &action, sizeof(action), MSG_WAITALL);
        imprime_possible_moves(action, direcoes); // imprime possíveis direções
        memset(msg, 0, sizeof(msg));
    }
   
    close(s); // Fecha o socket após o término da comunicação

    exit(EXIT_SUCCESS); // Encerra o programa com sucesso
}
