#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <atomic>

#include "../Socket/Socket.cpp"

using namespace std;

#define ERROR -1
#define LENGTH 4096

// Global variables
volatile atomic<int> flag(0);
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

// void str_trim_lf(char* arr, int length) {
//     int i;
//     for (i = 0; i < length; i++) {  // trim \n
//         if (arr[i] == '\n') {
//             arr[i] = '\0';
//             break;
//         }
//     }
// }

void recv_msg_handler() {
    char message[LENGTH] = {};
    while (1) {
        int receive = recv(sockfd, message, LENGTH, 0);
        if (receive > 0) {
            printf("%s", message);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
        memset(message, 0, sizeof(message));
    }
}

void send_msg_handler() {
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while (1) {
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        // str_trim_lf(message, LENGTH);

        if (strcmp(message, "exit") == 0) {
            break;
        } else {
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
    catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
    char message[LENGTH] = {};
    while (1) {
        int receive = recv(sockfd, message, LENGTH, 0);
        if (receive > 0) {
            printf("%s", message);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
        memset(message, 0, sizeof(message));
    }
}

char* askName() {
    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    // str_trim_lf(name, strlen(name));

    if (strlen(name) > 32 || strlen(name) < 2) {
        printf("Name must be less than 30 and more than 2 characters.\n");
        return "";
    }
}

void catch_ctrl_c_and_exit(int sig) { flag = 1; }

int main() {
    signal(SIGINT, catch_ctrl_c_and_exit);

	askName();
	
    Socket client(54400, "127.168.0.115");
    
    client.startClient();
}