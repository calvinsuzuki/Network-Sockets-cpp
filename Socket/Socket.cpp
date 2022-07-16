#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

#define ERROR -1

class Socket {
    public:
        unsigned int port;
        const char * ipAddrs;
        Socket(unsigned int _port, const char * _ipAddrs) {
            port = _port;
            ipAddrs = _ipAddrs;
        }
        int startClient() {
            //	Create a socket
            int client = socket(AF_INET, SOCK_STREAM, 0);
            if (client == -1) {
                cout << "SOCKET EXCEPTION: Could not create!";
                return ERROR;  
            }  

            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            inet_pton(AF_INET, ipAddrs, &hint.sin_addr);

            //	Connect to the server on the socket
            int connectRes = connect(client, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!";
                return ERROR;
            }
            
            // Control variables
            int bufsize = 4096;
            char buf[4096];
            string str_buf;
            string userInput;
            
            cout << endl;
            cout << "------WELCOME TO SERVERLESS, YOUNG CLIENT--------" << endl;
            cout << "-------------------------------------------------" << endl;
            cout << "---Use the character '$' to finish the message---" << endl;
            cout << endl;

            //	While loop:
            while ( true ) {
                cout << "\nClient: ";
                sendMessage(client, 4096);

                cout << "Waiting server message...";

                cout << "\nServer: ";
                do {
                    recv(client, buf, bufsize, 0);
                    if (*buf != '$') cout << buf;
                } while( *buf != '$');
            }

            close(client);
            return 0;
        }

        int startServer() {
            int opt = true;
            int listening = socket(AF_INET, SOCK_STREAM, 0);
            if (listening == -1) {
                cout << "SOCKET EXCEPTION: Could not create!";
                return ERROR;
            }

            //set master socket to allow multiple connections , 
            //this is just a good habit, it will work without this 
            if( setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
            {  
                perror("setsockopt");  
                return ERROR;
            } 
            
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            inet_pton(AF_INET, ipAddrs, &hint.sin_addr);

            if( bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
                cout << "BINDING EXCEPTION: Can't bind to IP/Port";
                return ERROR;
            }

            if( listen(listening, SOMAXCONN) == -1 ) {
                cout << "LISTENING EXCEPTION: Can't listen";
                return ERROR;
            }

            sockaddr_in client;
            socklen_t clientSize;
            char host[NI_MAXHOST];
            char svc[NI_MAXSERV];

            int server = accept(listening, (sockaddr*)&client, &clientSize);

            if( server == -1) {
                cout << "CONNECTION EXCEPTION: Client could not connect!";
                return ERROR;
            }

            close(listening);

            memset(host, 0, NI_MAXHOST);
            memset(svc, 0, NI_MAXSERV);

            int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

            if( !result ) {
                inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            }

            char buf[4096];
            int bufsize = 4096;
            
            cout << endl;
            cout << "--------WELCOME TO THE SERVERLESS SERVER---------" << endl;
            cout << "-------------------------------------------------" << endl;
            cout << "---Use the character '$' to finish the message---" << endl;
            cout << endl;

            while( true ) {
                memset( buf, 0, 4096 );
                send(server, buf, bufsize, 0);
                cout << "=> Enter '$' at the end of the message" << endl;

                while ( true ) {
                    cout << "Waiting client message...";
                    cout << "\nClient: ";
                    do {
                        recv(server, buf, bufsize, 0);
                        if (*buf != '$') cout << buf;
                    } while( *buf != '$');

                    cout << "\nServer: ";
                    sendMessage(server, 4096);
                }
            }

            close(server);
            return 0;
        }

    protected:
    private:
    int sendMessage(int client, int max_message_size) {
        //max_message_size /= 2;
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
            //cout << message;
            send(client, message, max_message_size, 0);

            //Subtracts the toSend variable
            toSend -= frag_text.length();   
        }

        send(client, "$", max_message_size, 0);
        return 0;
    }
};