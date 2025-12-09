#define _CRT_SECURE_NO_WARNINGS
#if defined _WIN32
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PROTOPORT 27015
#define BUFFERSIZE 512

void ErrorHandler(char* errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    int port;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    else {
        port = PROTOPORT;
    }

    if (port < 0) {
        printf("bad port number %s\n", argv[1]);
        return 0;
    }

#if defined _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Error at WSAStartup()\n");
        return 0;
    }
#endif

    // CREAZIONE SOCKET UDP
    int Mysocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Mysocket < 0) {
        ErrorHandler("Creazione socket fallita\n");
        ClearWinSock();
        return -1;
    }

    // ASSEGNAMENTO INDIRIZZO ALLA SOCKET
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(port);

    if (bind(Mysocket, (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        ErrorHandler("bind() failed.\n");
        closesocket(Mysocket);
        ClearWinSock();
        return -1;
    }

    printf("Server UDP in ascolto sulla porta %d...\n", port);

    struct sockaddr_in cad;
    unsigned int clientLen;
    char buffer[BUFFERSIZE];

    // CICLO INFINITO PER GESTIRE LE RICHIESTE
    while (1) {
        clientLen = sizeof(cad);

        // Ricevi operazione dal client
        int bytesRecv = recvfrom(Mysocket, buffer, BUFFERSIZE, 0,
            (struct sockaddr*)&cad, &clientLen);

        if (bytesRecv < 0) {
            ErrorHandler("recvfrom() failed\n");
            continue;
        }

        printf("Handling client %s\n", inet_ntoa(cad.sin_addr));

        char operazione = buffer[0];
        char risOperazione[50];

        switch (operazione) {
        case 'A':
            strcpy(risOperazione, "Addizione");
            break;
        case 'S':
            strcpy(risOperazione, "Sottrazione");
            break;
        case 'M':
            strcpy(risOperazione, "Moltiplicazione");
            break;
        case 'D':
            strcpy(risOperazione, "Divisione");
            break;
        default:
            strcpy(risOperazione, "TERMINE");
            break;
        }

        // Invia risposta operazione
        if (sendto(Mysocket, risOperazione, strlen(risOperazione), 0,
            (struct sockaddr*)&cad, clientLen) < 0) {
            ErrorHandler("sendto() failed\n");
            continue;
        }

        if (operazione == 'T') {
            printf("Client terminato\n");
            continue;
        }

        // Ricevi i due numeri
        bytesRecv = recvfrom(Mysocket, buffer, BUFFERSIZE, 0,
            (struct sockaddr*)&cad, &clientLen);

        if (bytesRecv < 0) {
            ErrorHandler("recvfrom() failed per i numeri\n");
            continue;
        }

        int num1, num2;
        memcpy(&num1, buffer, sizeof(int));
        memcpy(&num2, buffer + sizeof(int), sizeof(int));

        double risultato;
        int errore = 0;

        switch (operazione) {
        case 'A':
            risultato = num1 + num2;
            break;
        case 'S':
            risultato = num1 - num2;
            break;
        case 'M':
            risultato = num1 * num2;
            break;
        case 'D':
            if (num2 != 0) {
                risultato = (double)num1 / num2;
            }
            else {
                errore = 1;
                risultato = 0.0;
            }
            break;
        }

        // Prepara risposta con flag errore e risultato
        char risposta[sizeof(int) + sizeof(double)];
        memcpy(risposta, &errore, sizeof(int));
        memcpy(risposta + sizeof(int), &risultato, sizeof(double));

        // Invia risultato
        if (sendto(Mysocket, risposta, sizeof(risposta), 0,
            (struct sockaddr*)&cad, clientLen) < 0) {
            ErrorHandler("sendto() failed per il risultato\n");
            continue;
        }
    }

    closesocket(Mysocket);
    ClearWinSock();
    return 0;
}