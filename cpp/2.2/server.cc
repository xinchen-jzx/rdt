/*
    g++ rdt.cc server.cc -o server.out
    ./server.out
*/

#include "rdt.hh"

#include <iostream>
#include <string.h>

#define MY_DBUFFER_SIZE 1024

using namespace std;

int main(int argc, char *argv[])
{
    Address send_addr = {ip : "127.0.0.1", port : 1234};
    Address recv_addr = {ip : "127.0.0.1", port : 4321};

    Receiver receiver = Receiver(recv_addr, send_addr);
    cout << "receiver at: " << recv_addr.ip << ", port:" << recv_addr.port << endl;

    uint8_t data_buffer[MY_DBUFFER_SIZE];
    while (true)
    {
        cout << "===================================================" << endl;
        bool res = receiver.rdt_recv(data_buffer, MY_DBUFFER_SIZE);
        if (res == true)
        {
            string data((char *)data_buffer);
            cout << "recv: " << data << endl;
        }
        else
        {
            cout << "recv error!" << endl;
        }
    }

    // cout << send_addr.ip << " " << send_addr.port << endl;
    // cout << recv_addr.ip << " " << recv_addr.port << endl;
}