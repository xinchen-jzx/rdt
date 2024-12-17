import socket
import zlib
import pickle

def cal_checksum(data):
    """
    data: 字符串？其他数据类型怎么办？
    """
    crc32 = zlib.crc32(data)
    return crc32

def is_corrupt(packet):
    return packet['checksum'] != cal_checksum(packet['data'])

def make_pkt(data):
    checksum = cal_checksum(data)
    packet = {
        'checksum': checksum,
        'data': data
    }
    return packet

def extract(packet):
    return packet['data']


class RdtEndPoint:
    def __init__(self):
        self.cnt = 0
        self.send_state = 0
        self.recv_state = 0
    
    def bind(self, addr, port):
        self.addr = addr
        self.port = port
    
    def udt_send(self, packet):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.sendto(pickle.dumps(packet), self.receiver_addr)
        pass

    def rdt_send(self, data, peer_addr, peer_port):
        packet = make_pkt(data)
        self.udt_send()
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.sendto(pickle.dumps(packet), (peer_addr, peer_port))
        self.send_state = 0
        pass

    # def udt_rcv(self, data):
    #     pass

    def rdt_rcv(self):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind(('', self.port))
            packet, self.sender_addr = sock.recvfrom(1024)
        return packet

    def get_data(self):
        packet = self.rdt_rcv()
        if is_corrupt(packet):
            sndpkt = make_pkt("NAK")
            self.udt_send(sndpkt)
        else:
            data = extract(packet)
            sndpkt = make_pkt("ACK")
            self.udt_send(sndpkt)
            return data