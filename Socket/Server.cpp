//#include <sys/socket.h>
//#include <sys/types.h>
//#include <netdb.h>
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
#define MAX_CLIENTS 3

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
    char channel_name[50];
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
                    cout << " > Server: " << client->nick << " is not responding. Disconnecting..." << endl;
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
    if (write(client->sockfd, s, strlen(s)) < 0) {
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
            if (!(clients[i]->leave_flag))
                // sendAndReadACK( s, clients[i] );
                sendWithoutACK(s, clients[i]);
        }
    }
    m.unlock();
}

void announceDisconnect(client_t *client) {
    char sendBuff[50 + 16];
    sprintf(sendBuff, "** %s has left! **", client->nick);
    client->leave_flag = true;
    sendToAll(sendBuff);
    cout << sendBuff << endl;
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
                cout << " >Server: " << s << " -> " << clients[i]->nick << endl;
            }
        }
    }
    m.unlock();
}

void joinChannel(string channel_name, client_t *client) {
    // Check client
    if (!client) {
        return;
    }

    // Analyse all channels and see if its the first one
    bool first = true;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (strcmp(clients[i]->channel_name, channel_name.c_str()) == 0) {
                first = false;
                break;
            }
        }
    }

    cout << "HERE" << endl;
    // Set channel_name on the client struct
    strcpy(client->channel_name, channel_name.c_str());

    // Make the client admin if it's the first one
    client->admin = first;

    // Advertising
    if (first) {
        // Log to the server
        cout << "> Server: " << client->nick << " created the channel " << channel_name << " and is the admin." << endl;

        // Send a message to the client advertising the new nickname
        char message[150];
        sprintf(message, "** You created the channel %s and you are the admin **", channel_name.c_str());
        sendToOne(message, client->uid);
    } else {
        // Log to the server
        cout << "> Server: " << client->nick << " joined the channel " << channel_name << endl;

        // Send a message to the client advertising the new nickname
        char message[150];
        sprintf(message, "** You joined the channel %s**", channel_name.c_str());
        sendToOne(message, client->uid);
    }
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
};

void kickUser(string nickname, client_t *admin){
    char message[BUFFER_SZ];

    if (!admin->admin){
        // Log to the server
        cout << "> Server: " << admin->nick << " tryied to kick " << nickname << " but he/she is not an admin." << endl;

        // Send a message to the admin 
        sendToOne("** You are not an admin **", admin->uid);

        return;
    }

    // find the nickname and kick it, otherwise send a message to admin
    bool nick_exists = false;
    for (int i = 0; i <  MAX_CLIENTS; i++) {
        if (clients[i]){
            if (strcmp(clients[i]->nick, nickname.c_str()) == 0){
                
                // Log to the server
                cout << "> Server: " << admin->nick << " kicked " << nickname << "." << endl;
                
                // Tell the client that he/she was kicked
                sprintf(message, "** You was kicked by %s **", admin->nick);
                sendToOne(message, clients[i]->uid);
                
                // Tell the admin that he/she was kicked the client
                sprintf(message, "** You kicked %s **", nickname.c_str());
                sendToOne(message, admin->uid);
                
                announceDisconnect(clients[i]);
                nick_exists = true;
                break;
            }
        }
    }

    if (!nick_exists) {
        // Log to the server
        cout << "> Server: " << admin->nick << " tryied to kick " << nickname << " but " << nickname << " doesn't exist" << endl;

        sprintf(message, "** User %s doesn't exist. /kick failed **", nickname.c_str());
        sendToOne(message, admin->uid);
    }

    return;
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
    getline(streamData, command_arg, separator);

    if (command == "/quit") {
        cout << "** command /quit from " << client->nick << " **" << endl;
        announceDisconnect(client);

    } else if (command == "/ping") {
        cout << "** command /ping from " << client->nick << " **" << endl;
        sendToOne("pong!", client->uid);

    } else if (command == "/join") {
        cout << "** command /join from " << client->nick << " **" << endl;
        joinChannel(command_arg, client);

    } else if (command == "/nickname") {
        cout << "** command /nickname from " << client->nick << " **" << endl;
        changeNickname(command_arg, client);

    } else if (command == "/kick") {
        cout << "** command /kick from " << client->nick << " **" << endl;
        kickUser(command_arg, client);

    } else if (command == "/mute") {
        cout << "** command /mute from " << client->nick << " **" << endl;

    } else if (command == "/unmute") {
        cout << "** command /unmute from " << client->nick << " **" << endl;

    } else if (command == "/whois") {
        cout << "** command /whois from " << client->nick << " **" << endl;

    } else {
        cout << "** Unknown command " << command << " from " << client->nick << " **" << endl;
        sendToOne(command.c_str(), client->uid);
    }

    return;
}

/* Handle all communication with the client */
void handle_client(client_t *currentClient) {
    char sendBuff[BUFFER_SZ];
    string aux = "";
    char nickBuff[32];

    clientCount.fetch_add(1);  // Increments the atomic counter
    cout << " > Server: Clients Online: " << clientCount.load() << " -> ";

    // Receive the nickname
    recv(currentClient->sockfd, nickBuff, 50, 0);

    // Join anouncement
    if (strlen(nickBuff) < 2 || strlen(nickBuff) - 1 > 32) {
        cout << "Didn't enter the name.\n";
    } else {
        strcpy(currentClient->nick, nickBuff);

        sprintf(sendBuff, "** %s has joined **", nickBuff);  // sendBuff = {NICK} has joined!
        cout << nickBuff << " joined ( uid = " << currentClient->uid << " )" << endl;
        sendToAll(sendBuff);
    }

    while (!(currentClient->leave_flag)) {
        memset(sendBuff, 0, BUFFER_SZ);  // Clears buffer
        int receive = recv(currentClient->sockfd, sendBuff, BUFFER_SZ, 0);
        if (receive > 0) {
            if (strlen(sendBuff) > 0) {
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
                sendToAll(aux.c_str());
                cout << aux << endl;
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
            strcpy(client->channel_name, "#general");
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

            thread tr(handle_client, client);
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
