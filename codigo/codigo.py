import serial
import threading
import time
from flask import Flask, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

json_data = None

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(json_data)


############################# ARDUINO #############################


SPEED = 150
arduino = serial.Serial('COM3', 9600)

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


if __name__ == "__main__":
    threading.Thread(target=lambda: app.run()).start()
    start_time = time.time()

    while True:
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

            print(data)

        except Exception as e:
            print(e)
            print("Error de lectura")