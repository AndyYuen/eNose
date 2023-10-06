#include "mqtt_datalogger.h"

mqttDataLogger::mqttDataLogger(PubSubClient *client, int totalSensorCount, const char *mqttTopic) {
  _client = client;
  _sensor_ss.setf(std::ios::fixed, std::ios::floatfield);
  _topic = mqttTopic;
  _sensorCount = 0;
   _totalSensorCount = totalSensorCount;
}

void mqttDataLogger::beginSensorData() {
  _sensor_ss.clear();
  _sensor_ss.str("");

  _sensorCount = 0;
  _sensor_ss << "{ \"datapoints\" : [\n";
}

	/*!
	 * @brief : This function writes the sensor data to the current log file.
	 * 
	 * @param[in] sensorId 			: sensor number
	 * @param[in] bme68xData		: pointer to bme68x data, if NULL a null json object is inserted
         * 
         * @return  nothing
	 */
void mqttDataLogger::assembleAndPublishSensorData(const int sensorId, const bme68xData* sensorData) {

	_sensor_ss << "[ ";
	_sensor_ss << (int) sensorId;
	_sensor_ss << ", ";
	_sensor_ss << millis();
	_sensor_ss << ", ";
	(sensorData != nullptr) ? (_sensor_ss << sensorData->temperature) : (_sensor_ss << "null");
	_sensor_ss << ", ";
	(sensorData != nullptr) ? (_sensor_ss << (sensorData->pressure * .01f)) : (_sensor_ss << "null");
	_sensor_ss << ", ";
	(sensorData != nullptr) ? (_sensor_ss << sensorData->humidity) : (_sensor_ss << "null");
	_sensor_ss << ", ";
	(sensorData != nullptr) ? (_sensor_ss << sensorData->gas_resistance) : (_sensor_ss << "null");
	_sensor_ss << " ]";
  _sensorCount += 1;
  if (_sensorCount <= _totalSensorCount - 1) {
    _sensor_ss << ",\n";
  } else {
    _sensor_ss << "\n] }";
    // publishSensorData();
    if (! publishSensorData()) {
       Serial.print("MQTT failed: ");
       Serial.println(_client->state());;
     }
    beginSensorData();
  }

}

bool mqttDataLogger::publishSensorData() {

  std::string txt;
  txt = _sensor_ss.str();
  const char *data = txt.c_str();

  Serial.println(data);
  if (_client == NULL) return true;

  return _client->publish(_topic, data);
}
	
