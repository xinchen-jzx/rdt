import socket
import pickle
import zlib


def cal_checksum(data):
    crc32 = zlib.crc32(data)
    return crc32


def make_pkt(data, seq):
    checksum = cal_checksum(data)
    p = {"checksum": checksum, "seq": seq, "data": data}
    return pickle.dumps(p)


class Sender:
    def __init__(self, addr, paddr) -> None:
        self.addr = addr
        self.paddr = paddr
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(addr)

    def rdt_send(self, data):
        pkt = make_pkt(data, 0)
        self.sock.sendto(pkt, self.paddr)


class Receiver:
    def __init__(self, addr, paddr) -> None:
        self.addr = addr
        self.paddr = paddr
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(addr)

    def rdt_recv(self):
        pkt, paddr = self.sock.recvfrom(1024)
        p = pickle.loads(pkt)
        return p["data"]
