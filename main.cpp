#include <iostream>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>

#include <string.h>
#include <string>

using namespace std;

mutex m;
int x = 0 ;
atomic<int> y = atomic<int>(0);

int func(int a) {

    y.fetch_add( 1 );

    return 0;
}

int main() {

    vector<thread> thread_pool;
    thread_pool.reserve(10);

    int num = 1;    

    for( int i = 0; i < 10; i++) {
        thread_pool[i] = thread( func, num );
        thread_pool[i].detach();
    }
    sleep(3);
    cout << "Main: " << y.load() << endl;
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
            // plain_text = plain_text.substr(0, BUFFER_SZ-5);
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
