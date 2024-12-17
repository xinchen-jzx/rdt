#include "rdt.h"
#include <iostream>
using namespace std;
/*
g++ rdt.cpp client.cpp -o client.out
*/
int main() {
    Address addr;
    addr.ip = "localhost";
    addr.port = 4321;

    Sender sender(addr);
    while (1) {
        std::string data;
        cout << "type your message: ";
        cin >> data;
        sender.rdt_send(data);
    }

    return 0;
}