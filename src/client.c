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
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ); // Limpa o buffer para evitar resíduos de dados
    printf("mensagem> ");
    fgets(buf, BUFSZ-1, stdin); // Lê a mensagem do usuário até BUFSZ-1 caracteres
    size_t count = send(s, buf, strlen(buf)+1, 0); // Envia a mensagem ao servidor
    if (count != strlen(buf)+1) {
        logexit("send"); // Em caso de erro no envio, exibe mensagem e encerra o programa
    }

    // Recepção de resposta do servidor
    memset(buf, 0, BUFSZ); // Limpa o buffer para receber a resposta do servidor
    unsigned total = 0;
    while (1) {
        count = recv(s, buf + total, BUFSZ - total, 0); // Recebe dados do servidor
        if (count == 0) {
            // A conexão foi encerrada pelo servidor
            break;
        }
        total += count; // Atualiza o total de bytes recebidos
    }
    close(s); // Fecha o socket após o término da comunicação

    // Exibe o total de bytes recebidos e a resposta do servidor
    printf("received %u bytes\n", total);
    puts(buf); // Exibe o conteúdo da mensagem recebida

    exit(EXIT_SUCCESS); // Encerra o programa com sucesso
}
