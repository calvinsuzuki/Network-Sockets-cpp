#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>

#include "../Socket/Client.cpp"

using namespace std;

#define ERROR -1
#define LENGTH 4096

char* askName() {
    char name[32];

    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    // str_trim_lf(name, strlen(name));

    if (strlen(name) > 32 || strlen(name) < 2) {
        printf("Name must be less than 30 and more than 2 characters.\n");
        return "";
    }
    return name;
}

int main() {
    int sockfd = 0;
    char name[32];  

    // Handle Cntrl^C
    signal(SIGINT, catch_ctrl_c_and_exit);

	askName();
	
    Client client(name, 54400, "127.168.0.115");
    
    client.startClient();
}