if [ $#  -lt 2  ]; then
	echo "Usage: $0 mqttServerIP mqttServerPort"
	exit 1
fi

MQTT_SERVER_IP=$1
MQTT_SERVER_PORT=$2

BASENAME=enoseDeploy

if test -f $BASENAME.yaml
then
    rm $BASENAME.yaml
fi

cp $BASENAME.template $BASENAME.yaml

sed -i -e "s/TARGET_MQTT_SERVER_IP/$MQTT_SERVER_IP/g" \
-e "s/TARGET_MQTT_SERVER_PORT/$MQTT_SERVER_PORT/g" \
$BASENAME.yaml