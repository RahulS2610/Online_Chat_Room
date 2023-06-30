/**
 * Author: Rahul Sunkara
 * GTID: 903401231
 * GT Email: rsunkara3@gatech.edu
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <udt.h>
#include <thread>

#define MSG_LEN 1024

using namespace std;
using namespace UDT;

void output(UDTSOCKET sockServer, char* clientName);
void input(UDTSOCKET sockServer);

int main(int argc, char *argv[]) {
    char *hostnumber;
    char *username;
    char *password;
    char *port;

    if (argc == 10)
    {
        hostnumber = argv[3];
        username = argv[5];
        password = argv[7];
        port = argv[9];
    } else {
        cout << "Enter the correct number of inputs";
    }

    struct addrinfo hints, *peer;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    UDTSOCKET serv = UDT::socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

    if (0 != getaddrinfo(hostnumber, port, &hints, &peer))
    {
        cout << "incorrect server/peer address. " << argv[1] << ":" << argv[2] << endl;
        return -1;
    }

    // connect to the server, implict bind
    if (UDT::ERROR == UDT::connect(serv, peer->ai_addr, peer->ai_addrlen))
    {
        cout << "connect error: " << UDT::getlasterror().getErrorMessage() << endl;
        return -1;
    }

    UDT::send(serv, password, strlen(password), 0);

    char mesg[MSG_LEN] = {};
    UDT::recv(serv, mesg, MSG_LEN, 0);

    string errorpsd = "Incorrect passcode";
    if (strcmp(mesg, errorpsd.c_str()) == 0)
    {
        cout << errorpsd << endl;
        exit(0);
    }

   
    cout << "Connected to " << string(hostnumber) << " on port " << string(port) << endl;

   
    UDT::send(serv, username, strlen(username), 0);

   
    thread sendingMSG (output, serv, username);
    sendingMSG.detach();

    thread recevingMSG (input, serv);
    recevingMSG.detach();

    while (true) {}

}

void output(UDTSOCKET sockServer, char* clientName) {
    char sendMSG[MSG_LEN] = {};
    while (true)
    {
        memset(sendMSG, 0, MSG_LEN);
        fgets(sendMSG, MSG_LEN, stdin);
        string exitStr = ":Exit";
        string msg = string(clientName) + ": " + string(sendMSG);
        if (UDT::send(sockServer, msg.c_str(), strlen(msg.c_str()), 0) == UDT::ERROR)
        {
            cout << "send error: " << UDT::getlasterror().getErrorMessage() << endl;
        }
        if (strncmp(sendMSG, exitStr.c_str(), 5) == 0) {
            exit(0);
        }
    }
}

void input(UDTSOCKET sockServer) {
    char getMSG[MSG_LEN] = {};
    while (true)
    {
        memset(getMSG, 0, MSG_LEN);
        UDT::recv(sockServer, getMSG, MSG_LEN, 0);
        cout << string(getMSG) << endl;
    }
}