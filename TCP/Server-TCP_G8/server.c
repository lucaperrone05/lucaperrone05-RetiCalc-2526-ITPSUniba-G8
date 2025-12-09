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
#include <stdlib.h> // for atoi()
#define PROTOPORT 27015 // default protocol port number
#define QLEN 6 // size of request queue

//Gestione errori
void ErrorHandler(char* errorMessage) {
	printf("%s", errorMessage);
}	

void ClearWinSock() {
#if defined _WIN32
	WSACleanup();
#endif
}

void handleTCPClient(int clientSocket, int Mysocket) {
	char messaggio[40] = "Connessione avvenuta con successo!";

	if (send(clientSocket, messaggio, strlen(messaggio), 0) < 0) {
		ErrorHandler("send() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}

	char operazione;
	char risOperazione[50];

	if (recv(clientSocket, &operazione, sizeof(operazione), 0) < 0) {
		ErrorHandler("recv() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}

	switch (operazione)
	{
	case 'A':
		strcpy_s(risOperazione, sizeof(risOperazione), "Addizione");
		break;
	case 'S':
		strcpy_s(risOperazione, sizeof(risOperazione), "Sottrazione");
		break;
	case 'M':
		strcpy_s(risOperazione, sizeof(risOperazione), "Moltiplicazione");
		break;
	case 'D':
		strcpy_s(risOperazione, sizeof(risOperazione), "Divisione");
		break;
	default:
		strcpy_s(risOperazione, sizeof(risOperazione), "TERMINE PROCESSO CLIENT");
		break;
	}

	if (send(clientSocket, risOperazione, strlen(risOperazione), 0) < 0) {
		ErrorHandler("send() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}

	int num1, num2;
	double risultato;

	if (recv(clientSocket, &num1, sizeof(num1), 0) < 0) {
		ErrorHandler("recv() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}

	if (recv(clientSocket, &num2, sizeof(num2), 0) < 0) {
		ErrorHandler("recv() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}

	switch (operazione)
	{
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
			double err = -999999;  // codice errore
			send(clientSocket, (char*)&err, sizeof(err), 0);
			
			closesocket(clientSocket);

			return;
		}
		break;
	}
	if (send(clientSocket, (char*)&risultato, sizeof(risultato), 0) < 0) {
		ErrorHandler("send() failed\n");
		closesocket(Mysocket);
		ClearWinSock();
		return -1;
	}
}



int main(int argc, char* argv[]) {
	int port;
	if (argc > 1) {
		port = atoi(argv[1]); // if argument specified convert argument to binary
	}
	else {
		port = PROTOPORT; // use default port number
	}

	if (port < 0) {
		printf("bad port number %s \n", argv[1]);
		return 0;
	}

#if defined WIN32 // initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		ErrorHandler("Error at WSAStartup()\n");
		return 0;
	}
#endif

	//CREAZIONE SOCKET
	int Mysocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (Mysocket < 0) {
		ErrorHandler("Crezione socket fallita");
		ClearWinSock();
		return -1;
	}

	//ASSEGNAMENTO INDIRIZZO ALLA SOCKET 
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

	//SOCKET IN ASCOLTO
	if (listen(Mysocket, QLEN) < 0) {
		ErrorHandler("listen() failed.\n");
		closesocket(Mysocket);
		return -1;
	}

	//ACCETTARE UNA NUOVA CONNESSIONE
	struct sockaddr_in cad;
	int clientSocket;
	int clientLen;
	printf("Waiting for a client to connect...\n");

	while (1) {
		clientLen = sizeof(cad);

		if ((clientSocket = accept(Mysocket, (struct sockaddr*)&cad, &clientLen)) < 0) {

			ErrorHandler("accept() failed\n");
			closesocket(Mysocket);
			ClearWinSock();
			return -1;
		}

		printf("Handling client %s\n", inet_ntoa(cad.sin_addr));
		handleTCPClient(clientSocket, Mysocket);
	}


	return 0;
}