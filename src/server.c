#include "common.h" // Header que provavelmente contém funções utilitárias, como logexit

#include <stdio.h>      // Biblioteca padrão de entrada e saída
#include <stdlib.h>     // Biblioteca para funções gerais, como alocação de memória e controle de processo
#include <string.h>     // Biblioteca para manipulação de strings
#include <unistd.h>     // Biblioteca para funções POSIX, como close()

#include <sys/socket.h> // Biblioteca para criação e manipulação de sockets
#include <sys/types.h>  // Biblioteca com definições de tipos usadas em sistemas POSIX

#define BUFSZ 1024      // Define o tamanho do buffer usado para comunicação

// Função usage: exibe uma mensagem de uso e encerra o programa se os argumentos forem inválidos
void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]); // Exibe a forma correta de executar o programa
    printf("example: %s v4 51511\n", argv[0]);             // Exemplo de execução
    exit(EXIT_FAILURE);                                   // Encerra o programa com erro
}

// Função principal
int main(int argc, char **argv) {
    // Verifica se o número de argumentos é suficiente; se não, chama a função usage
    if (argc < 3) {
        usage(argc, argv);
    }

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
        printf("[log] connection from %s\n", caddrstr); // Exibe o endereço do cliente

        // Recebe dados do cliente
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ); // Inicializa o buffer
        size_t count = recv(csock, buf, BUFSZ - 1, 0); // Recebe dados do cliente
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf); // Exibe a mensagem recebida

        // Prepara e envia uma resposta ao cliente
        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr); // Formata a resposta com o endereço do cliente
        count = send(csock, buf, strlen(buf) + 1, 0); // Envia a resposta para o cliente
        if (count != strlen(buf) + 1) {
            logexit("send"); // Em caso de erro ao enviar, exibe mensagem e encerra o programa
        }
        close(csock); // Fecha o socket da conexão com o cliente
    }

    exit(EXIT_SUCCESS); // Encerra o programa com sucesso (nunca chega a este ponto devido ao loop)
}
