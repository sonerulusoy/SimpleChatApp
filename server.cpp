#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <list>
#include <vector>
#include <ctime>
#include "common.h"

#define PORT 8080


using namespace std;

pthread_mutex_t mutex;

class client{
public:
    char name[256];
    char lastMessage[1024];
    bool isActive;
    int socket;
    pthread_t thread;
};


int clients_size;
vector<client> clients;

void removeClient(client* _client){
    _client->isActive = false;
    for(int i=0;i<clients.size();i++){
        if(&clients[i] == _client){
            clients.erase(clients.begin() + i);
            break;
        }
    }
    clients_size--;
}

string clientListString(){
    string retVal = "\nKullanıcı Listesi:\n";
    for(int i=0;i<clients.size();i++){
        if(clients[i].isActive){

            string name(clients[i].name);
            retVal += to_string(i) + "- " + name + "\n";
        }
    }
    return retVal;
}

client* getClient(char* name){
    for(int i=0;i<clients.size();i++){
        if(strcmp(clients[i].name, name) == 0 && clients[i].isActive){
            return &clients[i];
        }
    }
    return NULL;
}

bool getRandomBoolean() {
    std::srand(std::time(0));
    return rand() % 100 == 0;
}

void* ServerClient(void* threadVal){
    client* _client = (client*)threadVal;
    char buffer[1024] = { 0 };
    int valread;

    char* token;

    valread = read(_client->socket , buffer, 1024);
    
    struct _data* currdata = parse_data(buffer); 

    if(currdata->type == CONN && !_client->isActive){
        string client_list = clientListString();
        string msg = createMESGMessage("Server", client_list.c_str());
        _client->isActive = true;
        strcpy(_client->name, currdata->data);
        cout << _client->name << " servera bağlandı." << endl;
        send(_client->socket, msg.c_str(), strlen(msg.c_str()), 0);

        string notification = createMESGMessage("Server", ((string)_client->name + " gruba katıldı.").c_str());

        for(int i=0;i<clients_size;i++){
            if(clients[i].socket != _client->socket && clients[i].isActive){
                send(clients[i].socket, notification.c_str(), strlen(notification.c_str()), 0);
            }
        }
    }

    while((valread = read(_client->socket, buffer, 1024)) > 0){
        pthread_mutex_lock(&mutex);

        struct _data* currdata = parse_data(buffer); 

        if(currdata->type == MESG){

            struct _message* currmessage = parse_message(currdata->data);


            client* reciever = getClient(currmessage->reciever_name);
            if(reciever != NULL){

                string msg_to_send = createMESGMessage(_client->name, currmessage->message);
                strcpy(reciever->lastMessage, msg_to_send.c_str());

                if(getRandomBoolean()){
                    msg_to_send = corruptMessage(msg_to_send);
                }

                send(reciever->socket, msg_to_send.c_str(), strlen(msg_to_send.c_str()), 0);
                cout << _client->name << " mesaj yolladı:" << currmessage->reciever_name << "." << endl;
            }
            else{
                cout << currmessage->reciever_name << " aktif değil." << endl;
            }
        }
        else if(currdata->type == MERR){
            
            send(_client->socket, _client->lastMessage, strlen(_client->lastMessage), 0);
        }

        else if(currdata->type == GONE){

            cout << _client->name << " serverla bağlantısı kesildi." << endl;
            close(_client->socket);
            _client->isActive = false;
            removeClient(_client);

            string notification = (string)_client->name + " gruptan ayrıldı.";

            for(int i=0;i<clients_size;i++){
                if(clients[i].socket != _client->socket && clients[i].isActive){

                    send(clients[i].socket, notification.c_str(), strlen(notification.c_str()), 0);
                }
            }

            break;
        }

        memset(buffer,0,1024);
        pthread_mutex_unlock(&mutex);

    }
    return 0;
}

int main(int argc, char const* argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    clients_size = 1;




    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    cout << "Server başlatıldı" << endl;

    while(true){
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&mutex);

        client tmpClient = client(); 

        clients.push_back(tmpClient);
        
        clients.back().socket = new_socket;

        pthread_create(&clients.back().thread, NULL, ServerClient, &clients.back());
        pthread_mutex_unlock(&mutex);

    }

    return 0;
}


