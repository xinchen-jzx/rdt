import curses
import time
import threading
from rdt import Receiver, Dychar
send_addr = ('127.0.0.1', 1234)
recv_addr = ('127.0.0.1', 4321)






# 初始化curses
stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.keypad(1)
curses.curs_set(0)
stdscr.nodelay(1)


def recv_cli(stdscr):
    receiver = Receiver(recv_addr, send_addr, 3)
    row = 2
    while True:
        data = receiver.rdt_recv()
        text = data.decode()
        stdscr.addstr(row, 0, f'= msg: {text}')
        row += 1
        stdscr.refresh()


def main(stdscr):
    stdscr.clear()
    dychar = Dychar()
    
    recv_cli_thread = threading.Thread(
        target=recv_cli, args=(stdscr,), daemon=True)
    recv_cli_thread.start()
    while True:
        stdscr.addstr(0, 0, f'receiver at {recv_addr}: {dychar.ch}')
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
