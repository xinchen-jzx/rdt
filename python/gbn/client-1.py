import curses
from rdt import Sender, Dychar
import time
import threading
send_addr = ('127.0.0.1', 1234)
recv_addr = ('127.0.0.1', 4321)

# 初始化curses
stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.keypad(1)
curses.curs_set(0)
stdscr.nodelay(1)


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


def send_arr(stdscr):
    arr = ['1', '2', '3', '4', '5', '6', '7']
    sender = Sender(send_addr, recv_addr, 3)
    arr_i = 0
    arr_len = len(arr)

    rowbase = 2
    idx = 0
    while arr_i < arr_len:
        stdscr.addstr(rowbase + idx % (curses.LINES - 2),
                      0, f'= {idx} send: {arr[arr_i]}')
        idx += 1
        if not sender.rdt_send(arr[arr_i].encode()):
            # False means window full
            time.sleep(0.2)
        else:
            # return True means buff is not full
            # continue send
            arr_i += 1
            continue
    sender.close()


def main(stdscr):
    stdscr.clear()
    dychar = Dychar()

    send_arr_thread = threading.Thread(
        target=send_arr, args=(stdscr,), daemon=True)
    send_arr_thread.start()
    while True:
        stdscr.addstr(0, 0, f'listening at {send_addr} {dychar.ch}')
        stdscr.addstr(1, 0, 'enter q to exit.')
        stdscr.refresh()
        key = stdscr.getch()
        if key == ord('q'):
            break
        time.sleep(0.1)


try:
    curses.wrapper(main)
except KeyboardInterrupt:
    pass
finally:
    curses.endwin()
