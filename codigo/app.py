from flask_socketio import SocketIO, emit
import serial
import threading
import time
from flask import Flask, jsonify, request
from flask_cors import CORS
import json

app = Flask(__name__)

socketio = SocketIO(app, mode='threading', debug=True, cors_allowed_origins='*')
stop_event = threading.Event()

json_data = {}

@app.route('/')
def index():
    return "Hello World"

@socketio.on('begin')
def begin():
    global t
    stop_event.clear()
    print("begin")
    t = threading.Thread(target=main_loop)
    t.start()
@socketio.on('stop')
def socket_stop():
    global t
    print("stop")
    stop_event.set()
    t.join()

@socketio.on('connect')
def connect():
    print("Connected")
@socketio.on('disconnect')
def disconnect():
    print("Disconnected")



############################# ARDUINO #############################


SPEED = 150
print("Ran")

def write_speed(speed1, speed2):
    global arduino
    arduino.write(f"{speed1},{speed2}".encode('utf-8'))

def move_forward():
    write_speed(SPEED, SPEED)

def move_right():
    pass

def move_left():
    pass

def turn_left():
    pass

def turn_back():
    pass

def stop():
    write_speed(0,0)


def main_loop():
    arduino = serial.Serial('COM9', 9600)
    global json_data
    start_time = time.time()
    print("main loop")
    while not stop_event.is_set():
        try:
            line = arduino.readline().decode('utf-8').strip()
            data = line.split(',')
            data = list(map(int, data))

            fl = data[0]
            cl = data[1]
            cc = data[2]
            cr = data[3]
            fr = data[4]
            ls = data[5]
            rs = data[6]

            if fr == 1:
                turn_left()
            elif cc == 0:
                turn_back()
            elif cl == 1:
                move_right()
            elif cr == 1:
                move_left()

            json_data = {
                "sensors": [fl, cl, cc, cr, fr],
                "speed": {
                    "left": ls,
                    "right": rs
                },
                "timestamp": round(time.time() - start_time, 2)
            }

            socketio.emit('data', json_data)
            # print(data)

        except Exception as e:
            print(e)
            print("Error de lectura")
    arduino.close()

t = None

if __name__ == '__main__':
    print("main")
    socketio.run(app, host='localhost', debug=True)
    stop_event.set() 



# if __name__ == "__main__":
#     t = threading.Thread(target=run_app)
#     t.start()
#     start_time = time.time()

#     try: 
#         while True:
#             try:
#                 line = arduino.readline().decode('utf-8').strip()
#                 data = line.split(',')
#                 data = list(map(int, data))

#                 fl = data[0]
#                 cl = data[1]
#                 cc = data[2]
#                 cr = data[3]
#                 fr = data[4]
#                 ls = data[5]
#                 rs = data[6]

#                 if fr == 1:
#                     turn_left()
#                 elif cc == 0:
#                     turn_back()
#                 elif cl == 1:
#                     move_right()
#                 elif cr == 1:
#                     move_left()

#                 json_data = {
#                     "sensors": [fl, cl, cc, cr, fr],
#                     "speed": {
#                         "left": ls,
#                         "right": rs
#                     },
#                     "timestamp": round(time.time() - start_time, 2)
#                 }

#                 print(data)

#             except Exception as e:
#                 print(e)
#                 print("Error de lectura")
#     except KeyboardInterrupt:
#         print("ctrlc")
#     finally:
#         arduino.close()
#         print("stopev")
#         t.join()
#         print("Programa terminado")
#         exit()