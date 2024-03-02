#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <iomanip>
#include <time.h>
#include <fstream>
#include "common.h"


#define PORT 8080


using namespace std;

pthread_mutex_t mutex;

std::ofstream logfile;

void* ReceiveChat(void* val){

    int sock = *((int*) val);

    char buffer[1024] = { 0 };
    int valread;
    while(valread = read(sock, buffer, 1024) > 0){
        pthread_mutex_lock(&mutex);

        _data* currdata = parse_data(buffer);

        uint8_t parity = simple_parity_check(currdata->data); 
        uint8_t crcVal = calculateCRC((const uint8_t*)currdata->data, strlen(currdata->data));

        if(currdata->type == MESG && (parity != currdata->parity || crcVal != currdata->CRC)){
            cout << "Hata Bozuk Mesaj Alındı" << endl;
            logfile << "Hata Bozuk Mesaj Alındı" << endl;

            char* merr_msg = createMERRMessage();
            send(sock, merr_msg, strlen(merr_msg), 0);
            memset(buffer, 0, 1024);

        }
        else{
            _message* msg_data = parse_message(currdata->data);

            cout << msg_data->reciever_name << "->" << msg_data->message << endl;
            logfile << msg_data->reciever_name << "->" << msg_data->message << endl;

            memset(buffer, 0, 1024);
        }

        
        pthread_mutex_unlock(&mutex);
        
    }

    return 0;
}


int main(int argc, char const* argv[])
{
    string name;
    cout << "Kullanıcı Adı: ";
    cin >> name;
    cout << endl;

    char date[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(date, sizeof(date)-1, "%d %m %Y %H:%M", t);
    
    string filename = "logs/" + string(date) + " " + name + ".txt";


    logfile.open(filename);

    if (!logfile.is_open()) {
        std::cerr << "Dosya açılamadı" << std::endl;
        return 1;
    }

    pthread_t thread;
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    char command[1024];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Soket oluşturulamadı" << endl;
        logfile << "Soket oluşturulamadı" << endl;
        return -1;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
   
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cout << "Yanlış adres" << endl;
        logfile << "Yanlış adres" << endl;
        return -1;
    }
  
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        cout << "Bağlantı Hatası" << endl;
        logfile << "Bağlantı Hatası" << endl;
        return -1;
    }
    char* comm_str = createCONNMessage(name.c_str());
    send(sock, comm_str, strlen(comm_str), 0);

    pthread_create(&thread, NULL, ReceiveChat, &sock);
    

    while(1){
        
        cin >> command;
        pthread_mutex_lock(&mutex);
        logfile << command << endl;
        if(strcmp(command, "exit") == 0){
            comm_str = createGONEMessage();
            send(sock, comm_str, strlen(comm_str), 0);
            break;
        }
        else if (strstr(command, ":") != NULL) {
            char* token = strtok(command, ":");
            char* rname = (char*)malloc(sizeof(token)+1);
            strcpy(rname, token);
            token = strtok(NULL, ":");
            char* message = (char*)malloc(sizeof(token)+1);
            strcpy(message, token);

            const char* com_str = createMESGMessage(rname, message);
            send(sock, com_str, strlen(com_str), 0);

        }
        else{
            cout << "Yanlış Komut Girildi" << endl;
            logfile << "Yanlış Komut Girildi"<< endl;
        }
        pthread_mutex_unlock(&mutex);
    }

    close(client_fd);
    logfile.close();
    return 0;
}