from rdt import Sender

if __name__ == '__main__':
    sender = Sender(('localhost', 1234))
    while True:
        data = input('type your message:')
        sender.rdt_send(data)
    # sender.rdt_send('Hello')

    # sender.rdt_send('World')