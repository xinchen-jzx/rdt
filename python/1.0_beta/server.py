from rdt import Receiver
send_addr = ('127.0.0.1', 1234)
recv_addr = ('127.0.0.1', 4321)


def recv_cli():
    receiver = Receiver(recv_addr, send_addr)
    while True:
        try:
            print(f'receiver at {recv_addr}')
            data = receiver.rdt_recv()
            text = data.decode()
            print(f'recv msg: {text}')
        except Exception as e:
            raise e

if __name__ == '__main__':
    recv_cli()