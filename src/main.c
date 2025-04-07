#include <sys/socket.h> // Criação e manipulação de sockets
#include <netinet/in.h> // Estruturas para endereços de rede (sockaddr_in)
#include <arpa/inet.h> // Funções para manipular endereços IP (inet_pton, inet_ntoa)
#include <unistd.h> // Função close() para fechar sockets
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8080
#define BUFFER_SIZE 1024

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
        sscanf(buffer, "%s %s %s", metodo, caminho_requisitado, protocolo);

        char caminho[1024];
        const char *pasta_base = "./public";
        snprintf(caminho, sizeof(caminho), "%s%s", pasta_base, caminho_requisitado);

        if (strstr(caminho_requisitado, "..") !=
            NULL) { // verifica se no caminho até o arquivo não tem nenhum ' .. '
            char response[BUFFER_SIZE];
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

        struct stat file_stat;
        if (stat(caminho, &file_stat) == -1) { // verificar o caminho é válido
            char response[BUFFER_SIZE];
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

        if (S_ISDIR(file_stat.st_mode)) { // Se for diretório, adiciona index.html
            strncat(caminho, "/index.html", sizeof(caminho) - strlen(caminho) - 1);
            if (stat(caminho, &file_stat) ==
                -1) { // verifica se index.html existe naquele diretório
                char response[BUFFER_SIZE];
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

        FILE *fp = fopen(caminho, "rb"); // executa o arquivo em binário
        if (!fp) {
            char response[BUFFER_SIZE];
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

        char conteudo[BUFFER_SIZE];
        int lidos = fread(conteudo, 1, sizeof(conteudo) - 1, fp);
        conteudo[lidos] = '\0';
        fclose(fp);

        const char *content_type = "text/plain";
        if (strstr(caminho, ".html"))
            content_type = "text/html";
        else if (strstr(caminho, ".css"))
            content_type = "text/css";
        else if (strstr(caminho, ".js"))
            content_type = "application/javascript";

        char response[BUFFER_SIZE];
        snprintf(
                response, sizeof(response),
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %d\r\n\r\n",
                content_type, lidos
        );

        write(client_fd, response, strlen(response));
        write(client_fd, conteudo, lidos);

        close(client_fd);
    }

    close(meu_socket);
    return 0;
}
// caso o GET seja apenas ' / ' , mostra o arquivo da pasta public/index.html
// caso seja GET /site , deve mostrar o arquivo public/site/index.html
// caso seja GET /recursos , como não existe index.html nessa pasta, deve retornar '404 not found''
// caso seja um arquivo, deve retornar o arquivo