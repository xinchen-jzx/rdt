from rdt import Receiver

if __name__ == '__main__':
    receiver = Receiver(1234)
    while True:
        data = receiver.deliver_data()
        print('Delivered:', data)
        if data == 'World':
            break