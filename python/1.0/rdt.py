# 基于 udp 的可靠数据传输协议

import socket

class Sender:
    def __init__(self, receiver_addr):
        self.receiver_addr = receiver_addr

    def rdt_send(self, data):
        print(f'rdt_send: {data}')
        packet = self.make_pkt(data)
        self.udt_send(packet)
    
    def make_pkt(self, data):
        return data
    
    def udt_send(self, packet):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.sendto(packet.encode(), self.receiver_addr)


class Receiver:
    def __init__(self, port):
        self.port = port

    def udt_recv(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(('', self.port))
            packet, sender_addr = sock.recvfrom(1024)
        return packet.decode()
    
    def extract(self, packet):
        return packet
    

    def deliver_data(self):
        packet = self.udt_recv()
        data = self.extract(packet)
        return data
 
if __name__ == '__main__':
    receiver = Receiver(1234)
    sender = Sender(('localhost', 1234))

    sender.rdt_send('Hello')
    sender.rdt_send('World')

    while True:
        data = receiver.deliver_data()
        print('Delivered:', data)
        if data == 'World':
            break