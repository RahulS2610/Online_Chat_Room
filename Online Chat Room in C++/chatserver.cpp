/**
 * Author: Rahul Sunkara
 * GTID: 903401231
 * GT Email: rsunkara3@gatech.edu
 */

#include <cstdlib>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#include <udt.h>
#include <list>
#include <time.h> 

#define MSG_LEN 1024

using namespace std;

void acceptingClients(UDTSOCKET sockServer, char* passwrod, list<int> *incomingClients);
void hanndlingClient(UDTSOCKET sockClient, char* passwrod, list<int> *incomingClients);
void interpreter(char* message);
bool passwordCheckerANDgetUsername(UDTSOCKET sockClient, char *password, list<int> *incomingClients);


struct clients{
  UDTSOCKET clientSock;
  char *password;
};

int main(int argc, char *argv[]){
  string servie("");;
  char *password;
  if (argc == 5){
    servie = argv[2];
    password = argv[4];
  } else {
    cout << "Enter the correct number of inputs";
  }

  // setting up the server socket based on the provided ports
  addrinfo hints;
  addrinfo *res;

  // set socket parameters, sock stream for stream like behavior
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // check if the port is busy
  if (0 != getaddrinfo(NULL, servie.c_str(), &hints, &res)) {
    cout << "port is busy \n" << endl;
    return 0;
  }

  cout << "Server started on port " << argv[2] << ". Acceping Connection\n" << endl;

  // creating the server socket
  UDTSOCKET serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);


  if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen))
  {
    std::cout << "bind error: " << UDT::getlasterror().getErrorMessage() << endl;
    return 0;
  }

  UDT::listen(serv, 10);
  std::list<int> incomingClients = {};
  
  acceptingClients(serv, password, &incomingClients);

  // clean up the server socket
  UDT::close(serv);
  UDT::cleanup();
  return 0;
}

void acceptingClients(UDTSOCKET sockServer, char* passwrod, list<int> *incomingClients) {
  sockaddr_storage clientADD;
  int addLEN = sizeof(clientADD);
  while (true) {
    UDTSOCKET sockClient;
    if (UDT::ERROR == (sockClient = UDT::accept(sockServer, (sockaddr *)&clientADD, &addLEN)))
    {
      cout << "accept error: " << UDT::getlasterror().getErrorMessage() << endl;
    }
    bool pswrd = passwordCheckerANDgetUsername(sockClient, passwrod, incomingClients);
    if (pswrd) {
      incomingClients->push_back(sockClient);
      thread threadHandling (hanndlingClient, sockClient, passwrod, incomingClients);
      threadHandling.detach();

    } else {
      UDT::close(sockClient);
    }
  }
}

void hanndlingClient(UDTSOCKET sockClient, char* password, list<int> *incomingClients){
    while (true){
      char msg[MSG_LEN] = {};
      memset(msg, 0, MSG_LEN);

      int content = UDT::recv(sockClient, msg, MSG_LEN, 0);
      if (content <= 0){
        continue;
      }

      string leave = ":Exit";
      if (strcmp(msg, leave.c_str()) == 0) {
        //cout << "Left the chatroom" << endl;
        UDT::close(sockClient);
        incomingClients->remove(sockClient);
      } else {
        interpreter(msg);
        cout << string(msg) << endl;

        for (auto& newClient: *incomingClients) {
          if (newClient != sockClient) {
            UDT::send(newClient, string(msg).c_str(), string(msg).length(), 0);
          }
        }
      }
    }
}

bool passwordCheckerANDgetUsername(UDTSOCKET sockClient, char *password, list<int> *incomingClients){
  char msg[MSG_LEN] = {};
  memset(msg, 0, MSG_LEN);
  if (UDT::ERROR == UDT::recv(sockClient, msg, MSG_LEN, 0)){
    cout << "receive error: " << UDT::getlasterror().getErrorMessage() << endl;
  }
  if (strcmp(msg, password) != 0){
    string error = "Incorrect passcode";
    UDT::send(sockClient, error.c_str(), error.length(), 0);
    UDT::close(sockClient);
    return false;
  }
  
  else{
    string good = "Good passcode";
    UDT::send(sockClient, good.c_str(), good.length(), 0);
   
    if (UDT::ERROR == UDT::recv(sockClient, msg, MSG_LEN, 0)){
      cout << "receive error: " << UDT::getlasterror().getErrorMessage() << endl;
    }

    cout << string(msg) << " joined the chatroom" << endl;
    for (auto& newClient: *incomingClients) {
        if (newClient != sockClient) {
          UDT::send(newClient, string(msg).c_str(), string(msg).length(), 0);
        }
      }

    return true;
  }
}

void interpreter(char* messages) {
  while (*messages != ':') {
    messages += 1;
  }
  messages += 2;
  if (strncmp(messages, ":)", 2) == 0) {
    string happy = "[feeling happy]";
    strcpy(messages, happy.c_str());
  } else if (strncmp(messages, ":(", 2) == 0) {
    string sad = "[feeling sad]";
    strcpy(messages, sad.c_str());
  } else if (strncmp(messages, ":mytime", 7) == 0) {
    time_t timenow;
    struct tm * timeinfo;
    time (&timenow);
    timeinfo = localtime (&timenow);
    string timeNOW = asctime(timeinfo);
    strcpy(messages, timeNOW.c_str());
  } else if (strncmp(messages, ":+1hr", 5) == 0) {
    time_t timeplus;
    struct tm * timeinfo;
    time (&timeplus);
    timeplus = timeplus + 3600;
    timeinfo = localtime (&timeplus);
    string timePLUSONE = asctime(timeinfo);
    strcpy(messages, timePLUSONE.c_str());
  }
}