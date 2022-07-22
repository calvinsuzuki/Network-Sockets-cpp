#include "../Socket/Client.cpp"

using namespace std;

string askName() {
    string name;

    while (true) {
        printf("\u001b[33mPlease enter your name: \u001b[0m");

        getline(cin, name);

        if (name.length() - 1 > 50 || name.length() <= 2) {
            printf("Name must be less than 50 and more than 2 characters.\n");
            continue;
        }

        return name;
    }
}

int main() {
    // define the server port and ip
    string line;
    string name;
    int port = 53400;
    char ip[] = "191.179.176.209";

    cout << endl;
    cout << "\u001b[33m**     Welcome to the Golden Chat     **\u001b[0m\n" << endl;

    while ( line != "/connect" ) {
        getline(cin, line);
    }

    name = askName();

    Client client(name.c_str(), port, ip);

    client.startClient();
}