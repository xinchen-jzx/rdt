from rdt import Sender
import time
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


def send_file(path):

    sender = Sender(send_addr, recv_addr)
    try:
        with open(path, 'rb') as fd:
            fb = fd.read()
            res = sender.rdt_send(fb)
            if res is False:
                pass
    except Exception as e:
        raise Exception(e)


def send_arr():
    arr = ['1', '2', '3', '4', '5', '6', '7']
    sender = Sender(send_addr, recv_addr, 3)
    arr_i = 0
    arr_len = len(arr)
    while arr_i < arr_len:
        if not sender.rdt_send(arr[arr_i].encode()):
            # False means window full
            time.sleep(0.2)
        else:
            # return True means buff is not full
            # continue send
            arr_i += 1
            continue

    sender.close()


if __name__ == '__main__':
    try:
        send_arr()
    except Exception as e:
        print(e)
    # send_file('./a.txt')
