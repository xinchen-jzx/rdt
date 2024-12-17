/*
    g++ rdt.cc client.cc -o client.out
    ./client.out
*/

#include "rdt.hh"
#include <string.h>
#include <iostream>
using namespace std;
int main(int argc, char *argv[])
{
    Address send_addr = {ip : "127.0.0.1", port : 1234};
    Address recv_addr = {ip : "127.0.0.1", port : 4321};

    Sender sender = Sender(send_addr, recv_addr);
    string input;
    cout << "sender at: " << send_addr.ip << ", port:" << send_addr.port << endl;

    while (true)
    {
        cout << "input your msg: ";
        cin >> input;

        bool res = sender.rdt_send(input);
        if (res == true)
        {
            cout << "send: " << input << endl;
        }
        else
        {
            cout << "send error!" << endl;
        }
    }

    // cout << send_addr.ip << " " << send_addr.port << endl;
    // cout << recv_addr.ip << " " << recv_addr.port << endl;
}