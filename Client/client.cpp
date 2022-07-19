#include "../Socket/Client.cpp"

using namespace std;

string askName() {
    string name;

    while ( true ) {

        printf("Please enter your name: ");

        getline( cin, name );

        if (name.length()-1 > 50 || name.length() <= 2) {
            printf("Name must be less than 50 and more than 2 characters.\n");
            continue;
        }

        return name;
    }
}

int main() {
    // string name, line, ip;  
    // int port;

    // do {
    //     cin >> line;
    // } while( line != "/connect" );
    
    // cin >> ip;
    // cin >> port;

    string name;
    name = askName();
	
    Client client(name.c_str(), 53400, "127.0.0.1");
    
    client.startClient();
}