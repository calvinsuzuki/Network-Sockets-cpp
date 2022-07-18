#include <iostream>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <atomic>

#include <string.h>
#include <string>

using namespace std;

int main() {
    char name[] = "calvin";
    char *str; 

    // strcpy(str, name);
    
    // strcat(str, " has joined");

    sprintf( str, "%s has joined", name);

    cout << "Str before: " << str << endl;

    memset(str, 0, strlen(str));

    cout << "Str after: " << str << endl;

    return 0;
}


// mutex m;
// int x = 0 ;
// atomic<int> y = atomic<int>(0);
// // atomic<int> y{0};

// int func(int a) {

//     m.lock();
//     // sleep(1);
//     x += a;
//     y += a;
//     cout << "Thread: " << x << endl;
//     m.unlock();

//     return 0;
// }

// int main() {

//     int num = 1;    

//     thread tr1( func, num );
//     thread tr2( func, num );
//     tr1.detach();
//     tr2.detach();
    
//     sleep(1);
//     cout << "Main: " << x << endl;
//     return 0;
// }


// MESSAGE FRAGMENTATION LOGIC (WORKING IN THEORY)

/* int main() {
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
} */
