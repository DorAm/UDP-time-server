#include <iostream>
#include <string.h>
#include <winsock2.h> 

using namespace std;

#define TIME_PORT 27015
#define SERVER_IP "127.0.0.1"
//#define IP "192.168.203.19"

// === REQUESTS ===

const short NUM_OF_OPTIONS = 12;
const char * requests[NUM_OF_OPTIONS] = {
	"GetTime",
	"GetTimeWithoutDate",
	"GetTimeSinceEpoch",
	"GetClientToServerDelayEstimation",
	"MeasureRTT",
	"GetTimeWithoutDateOrSeconds",
	"GetYear",
	"GetMonthAndDay",
	"GetSecondsSinceBeginingOfMonth",
	"GetDayOfYear",
	"GetDaylightSaving",
	"Quit"
};

// === MESSAGES ===

const char * GET_TIME =				"Return time [YEAR-MONTH-DAY-HOUR-MIN-SEC]";
const char * GET_TIME_NO_DATE =		"Return time [HOUR-MIN-SEC]";
const char * GET_TIME_EPOCH =		"Return time in [SEC] since 1/1/1970";
const char * DELAY_ESTIMATION =		"Return the delay between the client and the server";
const char * MEASURE_RTT =			"Return the (RTT) Round Trip Time";
const char * GET_TIME_NO_DATE_SEC = "Return time [HOUR-MIN]";
const char * GET_YEAR =				"Return the [YEAR]";
const char * MONTH_DAY =			"Get the [MONTH] and the [DAY]";
const char * MONTH_SEC =			"Get the [SEC] pased since the begining of the current [MONTH]";
const char * GET_DAY_OF_YEAR =		"Get the number of [DAYS] passed since the beginning of the [YEAR]";
const char * DAYLIGHT_SAVING =		"Check if Daylight Saving is defined (1) or not defined (0)";
const char * QUIT =					"Quit the application";

const char * messages[NUM_OF_OPTIONS] = {
	GET_TIME , GET_TIME_NO_DATE, GET_TIME_EPOCH, DELAY_ESTIMATION,
	MEASURE_RTT, GET_TIME_NO_DATE_SEC, GET_YEAR, MONTH_DAY,
	MONTH_SEC, GET_DAY_OF_YEAR, DAYLIGHT_SAVING, QUIT
};

// === I/O FUNCTIONS ===

void printOptionsMenu() {

	cout << endl << "Time Server Application:" << endl;
	cout << "--------------------------------" << endl;
	for (size_t i = 1; i <= NUM_OF_OPTIONS; i++) {
		cout << i << ". " << messages[i - 1] << endl;
	}

	cout << endl << "Please enter your choice:" << endl;
}

const char * getUsersChoice() {

	short userInput;
	cin >> userInput;
	return requests[userInput - 1];
}

// === NETWORK FUNCTIONS ===

void configNetwork(SOCKET & connSocket, sockaddr_in &server) {

	// Initialize Winsock (Windows Sockets).
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
	}

	// Client side:
	// Create a socket and connect to an internet address.
	
	connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a client to communicate on a network, it must connect to a server.
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called server. 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_port = htons(TIME_PORT);

}

void sendRequest(SOCKET & connSocket, sockaddr_in &server, const char * request) {

	// The send function sends data on a connected socket.
	// The buffer to be sent and its size are needed.
	// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
	// The two last arguments hold the details of the server to communicate with. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).

	int bytesSent = 0;
	char sendBuff[255];

	// Copy user's request to the send buffer
	strcpy(sendBuff, request); 

	bytesSent = sendto(connSocket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)&server, sizeof(server));
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
		closesocket(connSocket);
		WSACleanup();
		return;
	}
}

char * recvResponse(SOCKET & connSocket, sockaddr_in &server) {

	int bytesRecv = 0;
	char recvBuff[255];

	// Gets the server's answer using simple recieve (no need to hold the server's address).
	bytesRecv = recv(connSocket, recvBuff, 255, 0);
	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(connSocket);
		WSACleanup();
		return nullptr;
	}

	recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
	cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

	return recvBuff;
}

// === DELAY & RTT ===

int GetClientToServerDelayEstimation(SOCKET & connSocket, sockaddr_in & server) {

	int prevClockTick = 0, currClockTick = 0;
	char * response = nullptr;
	int sum = 0, average = 0;
	int deltaTimes[99] = { 0 };

	// Send 100 requests to the server 
	for (size_t i = 0; i < 100; i++)
	{
		sendRequest(connSocket, server, "GetClientToServerDelayEstimation");
	}

	// Recieve 100 responses from server
	for (size_t i = 0; i < 100; i++)
	{
		response = recvResponse(connSocket, server);
		currClockTick = atoi(response);

		if (i == 0) {
			prevClockTick = currClockTick;
			continue;
		} 
		else {
			// calculate delta time between messages. Store the result in an deltaTimes array
			deltaTimes[i - 1] = currClockTick - prevClockTick;;
			prevClockTick = currClockTick;
		}
	}
	
	// Calculate average
	for (size_t i = 0; i < 99; i++)
	{
		sum += deltaTimes[i];
	}

	average = sum / 99;
	return average;
}

int MeasureRTT(SOCKET & connSocket, sockaddr_in & server) {

	int sendTime = 0, recvTime = 0, deltaTime = 0;
	char * response = nullptr;
	int sum = 0, average = 0;

	// Send 100 messages to the server 

	for (size_t i = 1; i <= 100; i++)
	{
		// send message i to server:
		sendTime = GetTickCount();
		sendRequest(connSocket, server, "MeasureRTT");

		// get response from server
		response = recvResponse(connSocket, server);
		recvTime = GetTickCount();

		// calculate delta time between send and recieve:
		deltaTime = recvTime - sendTime;
		sum += deltaTime;
	}

	average = sum / 99;
	return average;
}

// === MAIN ===

void main()
{

	// Configure a UDP socket & Server
	SOCKET connSocket;
	sockaddr_in server;
	configNetwork(connSocket, server);
	
	// Interfacing with the User:
	const char * userChoice;
	char * response;

	printOptionsMenu();
	userChoice = getUsersChoice();

	while (userChoice != "Quit") {

		if (strcmp(userChoice, "GetClientToServerDelayEstimation") == 0) {
			int average = 0;
			average = GetClientToServerDelayEstimation(connSocket, server);
			cout << "===========================================================================" << endl;
			cout << "Time Client: The average Client To Server Delay Estimation is: " << average << endl;
		}
		
		else if (strcmp(userChoice, "MeasureRTT") == 0) {
			int average = 0;
			average = MeasureRTT(connSocket, server);
			cout << "===========================================================================" << endl;
			cout << "Time Client: The average RTT is: " << average << endl;
		}

		else {
			// Send the user's request & Recieve the server's response
			sendRequest(connSocket, server, userChoice);
			response = recvResponse(connSocket, server);
		}

		// re-rendering the options menu
		system("pause");
		system("cls");
		printOptionsMenu();
		userChoice = getUsersChoice();
	}

	// Closing connections and Winsock
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);
}