from rdt import Sender


def cli():
    send_addr = ('localhost', 1234)
    recv_addr = ('localhost', 4321)
    sender = Sender(send_addr, recv_addr)
    while True:
        try:
            print(f'sender at {send_addr}')
            input_text = input('请输入消息：\n')
            sender.rdt_send(input_text.encode())
        except Exception as e:
            raise Exception(e)


def send_file(path):
    send_addr = ('localhost', 1234)
    recv_addr = ('localhost', 4321)
    sender = Sender(send_addr, recv_addr)
    try:
        with open(path, 'rb') as fd:
            fb = fd.read()
            sender.rdt_send(fb)
    except Exception as e:
        raise Exception(e)


if __name__ == '__main__':
    send_file('./a.txt')
