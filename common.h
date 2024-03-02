

#define CONN 0
#define MESG 1 
#define MERR 2
#define GONE 3

#define DIVISOR 100


struct _data{
    short type;
    uint8_t parity;
    uint8_t CRC;
    char data[1024];
}typedef _data;

struct _message{
    char reciever_name[256];
    char message[1024];
}typedef _message;



struct _message* parse_message(char* msg);
struct _data* parse_data(char* buffer);

char* createMESGMessage(const char* r_name, const char* message);
char* createCONNMessage(const char* name);
char* createMERRMessage();
char* createGONEMessage();
std::string corruptMessage(std::string message);

uint8_t simple_parity_check(const char* message);
uint8_t calculateCRC(const uint8_t *data, size_t size);
