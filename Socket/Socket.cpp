#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#include <netinet/in.h>
#include <errno.h>
#include <thread>
#include <pthread.h>
#include <signal.h>
#include <atomic>

using namespace std;

#define ERROR -1
#define BUFFER_SZ 4096
#define MAX_CLIENTS 10

class Socket {
    public:
        Socket(unsigned int _port, const char * _ipAddrs) {
            port = _port;
            ipAddrs = _ipAddrs;
            hint = socketHandler();
        }
        
        int startClient() {
            //	Create client socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            //	Connect to the server on the socket
            socketConnect();
            
            // Control variables
            int bufsize = BUFFER_SZ;
            char buf[BUFFER_SZ];
            string str_buf;
            string userInput;
            
            cout << "----------------SERVER CONNECTED-----------------" << endl;


            close(_socket);
            return 0;
        }

        int startServer() {

            

            // Create master socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            signal( SIGPIPE, SIG_IGN ); // Ignore pipe signals ????
            
            multipleConnections(); // with same addrs AND port
            socketBind();
            socketListen();

            cout << endl << "--------------WELLCOME TO CHATROOM---------------\n" << endl;  

            // int listenFileDescriptor = socketAccept(); // Chat p2p
            int listenFD;
            sockaddr_in client_addrs;
            while( true ) {
                socklen_t clientSize = sizeof(sockaddr_in); // ??
            
                listenFD = accept(_socket, (sockaddr*)&client_addrs, &clientSize);

                if( listenFD == ERROR ) {
                    cout << "CONNECTION EXCEPTION: Client could not connect!";
                    return ERROR;
                }

                if( (clientCount+1) == MAX_CLIENTS ) {
                    cout << "Max clients reached. Rejected: ";
                    // Print client addrs and port?
                    close( listenFD );
                    continue;
                }

                // Client set
                client_t *client = (client_t *) malloc( sizeof(client_t) );
                client->address = client_addrs;
                client->sockfd = listenFD;
                client->uid = uid++;

                // Add client to the queue and fork thread
                // queue_add( client );
                pthread_mutex_lock(&clients_mutex);
                
                // ***LOCKED ACTION***
                for(int i=0; i < MAX_CLIENTS; ++i){
                    if(clients[i]){
                        if(clients[i]->uid == uid){
                            clients[i] = NULL;
                            break;
                        }
                    }
                }
    
                pthread_mutex_unlock(&clients_mutex);
                // 
                // pthread_create( &tid, NULL, &handle_client, (void*)client); ERROR

                sleep(1); // Reduce CPU usage ???
            }

            // close(_socket);            

            // char buf[BUFFER_SZ];
            // int bufsize = BUFFER_SZ;
            
            // cout << "----------------CLIENT CONNECTED-----------------\n" << endl;

            // while( true ) {
            //     memset( buf, 0, BUFFER_SZ );
            //     send(listenFD, buf, bufsize, 0);

            //     while ( true ) {
            //         //cout << "Waiting client message...";
            //         cout << "Client: ";
            //         do {
            //             recv(listenFD, buf, bufsize, 0);
            //             if (*buf != '$') cout << buf;
            //         } while( *buf != '$');

            //         cout << "\nServer: ";
            //         sendMessage(listenFD, BUFFER_SZ);
            //     }
            // }

            // close(listenFD);
            return 0;
        }

    protected:
    private:
        const char *ipAddrs;
        unsigned int port;
        sockaddr_in hint;
        int _socket;
        int uid = 10;

        /* Server variables */
        typedef struct{ // client struct
            sockaddr_in address;
            int sockfd;
            int uid;
            char name[32];
        } client_t;

        pthread_mutex_t clients_mutex; // server mutex
        pthread_t tid;

        // Start clients list
        client_t *clients[MAX_CLIENTS];

        atomic<unsigned int> clientCount{0}; // static _Atomic unsigned int cli_count = 0;

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
        
        int createSocket(int __domain, int __type, int __protocol) {
            int sock = socket(__domain, __type, __protocol);
            if (sock == -1) {
                cout << "SOCKET EXCEPTION: Could not create!";
                return ERROR;
            }
            return sock;
        }

        sockaddr_in socketHandler() {
            sockaddr_in hint;
            hint.sin_family = AF_INET;
            hint.sin_port = htons(port);
            /* inet_pton(AF_INET, ipAddrs, &hint.sin_addr); DEPRECATED */
            hint.sin_addr.s_addr = inet_addr(ipAddrs); // This accepts IPv6
            return hint;
        }

        int multipleConnections() {
            int opt = 1;
            // ERROR: CAN'T BIND TO PORT
            // if( setsockopt(_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&opt, sizeof(opt)) < 0 ) 
            if( setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
            {  
                perror("setsockopt");  
                return ERROR;
            }
            return 0;
        }

        int socketConnect() {
            int connectRes = connect(_socket, (sockaddr*)&hint, sizeof(hint));
            if (connectRes == -1) {
                cout << "CONNECT EXCEPTION: Server not reachable!";
                return ERROR;
            }
            else return 0;
        }

        int socketBind() {
            if( bind(_socket, (sockaddr*)&hint, sizeof(hint)) == -1) {
                cout << "BINDING EXCEPTION: Can't bind to IP/Port";
                return ERROR;
            }
            else return 0;
        }

        int socketListen() {
            if( listen(_socket, SOMAXCONN) == -1 ) {
                    cout << "MASTER EXCEPTION: Can't listen";
                    return ERROR;
            }
            else return 0;
        }

        /* Send message to all clients except sender */
        void send_messages(char *s, int uid){
            pthread_mutex_lock(&clients_mutex);

            for(int i=0; i<MAX_CLIENTS; ++i){
                if(clients[i]){
                    if(clients[i]->uid != uid){
                        if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                            perror("ERROR: write to descriptor failed");
                            break;
                        }
                    }
                }
            }

            pthread_mutex_unlock(&clients_mutex);
        }

        /* Remove clients to queue */
        void queue_remove(int uid){
            pthread_mutex_lock(&clients_mutex);

            for(int i=0; i < MAX_CLIENTS; ++i){
                if(clients[i]){
                    if(clients[i]->uid == uid){
                        clients[i] = NULL;
                        break;
                    }
                }
            }

            pthread_mutex_unlock(&clients_mutex);
        }

        /* Handle all communication with the client */
        void *handle_client(void *arg){
            char buff_out[BUFFER_SZ];
            char name[32];
            int leave_flag = 0;

            clientCount.fetch_add(1); //clientCount++;
            client_t *cli = (client_t *)arg;

            // Name
            if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
                printf("Didn't enter the name.\n");
                leave_flag = 1;
            } else{
                strcpy(cli->name, name);
                sprintf(buff_out, "%s has joined\n", cli->name);
                printf("%s", buff_out);
                send_messages(buff_out, cli->uid);
            }

            bzero(buff_out, BUFFER_SZ);

            while(1){
                if (leave_flag) {
                    break;
                }

                int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
                if (receive > 0){
                    if(strlen(buff_out) > 0){
                        send_messages(buff_out, cli->uid);

                        // str_trim_lf(buff_out, strlen(buff_out)); // may not help
                        printf("%s -> %s\n", buff_out, cli->name);
                    }
                } else if (receive == 0 || strcmp(buff_out, "exit") == 0){
                    sprintf(buff_out, "%s has left\n", cli->name);
                    printf("%s", buff_out);
                    send_messages(buff_out, cli->uid);
                    leave_flag = 1;
                } else {
                    printf("ERROR: -1\n");
                    leave_flag = 1;
                }

                bzero(buff_out, BUFFER_SZ);
            }

            /* Delete client from queue and yield thread */
            close(cli->sockfd);
            queue_remove(cli->uid);
            free(cli);
            clientCount.fetch_sub(1); // clientCount--;
            pthread_detach(pthread_self());

            return NULL;
        }

};
