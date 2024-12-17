from rdt import Receiver


def cli():
    send_addr = ('localhost', 1234)
    recv_addr = ('localhost', 4321)

    receiver = Receiver(recv_addr, send_addr)
    while True:
        try:
            print(f'receiver at {recv_addr}')
            data = receiver.rdt_recv()
            text = data.decode()
            print(f'recv msg: {text}')
        except Exception as e:
            raise e


def recv_file(path):
    send_addr = ('localhost', 1234)
    recv_addr = ('localhost', 4321)
    receiver = Receiver(recv_addr, send_addr)

    try:
        with open(path, 'wb') as fd:
            data = receiver.rdt_recv()
            fd.write(data)
    except Exception as e:
        print(e)


if __name__ == '__main__':
    recv_file("./b.txt")
