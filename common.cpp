
#include <stdio.h>
#include <string.h>
#include <string>
#include "common.h"

#define CRC_POLYNOMIAL 0xEDB88320

char* createMESGMessage(const char* r_name, const char* message){
    std::string retval = "MESG|";
    std::string msg_blck = r_name;
    msg_blck += "|";
    msg_blck += message;
    retval += msg_blck;

    uint8_t parity = simple_parity_check(msg_blck.c_str());

    uint8_t crcVal = calculateCRC((const uint8_t*)msg_blck.c_str(), msg_blck.size());
    
    retval += parity;
    retval += crcVal;

    char* ret = (char*) malloc(strlen(retval.c_str()) +1);
    strcpy(ret, retval.c_str());
    return ret;
}

char* createCONNMessage(const char* name){

    std::string retval = "CONN|";
    retval += name;
    return (char*)retval.c_str();

}

char* createMERRMessage(){
    std::string retval = "";
    retval += "MERR";

    return (char*)retval.c_str();
}

char* createGONEMessage(){
    std::string retval = "";
    retval += "GONE";

    return (char*)retval.c_str();
}

std::string corruptMessage(std::string message) {
    srand(time(NULL));

    int randomIndex = rand() % message.size();

    message[randomIndex] = (message[randomIndex] == '0') ? '1' : '0';
    return message;
}


struct _data* parse_data(char* buffer){
    struct _data* retval = (struct _data*)malloc(sizeof(_data));
    char* token = strtok(buffer, "|");
    std::string command = token;
    if(command.compare("CONN") == 0){
        retval->type = CONN;
        token = strtok(NULL, "");
        
        strcpy(retval->data, token);
    }
    else if(command.compare("MESG") == 0){
        retval->type = MESG;
        token = strtok(NULL, "");
        
        strcpy(retval->data, token);

        retval->CRC = retval->data[strlen(retval->data) - 1];
        retval->data[strlen(retval->data) - 1] = '\0';

        retval->parity = retval->data[strlen(retval->data) - 1] ;
        retval->data[strlen(retval->data) - 1] = '\0';

    }
    else if(command.compare("MERR") == 0){
        retval->type = MERR;
    }
    else if(command.compare("GONE") == 0){
        retval->type = GONE;
    }
    
    if(retval->type == 1 || retval->type == 0){
        
    }
    
    return retval;
}

struct _message* parse_message(char* msg){
    struct _message* retval = (struct _message*) malloc(sizeof(struct _message));
    char* token = strtok(msg, "|");
    strcpy(retval->reciever_name, token);
    token = strtok(NULL, "|");
    strcpy(retval->message, token);

    return retval;
}



uint8_t simple_parity_check(const char* data){
    int parity = 0;

    // Iterate through the buffer
    for (int i = 0; i < strlen(data); i++) {
        // Count the number of set bits (1s)
        parity += (data[i] & 1);
    }

    // Parity is even if the count is even
    return (parity % 2 == 0);
}

uint8_t calculateCRC(const uint8_t *data, size_t size) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < size; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? CRC_POLYNOMIAL : 0);
        }
    }

    return crc ^ 0xFFFFFFFF;
}

