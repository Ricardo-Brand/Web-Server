
# 🖥️ Web-Server (Servidor HTTP em C)

Implementação de um servidor HTTP simples em C, criada como projeto acadêmico para explorar os fundamentos de redes, sockets, protocolos HTTP e arquitetura de servidor.

##

## 📌 Sobre o Projeto

Este projeto demonstra como criar um servidor web básico em C utilizando sockets TCP/IP, capaz de atender requisições HTTP e retornar respostas estáticas.
O objetivo principal é entender conceitos de comunicação de rede de baixo nível, estrutura de servidores web e manipulação de protocolos.

##

## 🧠 Tecnologias e Conceitos Utilizados

💻 Linguagem C

🌐 Sockets TCP/IP

📡 Protocolos HTTP (GET)

🔄 Múltiplos clientes sequenciais

🧩 Manipulação de requisições e respostas

🛠️ Makefile para compilação

##

## 🚀 Funcionalidades

✔ Aceita conexões HTTP

✔ Recebe requisições GET

✔ Retorna respostas HTTP com conteúdo estático

✔ Registro simples de logs (requisições atendidas)

##

## 📥 Como Executar

### Passos

**1 - Clone o repositório:**

```http
git clone git@github.com:Ricardo-Brand/Web-Server.git
```

**2 - Entre na pasta:**

```http
cd Web-Server
```

**3 - Compile:**

```http
make
```

**4 - Rode o programa:**

```http
./webserver 8080
```

- 📌 Aqui `8080` é a porta em que o servidor vai escutar requisições. Você pode trocar por outra porta disponível.

##

## 📁 Estrutura do Projeto

```csharp
Web-Server/
├── src/
│   ├── main.c           # Função principal do servidor
│   ├── server.c         # Lógica de sockets e loop principal
│   └── server.h         # Declarações e helpers
├── public/              # Conteúdo estático a servir (HTML/CSS)
├── Makefile             # Compilação padrão
└── README.md
```

##

## 💡 Como Funciona (Resumo Técnico)

**1.** O servidor inicia e escuta na porta especificada

**2.** Aceita conexões TCP dos clientes

**3.** Lê requisições HTTP GET

**4.** Busca o arquivo requisitado em public/

**5.** Envia a resposta HTTP adequada

**6.** Fecha a conexão


##

## 🎓 O que Aprendi com este Projeto

- Conceitos básicos de redes e protocolos

- Uso de sockets em C

- Tratamento de requisições e respostas HTTP

- Organização modular em projetos C

- Uso de Makefile para build e automaçãos

##

## 📜 Licença

Este projeto está sob a licença MIT.
