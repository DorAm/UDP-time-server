#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <map>
using namespace std;

#define TIME_PORT 27015

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

// === MAPING ===
//typedef char * (*FnPtr)();

// A helper function for mapping function names to function pointers
// this reflection makes easy access to functions by name

//auto createMap() {
//
//	std::map<std::string, FnPtr> FunctionMap;
//	for (size_t i = 0; i < NUM_OF_OPTIONS; i++) {
//
//		FunctionMap[requests[i]] = (FnPtr)requests[i];
//	}
//
//	return FunctionMap;
//}

// === SERVER FUNCTIONS ===

//char * GetTime() {
//	
//	char * response = new char[255];
//
//	// Get the current time.
//	time_t timer;
//	time(&timer);
//
//	// Parse the current time to printable string.
//	strcpy(response, ctime(&timer));
//
//	// remove the new-line from the created string
//	response[strlen(response) - 1] = '\0';
//
//	return response;
//}

char * genericGetTime(const char * format) {
	char * response = new char[255];

	// Prepare a raw timer and a calendar struct tm
	time_t timer;
	struct tm * timeinfo;
	char buffer[80];

	// Save current time
	time(&timer);
	timeinfo = localtime(&timer);

	// Convert to time representation 
	strftime(response, 50, format, timeinfo); // TODO: check why max is 50 & don't we need to add null termination ?
	return response;
}

char * GetTimeSinceEpoch() {

	char * response = new char[255];

	time_t timer;
	time(&timer);

	// Parse the timer to a string.
	itoa((int)timer, response, 10); //don't we need to add null termination ?

	return response;
}

char * WhatIsTheTime() {
	char * response = new char[255];
	
	// Retrieves the number of milliseconds that have elapsed since the system was started
	int tickCount = GetTickCount();

	itoa(tickCount, response, 10);
	return response;
}

char * GetSecondsSinceBeginingOfMonth() {

	char * response = new char[255];
	int result = 0;

	// Get the current time.
	time_t timer;
	time(&timer);

	struct tm * timeinfo = localtime(&timer);
	result = (timeinfo->tm_mday) * 24 * 60 * 60 + (timeinfo->tm_hour) * 60 * 60 + (timeinfo->tm_min) * 60 + timeinfo->tm_sec;

	itoa(result, response, 10);
	return response;
}

char * GetDayOfYear() {

	char * response = new char[255];
	int result = 0;

	// Get the current time.
	time_t timer;
	time(&timer);

	struct tm * timeinfo = localtime(&timer);
	result = (timeinfo->tm_yday);

	itoa(result, response, 10);
	return response;
}

char * GetDaylightSaving() {

	char * response = new char[255];

	// Prepare a raw timer and a calendar struct tm
	time_t timer;
	struct tm * timeinfo;
	char buffer[80];

	// Save current time
	time(&timer);
	timeinfo = localtime(&timer);

	// use the tm_isdst flag to check Daylightsaving
	int res = timeinfo->tm_isdst;

	// Parse the result to a string.
	itoa(res, response, 10);
	return response;
}


// === ROUTER ===

char * processRequest(char * request) {

	// TODO: CLEAN THE MEMMORY!!!
	char * response = nullptr;

	if (strcmp(request, "GetTime") == 0) {
		response = genericGetTime("%c");
	}
	else if (strcmp(request, "GetTimeWithoutDate") == 0) {
		response = genericGetTime("%X");
	}
	else if (strcmp(request, "GetTimeSinceEpoch") == 0) {
		response = GetTimeSinceEpoch();
	}
	else if (strcmp(request, "GetClientToServerDelayEstimation") == 0) {
		response = WhatIsTheTime();
	}
	else if (strcmp(request, "MeasureRTT") == 0) {
		response = WhatIsTheTime();
	}
	else if (strcmp(request, "GetTimeWithoutDateOrSeconds") == 0) {
		response = genericGetTime("%R");
	}
	else if (strcmp(request, "GetYear") == 0) {
		response = genericGetTime("%Y");
	}
	else if (strcmp(request, "GetMonthAndDay") == 0) {
		response = genericGetTime("%d/%m");
	}
	else if (strcmp(request, "GetSecondsSinceBeginingOfMonth") == 0) {
		response = GetSecondsSinceBeginingOfMonth();
	}
	else if (strcmp(request, "GetDayOfYear") == 0) {
		response = GetDayOfYear();
	}
	else if (strcmp(request, "GetDaylightSaving") == 0) {
		response = GetDaylightSaving();
	}
	return response;
}

// === NETWORK ===

void configNetwork(SOCKET & m_socket, sockaddr_in & serverService){

	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
	}

	// Server side:
	// Create and bind a socket to an internet address.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
												// IP Port. The htons (host to network - short) function converts an
												// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR *)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return;
	}
}

// === MAIN ===

void main()
{

	// Configure a UDP socket & Server
	SOCKET m_socket;
	sockaddr_in server;
	configNetwork(m_socket, server);

	// Waits for incoming requests from clients.

	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);

	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are 
	// returned by recvfrom. The fourth argument is an idicator 
	// specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	cout << "Time Server: Wait for clients' requests.\n";

	while (true)
	{
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

		// Answer client's request

		//TODO: wait for Hadar's response
		//auto FunctionMap = createMap();
		//strcpy(sendBuff, FunctionMap[recvBuff]());

		char * response = processRequest(recvBuff);
		strcpy(sendBuff, response);
		delete response;

		// Sends the answer to the client, using the client address gathered by recvfrom. 

		bytesSent = sendto(m_socket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr *)&client_addr, client_addr_len);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
}