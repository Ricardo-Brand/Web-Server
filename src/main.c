#include <sys/socket.h> // Criação e manipulação de sockets
#include <netinet/in.h> // Estruturas para endereços de rede (sockaddr_in)
#include <arpa/inet.h> // Funções para manipular endereços IP (inet_pton, inet_ntoa)
#include <unistd.h> // Função close() para fechar sockets
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8080
#define BUFFER_SIZE 65536

int main(int argc, char *argv[]) {
    int meu_socket, client_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    meu_socket = socket(AF_INET, SOCK_STREAM, 0); // criar um socket

    if (meu_socket == -1) {
        perror("Error ao criar socket!!\n");
        return -1;
    }

    int opt = 1;
    if (setsockopt(meu_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
        -1) { // definir como será o comportamento do socket
        perror("Erro em setsockopt");
        close(meu_socket);
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(meu_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
        -1) { // vincula o socket a um endereço ip ou porta
        perror("Erro ao fazer bind\n");
        close(meu_socket);
        return -1;
    }

    if (listen(meu_socket, 20) ==
        -1) { // valida o socket para acesso a conexões, com um tamanho limitado de conexões simultâneas
        perror("Erro ao fazer listen\n");
        close(meu_socket);
        return -1;
    }

    printf("\nServidor rodando em: HTTP://127.0.0.1:%d\n\n", PORT);

    while (1) {
        client_fd = accept(
                meu_socket, (struct sockaddr *)&client_addr, &client_len
        ); // serve para aceitar a conexão com o cliente e aguarda até alguém se conectar denovo para prosseguir para a próxima linha de código
        if (client_fd == -1) {
            perror("Erro quando o cliente tentou se conectar ao servidor\n");
            close(meu_socket);
            close(client_fd);
            return -1;
        }

        read(client_fd, buffer, BUFFER_SIZE - 1);

        printf("Conexão aceita de %s:%d\n\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        printf("Requisição recebida:\n%s\n", buffer);

        char metodo[8], caminho_requisitado[512], protocolo[16];
        char response[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", metodo, caminho_requisitado, protocolo);

        if (strstr(caminho_requisitado, "/../") != NULL ||
            strncmp(caminho_requisitado, "../", 3) == 0 ||
            strstr(caminho_requisitado, "/..") ==
                    (caminho_requisitado + strlen(caminho_requisitado) - 3
                    )) { // verifica apenas se está tentando um diretório pai

            snprintf(
                    response, sizeof(response),
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>404 - Arquivo nao encontrado</h1>"
            );

            write(client_fd, response, strlen(response));
            close(client_fd);
            continue;
        }
        char caminho[1024];
        const char *pasta_base = "./public";
        snprintf(caminho, sizeof(caminho), "%s%s", pasta_base, caminho_requisitado);

        struct stat file_stat;
        if (stat(caminho, &file_stat) == -1) { // verifica se o caminho é válido

            snprintf(
                    response, sizeof(response),
                    "HTTP/1.0 404 Not Found\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>404 - Arquivo nao encontrado</h1>"
            );
            write(client_fd, response, strlen(response));
            close(client_fd);
            continue;
        }

        if (S_ISDIR(file_stat.st_mode
            )) { // Testa se é um diretório, caso seja um diretório, irá procurar por padrão "index.html"
            strncat(caminho, "/index.html", sizeof(caminho) - strlen(caminho) - 1);
            if (stat(caminho, &file_stat) ==
                -1) { // verifica se index.html existe naquele diretório

                snprintf(
                        response, sizeof(response),
                        "HTTP/1.0 404 Not Found\r\n"
                        "Content-Type: text/html\r\n\r\n"
                        "<h1>404 - Arquivo nao encontrado</h1>"
                );
                write(client_fd, response, strlen(response));
                close(client_fd);
                continue;
            }
        }

        FILE *fp = fopen(caminho, "rb"); // abre o arquivo em binário
        if (!fp) { // verifica se esse arquivo pode foi aberto
            snprintf(
                    response, sizeof(response),
                    "HTTP/1.0 500 Internal Server Error\r\n"
                    "Content-Type: text/html\r\n\r\n"
                    "<h1>500 - Erro ao abrir o arquivo</h1>"
            );
            write(client_fd, response, strlen(response));
            close(client_fd);
            continue;
        }

        const char *content_type = "text/plain";
        if (strstr(caminho, ".html"))
            content_type = "text/html";
        else if (strstr(caminho, ".css"))
            content_type = "text/css";
        else if (strstr(caminho, ".js"))
            content_type = "application/javascript";

        long tamanho_arquivo = file_stat.st_size;
        snprintf(
                response, sizeof(response),
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n\r\n",
                content_type, tamanho_arquivo
        );
        write(client_fd, response, strlen(response));

        size_t lidos;
        while ((lidos = fread(buffer, 1, sizeof(buffer), fp)) > 0
        ) { // irá fazer a leitura do arquivo inteiro, do inicio ao fim, independente de tamanho
            if (write(client_fd, buffer, lidos) != lidos) {
                perror("Erro ao enviar dados");
                break;
            }
        }
        fclose(fp);

        close(client_fd);
    }

    close(meu_socket);
    return 0;
}
// caso o GET seja apenas ' / ' , mostra o arquivo da pasta public/index.html
// caso seja GET /site , deve mostrar o arquivo public/site/index.html
// caso seja GET /recursos , como não existe index.html nessa pasta, deve retornar '404 not found''
// caso seja um arquivo, deve retornar o arquivo