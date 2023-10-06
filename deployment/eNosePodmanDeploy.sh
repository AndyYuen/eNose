#! /bin/bash

# Create a pod with 2 containers: enose-api and enose-app

if [ $#  -lt 2  ]; then
	echo "Usage: $0 mqttServerIP mqttServerPort"
	exit 1
fi

MQTT_SERVER_IP=$1
MQTT_SERVER_PORT=$2

# create pod
podman pod create \
--name enose \
-p 7000:7000 \
-p 7005:7005


# deploy enose AI model rest API  container 
podman run -d --rm --pod enose \
--name enose-api quay.io/andyyuen/enose-api:1.0

# deploy enose app container
podman run -d --rm --pod enose \
-e MQTT_SERVER_IP=$MQTT_SERVER_IP \
-e MQTT_SERVER_PORT=$MQTT_SERVER_PORT \
--name enose-app quay.io/andyyuen/enose-app:1.0

exit 0
