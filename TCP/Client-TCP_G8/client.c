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
#define BUFFERSIZE 512
#define PROTOPORT 5193 // Numero di porta di default

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
		printf("error at WSASturtup\n");
		return -1;
	}
#endif

	//creazione socket
	int cSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cSocket < 0) {
		ErrorHandler("Creazione socket fallita");
		ClearWinSock();
		return -1;
	}

	char nomeServer[20];
	printf("Inserire il nome del server: ");
	fgets(nomeServer, sizeof(nomeServer), stdin);
	nomeServer[strcspn(nomeServer, "\n")] = '\0'; // rimuove newline

	struct hostent* host = gethostbyname(nomeServer);

	//connessione al server
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	memcpy(&sad.sin_addr, host->h_addr_list[0], host->h_length);
	sad.sin_port = htons(27015);

	if ((connect(cSocket, (struct sockaddr*)&sad, sizeof(sad)) < 0)) {
		ErrorHandler("connect() failed\n");
		closesocket(cSocket);
		ClearWinSock();
		return -1;
	}

	char buf[BUFFERSIZE];
	int byteRiceves = recv(cSocket, buf, BUFFERSIZE, 0);

	if (byteRiceves < 0) {
		ErrorHandler("recv() failed\n");
		closesocket(cSocket);
		ClearWinSock();
		return -1;
	}
	else {
		buf[(byteRiceves)] = '\0';
		printf("%s", buf);
	}

	char operazione;
	printf("\nInserire l'operazione da eseguire (A, S, M, D): ");
	scanf(" %c", &operazione);
	operazione = toupper((unsigned char)operazione);

	if (send(cSocket, &operazione,sizeof(operazione), 0) < 0) {
		ErrorHandler("send() failed\n");
		closesocket(cSocket);
		ClearWinSock();
		return -1;
	}
	
	memset(buf, 0, sizeof(buf));

	if (recv(cSocket, buf, BUFFERSIZE, 0) < 0) {
		ErrorHandler("recv() failed\n");
		closesocket(cSocket);
		ClearWinSock(); 
		return -1;
	}

	if (buf[0] == 'T'){
		printf("Termine del processo client.\n");
		closesocket(cSocket);
		ClearWinSock();
		return 0;
	}else {
		buf[(byteRiceves)] = '\0';
		printf("Operazione scelta: %s\n", buf);

		int num1, num2;
		printf("Inserire il primo numero: ");
		scanf("%d", &num1);
		printf("Inserire il secondo numero: ");
		scanf("%d", &num2);

		if (send(cSocket, &num1, sizeof(num1), 0) < 0) {
			ErrorHandler("send() failed\n");
			closesocket(cSocket);
			ClearWinSock();
			return -1;
		}

		if (send(cSocket, &num2, sizeof(num2), 0) < 0) {
			ErrorHandler("send() failed\n");
			closesocket(cSocket);
			ClearWinSock();
			return -1;
		}

		double risultato;

		if (recv(cSocket, (char*)&risultato, sizeof(risultato), 0) < 0) {
			ErrorHandler("recv() failed\n");
			closesocket(cSocket);
			ClearWinSock();
			return -1;
		}


		if (risultato == -999999) {
			printf("Errore: divisione per zero!\n");
		}
		else {
			printf("Risultato: %.2f\n", risultato);
		}

		printf("Termine del processo client.\n");
		closesocket(cSocket);
		ClearWinSock();
		return 0;
	}


	return 0;
}