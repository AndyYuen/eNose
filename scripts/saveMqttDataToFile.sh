#! /bin/bash

# Utility script to receive data  from the 'sensorData' topic, 
# reformat them into proper json and 
# save the data on file.

if [ $#  -lt 3  ]; then
	echo "Usage: $0 mqttServerIP mqttServerPort targetFilePath"
	exit 1
fi

MQTT_SERVER_IP=$1
MQTT_SERVER_PORT=$2
FILE=$3

mosquitto_sub -h $MQTT_SERVER_IP -p $MQTT_SERVER_PORT  -t sensorData -u admin -P admin -C 600 > $FILE

sed -i -e '1i {"dataBlock": ['   -e  's/] }/] },/g' -e '$d' $FILE
echo "] } ]}" >> $FILE
