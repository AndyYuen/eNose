"""

This application implements an eNose to detect different smells by:
    - retrieving sensor data from a MQTT topic, 
    - calling a REST API to detemine the smell (Air, Alcohol or Coffee)
    - broadcasting the detection t clients via a websocket 

"""

import eventlet
import json
import os
import requests
from flask import Flask, render_template, request
from flask_mqtt import Mqtt
from flask_socketio import SocketIO
from flask_bootstrap import Bootstrap
from TransientAvoidance import TransientAvoidance

eventlet.monkey_patch()

app = Flask(__name__)
app.config['SECRET'] = 'my secret key'
app.config['TEMPLATES_AUTO_RELOAD'] = True
app.config['MQTT_BROKER_URL'] = os.environ['MQTT_SERVER_IP']
app.config['MQTT_BROKER_PORT'] = int(os.environ['MQTT_SERVER_PORT'])
app.config['MQTT_USERNAME'] = 'admin'
app.config['MQTT_PASSWORD'] = 'admin'
app.config['MQTT_KEEPALIVE'] = 5
app.config['MQTT_TLS_ENABLED'] = False

# Parameters for SSL enabled
# app.config['MQTT_BROKER_PORT'] = 8883
# app.config['MQTT_TLS_ENABLED'] = True
# app.config['MQTT_TLS_INSECURE'] = True
# app.config['MQTT_TLS_CA_CERTS'] = 'ca.crt'

mqtt = Mqtt(app)
mqtt.subscribe('sensorData')
socketio = SocketIO(app)
bootstrap = Bootstrap(app)
img = os.path.join('static', 'Image')
suppress = TransientAvoidance()

###########################################################
# Smell Detection REST service
###########################################################
REST_URL = "http://127.0.0.1:7000/v1/odour-detection"
HEADERS = {"Content-Type":"application/json"}
###########################################################
# Process only 1 in N messages to avoid piling up messages
###########################################################
RPOCESS_EVERY_1_IN_N_MESSAGES = 1
###########################################################
msg_count = 0

def process_sensor_data(sensor_data):
    global suppress 
    response = requests.post(REST_URL, data=sensor_data, headers=HEADERS)
    print(response.json())
    dict = response.json()
    #print(dict['prediction'])
    return suppress.avoidTransient(dict['prediction'])

"""
Serve root
"""
@app.route('/')
def home():
    suppress.reset()
    file = os.path.join(img, 'unknown.gif')
    return render_template('render_image.html', image=file)

"""
Decorator for connect
"""
@socketio.on('connect')
def connect():
    print('Client connected')


"""
Decorator for disconnect
"""
@socketio.on('disconnect')
def disconnect():
    print('Client disconnected',  request.sid)

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global msg_count
    msg_count += 1
    if msg_count % RPOCESS_EVERY_1_IN_N_MESSAGES == 0:
        send, odour = process_sensor_data(message.payload.decode())
        if send:
            socketio.emit('updatePrediction', {'prediction': odour})


if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=7005, use_reloader=False, debug=True)
