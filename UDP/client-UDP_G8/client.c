#define _CRT_SECURE_NO_WARNINGS
#if defined _WIN32
#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFERSIZE 512
#define PROTOPORT 27015

void ErrorHandler(char* errorMessage) {
    printf("%s", errorMessage);
}

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

int main(void) {
#if defined _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("error at WSAStartup\n");
        return -1;
    }
#endif

    // CREAZIONE SOCKET UDP
    int cSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (cSocket < 0) {
        ErrorHandler("Creazione socket fallita\n");
        ClearWinSock();
        return -1;
    }

    char nomeServer[20];
    printf("Inserire il nome del server: ");
    fgets(nomeServer, sizeof(nomeServer), stdin);
    nomeServer[strcspn(nomeServer, "\n")] = '\0';

    struct hostent* host = gethostbyname(nomeServer);
    if (host == NULL) {
        ErrorHandler("gethostbyname() failed\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    // CONFIGURAZIONE INDIRIZZO SERVER
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    memcpy(&sad.sin_addr, host->h_addr_list[0], host->h_length);
    sad.sin_port = htons(PROTOPORT);

    char operazione;
    printf("Inserire l'operazione da eseguire (A, S, M, D): ");
    scanf(" %c", &operazione);
    operazione = toupper((unsigned char)operazione);

    // Invia operazione
    if (sendto(cSocket, &operazione, sizeof(operazione), 0,
        (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        ErrorHandler("sendto() failed per operazione\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    // Ricevi conferma operazione
    char buf[BUFFERSIZE];
    struct sockaddr_in fromAddr;
    unsigned int fromSize = sizeof(fromAddr);

    int bytesRecv = recvfrom(cSocket, buf, BUFFERSIZE, 0,
        (struct sockaddr*)&fromAddr, &fromSize);

    if (bytesRecv < 0) {
        ErrorHandler("recvfrom() failed\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    buf[bytesRecv] = '\0';

    if (strcmp(buf, "TERMINE") == 0) {
        printf("Termine del processo client.\n");
        closesocket(cSocket);
        ClearWinSock();
        return 0;
    }

    printf("Operazione scelta: %s\n", buf);

    int num1, num2;
    printf("Inserire il primo numero: ");
    scanf("%d", &num1);
    printf("Inserire il secondo numero: ");
    scanf("%d", &num2);

    // Invia i due numeri in un unico datagram
    char numBuffer[sizeof(int) * 2];
    memcpy(numBuffer, &num1, sizeof(int));
    memcpy(numBuffer + sizeof(int), &num2, sizeof(int));

    if (sendto(cSocket, numBuffer, sizeof(numBuffer), 0,
        (struct sockaddr*)&sad, sizeof(sad)) < 0) {
        ErrorHandler("sendto() failed per i numeri\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    // Ricevi risultato con flag errore
    char risposta[sizeof(int) + sizeof(double)];
    fromSize = sizeof(fromAddr);

    bytesRecv = recvfrom(cSocket, risposta, sizeof(risposta), 0,
        (struct sockaddr*)&fromAddr, &fromSize);

    if (bytesRecv < 0) {
        ErrorHandler("recvfrom() failed per il risultato\n");
        closesocket(cSocket);
        ClearWinSock();
        return -1;
    }

    int errore;
    double risultato;
    memcpy(&errore, risposta, sizeof(int));
    memcpy(&risultato, risposta + sizeof(int), sizeof(double));

    if (errore) {
        printf("Errore: Divisione per zero!\n");
    }
    else {
        printf("Risultato: %.2f\n", risultato);
    }

    printf("Termine del processo client.\n");
    closesocket(cSocket);
    ClearWinSock();
    return 0;
}