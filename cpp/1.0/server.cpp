#include "rdt.h"
#include <iostream>
using namespace std;
/*
g++ rdt.cpp server.cpp -o server.out
*/
int main()
{
    Receiver receiver(4321);
    while (1)
    {
        std::string data = receiver.deliver_data();
        cout << "Delivered data: " << data << endl;
    }

    return 0;
}