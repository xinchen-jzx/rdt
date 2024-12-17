import time
import threading
from rdt import Receiver
send_addr = ('127.0.0.1', 1234)
recv_addr = ('127.0.0.1', 4321)


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


dychar = Dychar()


def p():
    while True:
        print(f'receiver at {recv_addr}: {dychar.ch}', end='\r', flush=True)
        time.sleep(0.2)

def recv_cli():
    receiver = Receiver(recv_addr, send_addr, 3)
    while True:
        data = receiver.rdt_recv()
        text = data.decode()
        print(f'recv msg: {text}')

def recv_file(path):
    receiver = Receiver(recv_addr, send_addr)

    try:
        with open(path, 'wb') as fd:
            data = receiver.rdt_recv()
            fd.write(data)
    except Exception as e:
        print(e)


if __name__ == '__main__':
    # recv_file("./b.txt")
    t1 = threading.Thread(target=p)
    t2 = threading.Thread(target=recv_cli, daemon=True)
    try:
        t1.start()
        t2.start()
    except Exception as e:
        t1.join()
        t2.join()
        raise e
