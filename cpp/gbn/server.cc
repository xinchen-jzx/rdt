/*
    g++ rdt.cc server.cc -o server.out
    ./server.out
*/

#include "rdt.hh"

#include <iostream>
#include <string.h>
#include <fstream>

using namespace std;

Address send_addr = {ip : "127.0.0.1", port : 1234};
Address recv_addr = {ip : "127.0.0.1", port : 4321};

void recv_cli()
{
    Base receiver = Base(recv_addr, send_addr);
    // receiver.set_bit_err_rate(0.2);
    // receiver.set_pkt_loss_rate(0.2);
    // receiver.set_delay_time(0.5);
    cout << "new receiver at: " << recv_addr.ip << ", port:" << recv_addr.port << endl;

    uint8_t data_buffer[DATA_MAX_SIZE];
    int cnt = 0;
    while (true)
    {
        cout << "===================================================" << endl;
        int res = receiver.rdt_recv(data_buffer, DATA_MAX_SIZE);
        if (res > 0)
        {
            string data((char *)data_buffer);
            cnt++;
            cout << "recv-" << cnt << ", data: " << data << endl;
        }
        else if (res == 0)
        {
            cout << "recv-0" << endl;
            break;
        }
        else
        {
            cout << "recv error!" << endl;
        }
    }
}

void recv_file()
{
    uint8_t server_buffer[DATA_MAX_SIZE];

    Base receiver = Base(recv_addr, send_addr);
    // receiver.set_bit_err_rate(0.2);
    // receiver.set_pkt_loss_rate(0.2);
    // receiver.set_delay_time(0.5);

    ofstream outfile("../rdt_recv.jpg", std::ios::binary);
    size_t bytes_recv = 0;
    size_t bytes_all = 0;
    size_t cnt = 0;
    while (true)
    {
        bytes_recv = receiver.rdt_recv(server_buffer, DATA_MAX_SIZE);
        if (bytes_recv > 0)
        {
            cnt++;
            bytes_all += bytes_recv;
            outfile.write(reinterpret_cast<char *>(server_buffer), bytes_recv);

            cout << "recv-" << cnt << ", bytes_recv: " << bytes_recv << endl;
        }
        else if (bytes_recv == 0)
        {
            cout << "recv-0" << endl;
            cout << "bytes_all: " << bytes_all << endl;
            break;
        }
        else
        {
            cout << "recv error!" << endl;
        }
    }
    outfile.close();
}
int main(int argc, char *argv[])
{
    // recv_cli();
    recv_file();
}