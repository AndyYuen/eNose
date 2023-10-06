#! /bin/bash

# deploy a Eclipse Mosquitto MQTT server

podman  run -it --rm --name mqtt -p 1883:1883 quay.io/andyyuen/mymosquitto
