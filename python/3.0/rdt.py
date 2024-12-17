"""
基于 udp 的可靠数据传输协议

rdt 3.0 添加:
    - 计时器

数据类型:
    - data 字节序列
    - pkt 为打包 {data, checksum, seq} 后的包字节序列
    - p 为字节序列解包后得到的对象
"""
import socket
import zlib
import pickle
import random
import time
RATE = 0  # bit err rate
TIME = 0.3
RECV_TIME = 0.1
RECVIVER_DELAY = 0.35
# helper functions


def cal_checksum(data):
    crc32 = zlib.crc32(data)
    return crc32


def random_bits_err(pkt):
    if random.random() > RATE:
        return pkt
    else:
        p = pickle.loads(pkt)
        p['data'] = "9999".encode()
        return pickle.dumps(p)


def is_corrupt(pkt):
    try:
        p = pickle.loads(pkt)
    except Exception as e:
        print("corrupted 0! {e}")
        return True
    if p['checksum'] != cal_checksum(p['data']):
        print("corrupted 1!")
        return True
    else:
        return False


def make_pkt(data, seq):
    checksum = cal_checksum(data)
    p = {
        'checksum': checksum,
        'seq': seq,
        'data': data
    }
    return pickle.dumps(p)


def udt_send(pkt, addr, paddr):
    """
    不可靠数据传输, send pkt from addr to paddr
    """
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(addr)
        sock.sendto(pkt, paddr)


def udt_recv(addr):
    """
    不可靠数据接收, 监听发给addr的消息
    """

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        try:
            sock.settimeout(RECV_TIME)
            sock.bind(addr)
            pkt, paddr = sock.recvfrom(1024)
            return pkt
        except socket.timeout:
            # print("监听超时")
            return None


class Sender:
    def __init__(self, addr, paddr):
        """
        recv_addr: (hostname, port) or (ipaddr, port)
        """
        self.addr = addr
        self.paddr = paddr
        self.winsz = 1
        self.seq = 0

    def rdt_send(self, data):
        print(f"== send pkt seq: {self.seq}")
        send_pkt = make_pkt(data, self.seq)
        udt_send(send_pkt, self.addr, self.paddr)
        # start timer
        start_time = time.time()
        while True:
            # check timer
            if time.time() - start_time > TIME:  # 超时 resend, start timer
                print("time out: resend!")
                udt_send(send_pkt, self.addr, self.paddr)
                start_time = time.time()
                continue

            recv_pkt = udt_recv(self.addr)
            if recv_pkt is None:
                continue
            isco = is_corrupt(recv_pkt)
            # 坏包 -> continue, 不转移
            if isco:
                print(f"坏包: resend: {self.seq}, continue")
                continue

            recv_p = pickle.loads(recv_pkt)
            # 错序包 -> continue, 不转移
            if recv_p['seq'] == self.last_seq:
                print(
                    f"错序包: recv seq: {recv_p['seq']}, resend: {self.seq}, continue")
                continue
            # 好包 and 正序包 -> 进入下一个等待状态
            elif (not isco) and (recv_p['seq'] == self.seq):
                print(
                    f"收到好包: recv seq: {recv_p['seq']}, sended: {self.seq}, dt: {time.time() - start_time}")

                break
            else:
                raise Exception("Sender else!")
        # 更新seq
        self.seq = (self.seq + 1) % (self.winsz + 1)
        # stop timer

    @property
    def last_seq(self):
        return (self.seq - 1) % (self.winsz + 1)


class Receiver:
    def __init__(self, addr, paddr):
        self.addr = addr
        self.paddr = paddr
        self.winsz = 1
        self.seq = 0

    def rdt_recv(self):
        while True:
            recv_pkt = udt_recv(self.addr)
            if recv_pkt is None:
                continue
            recv_pkt = random_bits_err(recv_pkt)  # bit 差错包
            isco = is_corrupt(recv_pkt)
            # 坏包 -> 重传, 不转移
            if isco:
                print(
                    f"坏包: send: ACK_{self.last_seq}, need: resend {self.seq}")
                send_pkt = make_pkt("ACK".encode(), self.last_seq)
                udt_send(send_pkt, self.addr, self.paddr)
                continue

            recv_p = pickle.loads(recv_pkt)
            # 错序包
            if (recv_p['seq'] == self.last_seq):
                print(
                    f"错序包: recv seq:{recv_p['seq']}, send: ACK_{self.last_seq}")
                send_pkt = make_pkt("ACK".encode(), self.last_seq)
                udt_send(send_pkt, self.addr, self.paddr)
                continue
            # 好包：
            elif (not isco) and (recv_p['seq'] == self.seq):
                print(f"好包: recv seq: {recv_p['seq']}, send: ACK_{self.seq}")
                data = recv_p['data']
                send_pkt = make_pkt("ACK".encode(), self.seq)
                time.sleep(RECVIVER_DELAY)
                udt_send(send_pkt, self.addr, self.paddr)
                break
            else:
                raise Exception("Receicer else!")
        self.seq = (self.seq + 1) % (self.winsz + 1)
        return data

    @property
    def last_seq(self):
        return (self.seq - 1) % (self.winsz + 1)
