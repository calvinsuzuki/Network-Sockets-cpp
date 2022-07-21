#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

#define ERROR -1
#define BUFFER_SZ 4096
#define MAX_CLIENTS 10

// Global variables
atomic<unsigned int> clientCount{0};
mutex m;
int uid = 0;  // Starting USER ID

/* Server variables */
typedef struct {  // client struct
    sockaddr_in address;
    int sockfd;
    int uid;
    char nick[50];
    bool leave_flag;
    char channel[200];
    bool mute;
    bool admin;
} client_t;

client_t *clients[MAX_CLIENTS];

void disconnectClient(client_t *);

// TODO: FIXME
void sendAndReadACK(const char *s, client_t *client) {
    char ACK[3];
    bool readACK = true;
    int j = 0;
    chrono::_V2::steady_clock::time_point start, end;

    while (true) {
        if (readACK) {
            if (write(client->sockfd, s, strlen(s)) < 0) {
                cout << "ERROR: write to descriptor failed" << endl;
                break;
            }
        }

        if (readACK) {
            recv(client->sockfd, ACK, 3, 0);

            // Set read flag and UNLOCK mutex
            readACK = false;
            m.unlock();

            start = chrono::steady_clock::now();

            // Analyse ACK
            if (strcmp(ACK, "ACK") != 0) {
                if (j < 5) {
                    cout << " > Server: Not received ACK from " << client->nick;
                    cout << " (attempt " << (j + 1) << ")" << endl;
                    memset(ACK, 0, 3);
                    j++;
                    // sleep(1);
                } else {
                    cout << "> Server: " << client->nick << " is not responding. Disconnecting..." << endl;
                    // Disconects this client
                    client->leave_flag = true;
                    break;
                }
            } else
                break;
        }

        end = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(end - start).count() > 1) {
            readACK = true;
            m.lock();
        };
    }
    return;
}

void sendWithoutACK(const char *s, client_t *client) {
    char message[strlen(s)+1];
    strcpy( message, s);
    strcat( message, "\n");
    if (write(client->sockfd, message, strlen(message)) < 0) {
        cout << "ERROR: write to descriptor failed" << endl;
    }
    return;
}

/* Send message to all clients */
void sendToAll(const char *s) {
    m.lock();
    // ***LOCKED ACTION***
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (!(clients[i]->leave_flag)) {
                // sendAndReadACK( s, clients[i] );
                sendWithoutACK(s, clients[i]);
            }
        }
    }
    m.unlock();
    // Server log
    cout << "GLOBAL >> " << s << endl; 
}

/* Send message to all clients of a certain channel*/
void sendToAll(const char *s, const char * channel) {
    m.lock();
    // ***LOCKED ACTION***
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if ( !(clients[i]->leave_flag) ) {
                if ( strcmp(clients[i]->channel, channel ) == 0 ) { 
                    sendWithoutACK(s, clients[i]);
                    // sendAndReadACK( s, clients[i] );
                }
            }
        }
    }
    m.unlock();
    // Server log
    cout << channel << " >> " << s << endl;
}

/* Disconnects a given client and announces it  */
void announceDisconnect(client_t *client) {
    char sendBuff[BUFFER_SZ];
    sprintf(sendBuff, "** %s left the server! **", client->nick);
    client->leave_flag = true;
    sendToAll(sendBuff); // sendToAll(sendBuff, client->channel);
    return;
}

/* Send message to all clients */
void sendToOne(const char *s, int uid) {
    m.lock();
    // ***LOCKED ACTION***
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                // sendAndReadACK( s, clients[i] );
                sendWithoutACK(s, clients[i]);
                // Server log
                cout << "> Server: " << s << " -> " << clients[i]->nick << endl;
            }
        }
    }
    m.unlock();
}

void joinChannel(string channel, client_t *client) {
    // Check client
    if (!client) {
        return;
    }

    // Analyse all channels and see if its the first one
    bool first = true;
    char message[BUFFER_SZ];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (strcmp(clients[i]->channel, channel.c_str()) == 0) {
                first = false;
                break;
            }
        }
    }

    // Set channel on the client struct, announces its transition
    sprintf(message, "** %s left the channel %s **", client->nick, client->channel);
    sendToAll( message, client->channel );
    memset( message, 0, BUFFER_SZ ); // Clears buffer
    
    strcpy( client->channel, channel.c_str() );

    sprintf(message, "** %s joined the channel %s **", client->nick, client->channel);
    sendToAll( message, client->channel );
    memset( message, 0, BUFFER_SZ ); // Clears buffer

    // Make the client admin if it's the first one
    client->admin = first;

    // Advertising
    if (first) {
        // Log to the server
        cout << "> Server: " << client->nick << " created the channel " << channel << " and is the admin." << endl;

        // Send a message to the client advertising the new nickname
        sprintf(message, "** You are the channel admin. **");
        sendToOne(message, client->uid);
    }
    memset( message, 0, BUFFER_SZ ); // Clears buffer

};

void changeNickname(string new_nickname, client_t *client) {
    // check new_nickname len

    // Save old nickname
    char old_nick[50];
    strcpy(old_nick, client->nick);

    // change nickname
    strcpy(client->nick, new_nickname.c_str());

    // Send a message to the client advertising the new nickname
    char message[150];
    sprintf(message, "** Your nickname changed from %s to %s **", old_nick, new_nickname.c_str());
    sendToOne(message, client->uid);
    memset( message, 0, BUFFER_SZ ); // Clears buffer
};

void kickUser(string nickname, client_t *admin){
    char message[BUFFER_SZ];

    if (!admin->admin){
        // Log to the server
        cout << "> Server: " << admin->nick << " tried to kick " << nickname << " but it is not an admin." << endl;

        // Send a message to the admin 
        sendToOne("** You are not an admin **", admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer

        return;
    }

    // See if the admin is trying to kick itself
    if(strcmp(admin->nick, nickname.c_str()) == 0) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tryied to kick himself/herself." << endl;

        // Send a message to the admin
        sendToOne("** You cannot kick yourself **", admin->uid);

        return;
    }

    // find the nickname and kick it, otherwise send a message to admin
    bool nick_exists = false;
    for (int i = 0; i <  MAX_CLIENTS; i++) {
        if (clients[i]){
            if ((strcmp(clients[i]->nick, nickname.c_str()) == 0) && (strcmp(admin->channel, clients[i]->channel) == 0)){
                
                // Log to the server
                cout << "> Server: " << admin->nick << " kicked " << nickname << "." << endl;
                
                // Tell the client that he/she was kicked
                sprintf(message, "** You were kicked by %s **", admin->nick);
                sendToOne(message, clients[i]->uid);
                memset( message, 0, BUFFER_SZ ); // Clears buffer

                // Tell the admin that he/she was kicked the client
                sprintf(message, "** You kicked %s **", nickname.c_str());
                sendToOne(message, admin->uid);
                memset( message, 0, BUFFER_SZ ); // Clears buffer

                announceDisconnect(clients[i]);
                nick_exists = true;
                break;
            }
        }
    }

    if (!nick_exists) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tried to kick " << nickname << " but " << nickname << " doesn't exist" << endl;

        sprintf(message, "** User %s doesn't exist in %s channel. /kick failed **", nickname.c_str(), admin->channel);
        sendToOne(message, admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer
    }

    return;
}

/* Restrains the channel name to RFC-1459 norm */
bool RFC_channelName(string channel_name) {
    if ( channel_name.length() > 200 || channel_name.length() <= 0 ) {
        cout << "Bad size!" << endl;
        return false;
    }
    if( channel_name[0] != '#' && channel_name[0] != '&' ) {
        cout << "Bad format!" << endl;
        return false;
    }
    if ( channel_name.find(' ') != string::npos ) {
        cout << "Must not contain ' '" << endl;
        return false;
    }
    if ( channel_name.find(',') != string::npos ) {
        cout << "Must not contain ','" << endl;
        return false;
    }
    if ( channel_name.find('^') != string::npos ) {
        cout << "Must not contain '^'" << endl;
        return false;
    }
    return true;
}
void whoIs(string nickname, client_t *admin) {
    char message[BUFFER_SZ];

    if (!admin->admin) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tried /whois " << nickname << " but it is not an admin." << endl;

        // Send a message to the admin 
        sendToOne("** You are not an admin **", admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer

        return;
    }

    // find the nickname and kick it, otherwise send a message to admin
    bool nick_exists = false;
    for (int i = 0; i <  MAX_CLIENTS; i++) {
        if (clients[i]){
            if ((strcmp(clients[i]->nick, nickname.c_str()) == 0) && (strcmp(admin->channel, clients[i]->channel) == 0)){
                // Send to the admin the client IP address
                char ip_adrr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(clients[i]->address.sin_addr), ip_adrr, INET_ADDRSTRLEN);

                cout << "> Server: " << admin->nick << " requested " << nickname << " IP address." << endl; 
                sprintf(message, "** %s IP address is %s **", nickname.c_str(), ip_adrr);
                sendToOne(message, admin->uid);
                memset( message, 0, BUFFER_SZ ); // Clears buffer

                nick_exists = true; 
                break;
            }
        }
    }
    
    if (!nick_exists) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tried /whois " << nickname << " but " << nickname << " doesn't exist." << endl;

        sprintf(message, "** User %s doesn't exist in %s channel. /whois failed **", nickname.c_str(), admin->channel);
        sendToOne(message, admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer
    }

    return;
}

void muteUser(bool mute, string nickname, client_t *admin){
    char message[BUFFER_SZ];

    if (!admin->admin) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tried to mute/unmute " << nickname << " but it is not an admin." << endl;

        // Send a message to the admin 
        sendToOne("** You are not an admin **", admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer

        return;
    }

    // find the nickname and kick it, otherwise send a message to admin
    bool nick_exists = false;
    for (int i = 0; i <  MAX_CLIENTS; i++) {
        if (clients[i]){
            if ((strcmp(clients[i]->nick, nickname.c_str()) == 0) && (strcmp(admin->channel, clients[i]->channel) == 0)){
                
                // Set mute
                clients[i]->mute = mute;

                if(mute){
                    // Log to the server
                    cout << "> Server: " << admin->nick << " mute " << nickname << "." << endl; 
                    
                    // Send to the admin
                    sprintf(message, "** You muted %s **", nickname.c_str());
                    sendToOne(message, admin->uid);
                    memset( message, 0, BUFFER_SZ ); // Clears buffer

                    // Send to the client
                    sprintf(message, "** You were muted by %s **", admin->nick);
                    sendToOne(message, clients[i]->uid);
                    memset( message, 0, BUFFER_SZ ); // Clears buffer
                }

                else{
                    // Log to the server
                    cout << "> Server: " << admin->nick << " unmute " << nickname << "." << endl; 
                    
                    // Send to the admin
                    sprintf(message, "** You unmuted %s **", nickname.c_str());
                    sendToOne(message, admin->uid);
                    memset( message, 0, BUFFER_SZ ); // Clears buffer

                    // Send to the client
                    sprintf(message, "** You were unmuted by %s **", admin->nick);
                    sendToOne(message, clients[i]->uid);
                    memset( message, 0, BUFFER_SZ ); // Clears buffer
                }

                nick_exists = true; 
                break;
            }
        }
    }

    if (!nick_exists) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tried /mute or /unmute " << nickname << " but " << nickname << " doesn't exist." << endl;

        sprintf(message, "** User %s doesn't exist in %s channel **", nickname.c_str(), admin->channel);
        sendToOne(message, admin->uid);
        memset( message, 0, BUFFER_SZ ); // Clears buffer
    }
}

void serverCommandSet(const char *s, client_t *client) {
    // Char to string
    string str;
    str = s;

    // Split command and command argument
    const char separator = ' ';
    string command;
    string command_arg;
    stringstream streamData(str);
    getline(streamData, command, separator);
    getline(streamData, command_arg);

    if (command == "/quit") {
        cout << "** /quit called by " << client->nick << " **" << endl;
        announceDisconnect(client);

    } else if (command == "/ping") {
        cout << "** /ping called by " << client->nick << " **" << endl;
        sendToOne("pong!", client->uid);

    } else if (command == "/join") {
        cout << "** /join called by " << client->nick << " **" << endl;
        
        if ( RFC_channelName( command_arg ) )
            joinChannel(command_arg, client);
        else 
            sendToOne( "ERROR: Channel name not formated!", client->uid);

    } else if (command == "/nickname") {
        cout << "** /nickname called by " << client->nick << " **" << endl;
        changeNickname(command_arg, client);

    } else if (command == "/kick") {
        cout << "** /kick called by " << client->nick << " **" << endl;
        kickUser(command_arg, client);

    } else if (command == "/mute") {
        cout << "** /mute called by " << client->nick << " **" << endl;
        muteUser(true, command_arg, client);

    } else if (command == "/unmute") {
        cout << "** /unmute called by " << client->nick << " **" << endl;
        muteUser(false, command_arg, client);

    } else if (command == "/whois") {
        cout << "** command /whois from " << client->nick << " **" << endl;
        whoIs(command_arg, client);

    } else {
        sendToOne("Unknown command!", client->uid);
    }

    return;
}

/* Handle all communication with the client */
void handleClient(client_t *currentClient) {
    char sendBuff[BUFFER_SZ];
    string aux = "";
    char nickBuff[50];

    clientCount.fetch_add(1);  // Increments the atomic counter
    cout << "> Server: Clients Online: " << clientCount.load() << " -> "
        << nickBuff << " ( uid = " << currentClient->uid << " )" << endl;

    // Receive the nickname
    recv(currentClient->sockfd, nickBuff, 50, 0);

    // Join anouncement
    if (strlen(nickBuff) < 2 || strlen(nickBuff) - 1 > 50) {
        cout << "Didn't enter the name.\n";
    } else {
        strcpy(currentClient->nick, nickBuff);
        sprintf(sendBuff, "** %s joined the server **", nickBuff);
        sendToAll(sendBuff); //sendToAll(sendBuff, currentClient->channel);
    }

    while (!(currentClient->leave_flag)) {
        memset(sendBuff, 0, BUFFER_SZ);  // Clears buffer
        int receive = recv(currentClient->sockfd, sendBuff, BUFFER_SZ, 0);
        if (receive > 0) {
            if (strlen(sendBuff) > 0) {

                if(currentClient->mute){
                    sendToOne("** You are muted **", currentClient->uid);
                    continue;
                }

                // If its a command
                if (sendBuff[0] == '/') {
                    serverCommandSet(sendBuff, currentClient);
                    continue;
                }
                if (strcmp(sendBuff, "ACK") == 0) {
                    continue;
                }

                // Send message to all
                aux = currentClient->nick;
                aux.append(": ");
                aux.append(sendBuff);
                sendToAll(aux.c_str(), currentClient->channel);   
            }
        }
        /* Leave anouncement */
        else if (receive == 0) {
            announceDisconnect(currentClient);
            continue;
        } else {
            cout << "ERROR IN RECEIVE MESSAGE" << endl;
            currentClient->leave_flag = true;
        }
    }

    disconnectClient(currentClient);
    return;
}

void disconnectClient(client_t *currentClient) {
    close(currentClient->sockfd);

    m.lock();  // ***LOCKED***
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == currentClient->uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    m.unlock();  // ***UNLOCKED***

    free(currentClient);
    currentClient = NULL;
    clientCount.fetch_sub(1);  // Decrements the atomic counter
    cout << " > Server: Clients Online: " << clientCount.load() << endl;
}

class Server {
   public:
    Server(unsigned int _port, const char *_ipAddrs) {
        port = _port;
        ipAddrs = _ipAddrs;
        hint = socketHandler();
    }

    int startServer() {
        // Create master socket
        _socket = createSocket(AF_INET, SOCK_STREAM, 0);

        signal(SIGPIPE, SIG_IGN);  // Ignore pipe signals ????

        multipleConnections();  // with same addrs AND port
        socketBind();
        socketListen();

        cout << endl
             << "--------------WELLCOME TO CHATROOM---------------\n"
             << endl;

        int listenFD;
        sockaddr_in client_addrs;
        while (true) {
            socklen_t clientSize = sizeof(sockaddr_in);  // ??

            listenFD = accept(_socket, (sockaddr *)&client_addrs, &clientSize);

            if (listenFD == ERROR) {
                cout << "CONNECTION EXCEPTION: Client could not connect!";
                return ERROR;
            }

            if ((clientCount + 1) == MAX_CLIENTS) {
                cout << "Max clients reached. Rejected: " << endl;
                // Print client addrs and port?
                close(listenFD);
                continue;
            }

            // Client set
            client_t *client = (client_t *)malloc(sizeof(client_t));
            client->address = client_addrs;
            client->sockfd = listenFD;
            client->uid = uid++;
            client->leave_flag = false;
            strcpy(client->channel, "#general");
            client->mute = false;
            client->admin = false;

            // Add client to the queue
            m.lock();  // ***LOCK***
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (!clients[i]) {
                    clients[i] = client;
                    break;
                }
            }
            m.unlock();  // ***UNLOCK***

            thread tr(handleClient, client);
            tr.detach();

            sleep(1);  // Reduce CPU usage ???
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
        hint.sin_addr.s_addr = inet_addr(ipAddrs);  // This accepts IPv6
        return hint;
    }

    int multipleConnections() {
        int opt = 1;
        if (setsockopt(_socket, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            return ERROR;
        }
        return 0;
    }

    int socketConnect() {
        int connectRes = connect(_socket, (sockaddr *)&hint, sizeof(hint));
        if (connectRes == -1) {
            cout << "CONNECT EXCEPTION: Server not reachable!";
            return ERROR;
        } else
            return 0;
    }

    int socketBind() {
        if (bind(_socket, (sockaddr *)&hint, sizeof(hint)) == -1) {
            cout << "BINDING EXCEPTION: Can't bind to IP/Port";
            return ERROR;
        } else
            return 0;
    }

    int socketListen() {
        if (listen(_socket, SOMAXCONN) == -1) {
            cout << "MASTER EXCEPTION: Can't listen";
            return ERROR;
        } else
            return 0;
    }
};
