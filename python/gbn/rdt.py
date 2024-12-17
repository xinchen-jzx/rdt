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
import threading

RATE = 0  # bit err rate
TIME = 0.3
RECV_TIME = 0.1
RECVIVER_DELAY = 0.35

# helper functions
ACK = "ACK".encode()


def cal_checksum(data):
    crc32 = zlib.crc32(data)
    return crc32


def random_bits_err(pkt):
    if random.random() > RATE:
        return pkt
    else:
        p = pickle.loads(pkt)
        p["data"] = "9999".encode()
        return pickle.dumps(p)


def is_corrupt(pkt):
    # 整包格式损坏
    try:
        p = pickle.loads(pkt)
    except Exception as e:
        print(f"corrupted 0! {e}")
        return True
    # data or checksum
    if p["checksum"] != cal_checksum(p["data"]):
        print("corrupted 1!")
        return True
    else:
        return False


def make_pkt(data, seq):
    checksum = cal_checksum(data)
    p = {"checksum": checksum, "seq": seq, "data": data}
    return pickle.dumps(p)


class Sender:
    def __init__(self, addr, paddr, winsz):
        """Sender class

        Args:
            addr (tuple(ip/host, port)): addr
            paddr (tuple(ip/host, port)): paddr
            winsz (int): window size
        """
        self.addr = addr
        self.paddr = paddr

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(addr)
        self.sock.settimeout(RECV_TIME)

        self.winsz = winsz
        self.base = 0           # 最早未确认分组序号
        self.nextseq = 0        # 最小的未使用序号

        self.start_time = None  # 开始时间
        self.is_full = False
        self.over = False       # 信号量，告知listen终止
        
        self.pkt_buf = ["" for i in range(winsz)]   # 包缓冲区
        # 监听线程
        self.listen_t = threading.Thread(target=self.listen)
        self.listen_t.start()

    def close(self):
        self.over = True
        self.listen_t.join()

    def listen(self):
        """Listen func for thread to use
        """
        while True:
            if self.over:
                return
            # print(f"listening at {threading.current_thread().name}")

            if self.start_time and time.time() - self.start_time > TIME:  # 超时重传
                self.start_time = time.time()
                for bi in range(self.base, self.nextseq):
                    self.sock.sendto(self.pkt_buf[bi], self.paddr)
            try:
                recv_pkt, paddr = self.sock.recvfrom(1024)  # 检查一下 paddr
            except Exception as e:
                print(f"Recv time out! {e}")
                continue

            isco = is_corrupt(recv_pkt)
            if not isco: # and right seq?
                recv_p = pickle.loads(recv_pkt)
                self.base = (recv_p["seq"] + 1) % self.winsz
                if self.base == self.nextseq:  # base + 1 赶上 nextseq, 空，未满
                    self.is_full = False
                    self.start_time = None  # stop timer
                else:
                    self.start_time = time.time()
            else:  # 坏包，继续等待
                continue

    def rdt_send(self, data) -> bool:
        """exposed api for upper program to use.

        Args:
            data (bytes): data need to send

        Returns:
            True: inform window is not full
            False: window is full
        """
        if self.is_full:  # 窗口满, 告知上层
            return False

        else:  # 窗口未满, is_full == False
            self.pkt_buf[self.nextseq] = make_pkt(data, self.nextseq)
            self.sock.sendto(self.pkt_buf[self.nextseq], self.paddr)

            if self.nextseq == self.base:  # 未满 and next == base, 空
                self.start_time = time.time()

            self.nextseq = (self.nextseq + 1) % self.winsz
            if self.nextseq == self.base:  # nextseq + 1 赶上 base, 满
                self.is_full = True

            return True  # 未满, 告知上层


class Receiver:
    def __init__(self, addr, paddr, winsz):
        """init func for Receiver

        Args:
            addr (tuple(ip, port)): addr bind on receiver
            paddr (tuple(ip, port)): addr bind on sender (peer)
            winsz (int): window size
        """
        self.addr = addr
        self.paddr = paddr

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(addr)

        self.winsz = winsz
        self.exp_seq = 0    # expected seqnum

    def rdt_recv(self):
        """Exposed API for upper to use.

        Returns:
            data: (bytes)
        """

        while True:
            try:
                recv_pkt, paddr = self.sock.recvfrom(1024)  # 检查一下 paddr

            except Exception as e:
                print(f"Recv time out! {e}")
                continue

            recv_pkt = random_bits_err(recv_pkt)  # bit 差错包
            isco = is_corrupt(recv_pkt)

            if not isco:  # not corrupt
                recv_p = pickle.loads(recv_pkt)
                if recv_p["seq"] == self.exp_seq:  # 顺序
                    send_pkt = make_pkt(ACK, self.exp_seq)
                    self.sock.sendto(send_pkt, self.paddr)
                    self.exp_seq = (self.exp_seq + 1) % self.winsz
                    return recv_p["data"]

            # corrupt or disorder
            send_pkt = make_pkt(ACK, (self.exp_seq - 1) % self.winsz)
            self.sock.sendto(send_pkt, self.paddr)
            continue


class Dychar:
    def __init__(self, dychars=None) -> None:
        self.dychars = ['|', '/', '-', '\\', '|']
        if dychars:
            self.dychars = dychars

        self.i = 0
        self.l = len(self.dychars)

    @property
    def ch(self):
        ch = self.dychars[self.i]
        self.i = (self.i + 1) % self.l
        return ch
