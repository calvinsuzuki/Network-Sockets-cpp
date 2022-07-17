#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

#define max_message_size 5

int main() {
    string plain_text, frag_text;
    // Read the string
    getline(cin, plain_text);

    int toSend = (int) plain_text.length();

    while( toSend > 0) {      
        // Fragments the message
        if (toSend >= max_message_size) {
            frag_text = plain_text.substr(0, max_message_size);
            plain_text = plain_text.substr(max_message_size, plain_text.length());
            // Send message
        } 
        // When the message is already small enought
        else {
            frag_text = plain_text;
        }

        // Transform to array of char
        char message[frag_text.length()];
        strcpy( message, frag_text.c_str() );

        //Send message
        cout << "(" << message << ")" ;

        //Subtracts the toSend variable
        toSend -= frag_text.length();
    }

    return 0;
}
