//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>

#include <netinet/in.h>
#include <signal.h>
#include <atomic>

#include <thread>
#include <mutex>

using namespace std;

#define ERROR -1
#define BUFFER_SZ 4096
#define MAX_CLIENTS 3

// Global variables
atomic<unsigned int> clientCount{0};
mutex m;
int uid = 0; // Starting USER ID 

/* Server variables */
typedef struct{ // client struct
    sockaddr_in address;
    int sockfd;
    int uid;
    char nick[50];
    bool leave_flag;
} client_t;

client_t *clients[MAX_CLIENTS];

void disconnectClient(client_t*);

void sendAndReadACK(const char *s, client_t *client) {
    char ACK[3];

    for( int j = 0; j < 6; ++j ) {
        if( write(client->sockfd, s, strlen(s)) < 0 ) {
            cout << "ERROR: write to descriptor failed" << endl;
            break;
        }
        recv( client->sockfd, ACK, 3, 0 );
        if (strcmp( ACK, "ACK") != 0) {
            if ( j < 5) {
                cout << " > Server: Not received ACK from " << client->nick;
                cout << " (attempt " << (j+1) << ")" << endl;
                memset( ACK, 0, 3 );    
                sleep(1);
            } else {
                cout << " > Server: " << client->nick << 
                    " is not responding. Disconnecting..." << endl;
                // Disconects this client
                client->leave_flag = true;
            }
        }
        else break;
    }
    return;
}

/* Send message to all clients */
void sendToAll(const char *s) {
    m.lock();
    // ***LOCKED ACTION***
    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]) {
            sendAndReadACK( s, clients[i] );            
        }
    }
    m.unlock();
}

/* Send message to all clients */
void sendToOne(const char *s, int uid) {
    m.lock();
    // ***LOCKED ACTION***
    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]) {
            if( clients[i]->uid == uid ) {
                sendAndReadACK( s, clients[i] );            
                cout << " >Server: " << s <<" -> " << clients[i]->nick << endl;
            }
        }
    }
    m.unlock();
}


/* Handle all communication with the client */
void handle_client(client_t *currentClient) {
    char sendBuff[BUFFER_SZ];
    string aux = "";
    char nickBuff[32];

    clientCount.fetch_add(1); // Increments the atomic counter
    cout << " > Server: Clients Online: " << clientCount.load() << " -> ";
    
    // Receive the nickname
    recv(currentClient->sockfd, nickBuff, 50, 0);

    // Join anouncement
    if( strlen(nickBuff) < 2 || strlen(nickBuff)-1 > 32 ) {
        cout << "Didn't enter the name.\n";
    }
    else {
        strcpy( currentClient->nick, nickBuff );

        sprintf( sendBuff, "** %s has joined **", nickBuff); // sendBuff = {NICK} has joined!
        cout << nickBuff << " joined ( uid = " << currentClient->uid << " )" << endl;        
        sendToAll( sendBuff );
    }

    memset(sendBuff, 0, BUFFER_SZ); // Clears buffer
    while( !(currentClient->leave_flag) ) {        
        int receive = recv( currentClient->sockfd, sendBuff, BUFFER_SZ, 0);
        if ( receive > 0 ) {
            if ( strlen(sendBuff) > 0 && strcmp(sendBuff, "/quit") != 0 ) {

                if ( strcmp( sendBuff, "/ping") == 0 ) {
                    sendToOne( "pong!", currentClient->uid );
                    continue;
                }
                // Send message to all
                aux = currentClient->nick;
                aux.append( ": " );
                aux.append( sendBuff );
                sendToAll( aux.c_str() );
                cout << aux << endl;
            }
        } 
        /* Leave anouncement */
        else if( receive == 0 || strcmp(sendBuff, "/quit") == 0 ) {
            sprintf( sendBuff, "** %s has left! **", currentClient->nick);
            sendToAll( sendBuff );
            cout << sendBuff << endl;
            currentClient->leave_flag = true;
        } else {
            cout << "ERROR IN RECEIVE MESSAGE" << endl;
            currentClient->leave_flag = true;
        }

        memset(sendBuff, 0, BUFFER_SZ); // Clears buffer
    }

    disconnectClient( currentClient );
    return;
}

void disconnectClient(client_t *currentClient) {
    close( currentClient->sockfd );
    
    m.lock();   // ***LOCKED***
    for(int i=0; i < MAX_CLIENTS; ++i) {
        if(clients[i]) {
            if(clients[i]->uid == currentClient->uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    m.unlock(); // ***UNLOCKED***

    free( currentClient );
    currentClient = NULL;
    clientCount.fetch_sub(1);// Decrements the atomic counter
    cout << " > Server: Clients Online: " << clientCount.load() << endl;
}

class Server {
    public:
        Server(unsigned int _port, const char * _ipAddrs) {
            port = _port;
            ipAddrs = _ipAddrs;
            hint = socketHandler();
        }

        int startServer() {            
            // Create master socket
            _socket = createSocket(AF_INET, SOCK_STREAM, 0);

            signal( SIGPIPE, SIG_IGN ); // Ignore pipe signals ????

            multipleConnections(); // with same addrs AND port
            socketBind();
            socketListen();

            cout << endl << "--------------WELLCOME TO CHATROOM---------------\n" << endl;  

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
                    cout << "Max clients reached. Rejected: " << endl;
                    // Print client addrs and port?
                    close( listenFD );
                    continue;
                }

                // Client set
                client_t *client = (client_t *) malloc( sizeof(client_t) );
                client->address = client_addrs;
                client->sockfd = listenFD;
                client->uid = uid++;
                client->leave_flag = false;

                // Add client to the queue
                m.lock();   // ***LOCK***                
                for(int i=0; i < MAX_CLIENTS; ++i){
                    if(!clients[i]){
                        clients[i] = client;
                        break;
                    }
                }  
                m.unlock(); // ***UNLOCK***

                thread tr( handle_client, client );
                tr.detach();                

                sleep(1); // Reduce CPU usage ???
            }
            return 0;
        }

    protected:
    private:
        const char *ipAddrs;
        unsigned int port;
        sockaddr_in hint;
        int _socket;        
    
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
            hint.sin_addr.s_addr = inet_addr(ipAddrs); // This accepts IPv6
            return hint;
        }

        int multipleConnections() {
            int opt = 1;
            if( setsockopt(_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&opt, sizeof(opt)) < 0 ) 
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

};
