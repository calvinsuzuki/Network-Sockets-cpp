#include "../Socket/Client.cpp"

using namespace std;

#define ERROR -1
#define LENGTH 4096

string askName() {
    string name;

    printf("Please enter your name: ");
    getline(cin, name);

    // // Transform to array of char
    // char name_char[name.length()];
    // strcpy( name_char, name.c_str() );
    // str_trim_lf(name, strlen(name));

    if (name.length() > 32 || name.length() < 2) {
        printf("Name must be less than 30 and more than 2 characters.\n");
        return "";
    }
    return name;
}

int main() {
    string name;  

    // Handle Cntrl^C
    signal(SIGINT, catch_ctrl_c_and_exit);

	name = askName();
	
    Client client(name.c_str(), 54400, "127.0.0.1");
    
    client.startClient();
}