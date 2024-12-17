/*
    g++ rdt.cc utils.cc client.cc -o client.out
    ./client.out
*/

#include "rdt.hh"
#include "utils.hh"

#include <iostream>
#include <string.h>
#include <fstream>
// #include <thread>

#include <vector>

using namespace std;

Address send_addr = {ip : "127.0.0.1", port : 1234};
Address recv_addr = {ip : "127.0.0.1", port : 4321};

void send_cli()
{
    Base sender = Base(send_addr, recv_addr);
    sender.set_recv_time(0.1); // set udp recvfrom listen time
    sender.start_listen();
    vector<uint8_t> arr = {'a', 'b', 'c', 'd', 'e', 'f'};

    int arr_idx = 0;
    int arr_len = arr.size();
    for (arr_idx = 0; arr_idx < arr_len; arr_idx++)
    {
        while (!sender.rdt_send(&(arr[arr_idx]), 1))
        {
            cout << "sim_delay" << endl;
            sim_delay(0.2);
        }
    }
    while (!sender.rdt_send(0, 0))
    {
        cout << "sim_delay" << endl;
        sim_delay(0.2);
    }
    sender.stop_listen();
    // while (arr_idx < arr_len)
    // {
    //     if (!sender.rdt_send(&(arr[arr_idx]), 1))
    //     {
    //         cout << "sim_delay" << endl;
    //         sim_delay(0.2);
    //     }
    //     else
    //     {
    //         cout << "sended: " << arr[arr_idx] << endl;
    //         arr_idx++;
    //     }
    // }
    // while (true)
    // {
    //     if (!sender.rdt_send(0, 0))
    //     {
    //         sim_delay(0.2);
    //     }
    //     else
    //     {
    //         cout << "sended: " << 0 << endl;
    //         break;
    //     }
    // }
    // sender.stop_listen();
}

void send_file()
{
    uint8_t client_buffer[DATA_MAX_SIZE];

    Base sender = Base(send_addr, recv_addr);
    sender.set_recv_time(0.1); // set udp recvfrom listen time
    sender.start_listen();

    ifstream file("../rdt.jpg", std::ios::binary);
    // ofstream outfile("../rdt_recv.jpg", std::ios::binary);
    size_t bytes_read;
    size_t bytes_all = 0;
    // int cnt;
    while (file)
    {

        file.read(reinterpret_cast<char *>(client_buffer), sizeof(client_buffer));
        bytes_read = file.gcount();

        if (bytes_read > 0)
        {
            while (!sender.rdt_send(client_buffer, bytes_read))
            {
                cout << "sim_delay" << endl;
                sim_delay(0.2);
            }
        }
        // outfile.write(reinterpret_cast<char *>(client_buffer), bytes_read);

        bytes_all = bytes_all + bytes_read;
        // process bytes in buffer
        cout << "bytes_read: " << bytes_read << endl;
        if (file.eof())
        {
            cout << "eof" << endl;
            while (!sender.rdt_send(0, 0))
            {
                cout << "sim_delay" << endl;
                sim_delay(0.2);
            }
            break;
        }
    }
    cout << "bytes_all: " << bytes_all << endl;
    file.close();
    sender.stop_listen();
    // outfile.close();
}

int main(int argc, char *argv[])
{
    // send_cli();
    send_file();
}