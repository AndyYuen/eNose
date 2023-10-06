import random
import requests
from paho.mqtt import client as mqtt_client

###########################################################
# MQTT Broker info
###########################################################
broker = '192.168.1.153'
port = 1883
topic = "sensorData"
client_id = f'eNose-{random.randint(0, 100)}'
username = 'admin'
password = 'admin'
###########################################################
# Smell Detection REST service
###########################################################
REST_URL = "http://127.0.0.1:7000/v1/odour-detection"
HEADERS = {"Content-Type":"application/json"}
###########################################################
# Process only 1 in N messages to avoid piling up messages
###########################################################
RPOCESS_EVERY_1_IN_N_MESSAGES = 3
###########################################################
msg_count = 0

def process_sensor_data(sensor_data):
    response = requests.post(REST_URL, data=sensor_data, headers=HEADERS)
    print(response.json())

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker: {}".format(broker))
        else:
            print("Failed to connect, erorr {}}".format(rc))

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client, topic):
    def on_message(client, userdata, msg):
        global msg_count
        msg_count += 1
        if msg_count % RPOCESS_EVERY_1_IN_N_MESSAGES == 0:
            process_sensor_data(msg.payload.decode())
            #print(f"Received `{message_received}` from `{msg.topic}` topic")

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client, topic)
    client.loop_forever()


if __name__ == '__main__':
    run()