from machine import UART
import time

rate=115200

uart0=UART(0, baudrate=rate)
uart1=UART(1, baudrate=rate)
uart0.init(bits=8, parity=None, stop=2)
uart1.init(bits=8, parity=None, stop=2)

while True:
    uart0.write('U')
    message=uart0.read()
    if message != None:
        uart1.write('X')
    time.sleep(1)