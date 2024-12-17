from rdt import Sender


send_addr = ('127.0.0.1', 1234)
recv_addr = ('127.0.0.1', 4321)


def send_cli():
    sender = Sender(send_addr, recv_addr)
    while True:
        try:
            print(f'sender at {send_addr}')
            input_text = input('请输入消息：\n')
            sender.rdt_send(input_text.encode())
        except Exception as e:
            raise Exception(e)

if __name__ == '__main__':
    send_cli()