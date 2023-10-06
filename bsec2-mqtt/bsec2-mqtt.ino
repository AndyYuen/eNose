/**
 * Copyright (C) 2021 Bosch Sensortec GmbH
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* The new sensor needs to be conditioned before the example can work reliably. You may run this
 * example for 24hrs to let the sensor stabilize.
 */

/**
* This sketch was derived from the basic.ino sketch described below.
*
 * basic.ino sketch :
 * This is an example for illustrating the BSEC virtual outputs using BME688 Development Kit,
 * which has been designed to work with Adafruit ESP32 Feather Board
 * For more information visit : 
 * https://www.bosch-sensortec.com/software-tools/software/bme688-software/
 */

#include <bsec2.h>
#include "commMux.h"

#include "mqtt_datalogger.h"
#include <WiFi.h>
#include <PubSubClient.h>

/* Macros used */
/* Number of sensors to operate*/
#define NUM_OF_SENS    8
#define PANIC_LED   LED_BUILTIN
#define ERROR_DUR   1000

// WiFi parameters
const char* ssid = "myssid";
const char* password = "mypassword";

// MQTT parameters
const char* mqttServer = "192.168.1.100";
const int mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "admin";
const char* mqttTopic = "sensorData";
const char* mqttClientName = "BME688";

// create the clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// create MQTT logger
bme68xData sensorData[NUM_OF_SENS] = {0};
mqttDataLogger logger(&mqttClient, NUM_OF_SENS, mqttTopic);

void reconnect() {
  // mqttClient.setServer(mqttServer, mqttPort);
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqttClientName, mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/* Helper functions declarations */
/**
 * @brief : This function toggles the led when a fault was detected
 */
void errLeds(void);

/**
 * @brief : This function checks the BSEC status, prints the respective error code. Halts in case of error
 * @param[in] bsec  : Bsec2 class object
 */
void checkBsecStatus(Bsec2 bsec);

/**
 * @brief : This function is called by the BSEC library when a new output is available
 * @param[in] input     : BME68X sensor data before processing
 * @param[in] outputs   : Processed BSEC BSEC output data
 * @param[in] bsec      : Instance of BSEC2 calling the callback
 */
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);

/* Create an array of objects of the class Bsec2 */
Bsec2 envSensor[NUM_OF_SENS];
commMux communicationSetup[NUM_OF_SENS];
uint8_t bsecMemBlock[NUM_OF_SENS][BSEC_INSTANCE_SIZE];
uint8_t sensor = 0;

/* Entry point for the example */
void setup(void)
{
    /* Desired subscription list of BSEC2 outputs */
    bsecSensor sensorList[] = {
            BSEC_OUTPUT_IAQ,
            BSEC_OUTPUT_RAW_TEMPERATURE,
            BSEC_OUTPUT_RAW_PRESSURE,
            BSEC_OUTPUT_RAW_HUMIDITY,
            BSEC_OUTPUT_RAW_GAS,
            BSEC_OUTPUT_STABILIZATION_STATUS,
            BSEC_OUTPUT_RUN_IN_STATUS
    };

    /* Initialize the communication interfaces */
    Serial.begin(115200);
    commMuxBegin(Wire, SPI);
    pinMode(PANIC_LED, OUTPUT);
    delay(100);
    /* Valid for boards with USB-COM. Wait until the port is open */
    while(!Serial) delay(10);

     // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(WiFi.status());
        delay(500);
          Serial.print(".");
    }

    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setBufferSize(600);
    Serial.print("MQTT client buffer size: ");
    Serial.println(mqttClient.getBufferSize());

    reconnect();
    
    logger.beginSensorData();
    for (uint8_t i = 0; i < NUM_OF_SENS; i++)
    {        
        /* Sets the Communication interface for the sensors */
        communicationSetup[i] = commMuxSetConfig(Wire, SPI, i, communicationSetup[i]);

        /* Assigning a chunk of memory block to the bsecInstance */
         envSensor[i].allocateMemory(bsecMemBlock[i]);

        /* Initialize the library and interfaces */
        if (!envSensor[i].begin(BME68X_SPI_INTF, commMuxRead, commMuxWrite, commMuxDelay, &communicationSetup[i]))
        {
            checkBsecStatus (envSensor[i]);
        }

        /* Subscribe to the desired BSEC2 outputs */
        if (!envSensor[i].updateSubscription(sensorList, ARRAY_LEN(sensorList), BSEC_SAMPLE_RATE_LP))
        {
            checkBsecStatus (envSensor[i]);
        }

        /* Whenever new data is available call the newDataCallback function */
        envSensor[i].attachCallback(newDataCallback);

       
    }

    Serial.println("BSEC library version " + \
            String(envSensor[0].version.major) + "." \
            + String(envSensor[0].version.minor) + "." \
            + String(envSensor[0].version.major_bugfix) + "." \
            + String(envSensor[0].version.minor_bugfix));
}

/* Function that is looped forever */
void loop(void)
{
    /* Call the run function often so that the library can 
     * check if it is time to read new data from the sensor  
     * and process it.
     */
    for (sensor = 0; sensor < NUM_OF_SENS; sensor++)
    {
        if (!envSensor[sensor].run())
        {
         checkBsecStatus(envSensor[sensor]);
        }
    }
    reconnect();
}

void errLeds(void)
{
    while(1)
    {
        digitalWrite(PANIC_LED, HIGH);
        delay(ERROR_DUR);
        digitalWrite(PANIC_LED, LOW);
        delay(ERROR_DUR);
    }
}

void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec)
{
    if (!outputs.nOutputs)
    {
        return;
    }

    Serial.println("BSEC outputs:\n\tsensor num = " + String(sensor));
    Serial.println("\ttimestamp = " + String((int) (outputs.output[0].time_stamp / INT64_C(1000000))));
    for (uint8_t i = 0; i < outputs.nOutputs; i++)
    {
        const bsecData output  = outputs.output[i];
        switch (output.sensor_id)
        {
            case BSEC_OUTPUT_IAQ:
                Serial.println("\tiaq = " + String(output.signal));
                Serial.println("\tiaq accuracy = " + String((int) output.accuracy));
                break;
            case BSEC_OUTPUT_RAW_TEMPERATURE:
                Serial.println("\ttemperature = " + String(output.signal));
                sensorData[sensor].temperature = output.signal;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                Serial.println("\tpressure = " + String(output.signal));
                sensorData[sensor].pressure = output.signal;
                break;
            case BSEC_OUTPUT_RAW_HUMIDITY:
                Serial.println("\thumidity = " + String(output.signal));
                sensorData[sensor].humidity = output.signal;
                break;
            case BSEC_OUTPUT_RAW_GAS:
                Serial.println("\tgas resistance = " + String(output.signal));
                sensorData[sensor].gas_resistance = output.signal;
                break;
            case BSEC_OUTPUT_STABILIZATION_STATUS:
                Serial.println("\tstabilization status = " + String(output.signal));
                break;
            case BSEC_OUTPUT_RUN_IN_STATUS:
                Serial.println("\trun in status = " + String(output.signal));
                break;
            default:
                break;
        }
    }
     logger.assembleAndPublishSensorData(sensor, &sensorData[sensor]);
}

void checkBsecStatus(Bsec2 bsec)
{
    if (bsec.status < BSEC_OK)
    {
        Serial.println("BSEC error code : " + String(bsec.status));
        errLeds(); /* Halt in case of failure */
    }
    else if (bsec.status > BSEC_OK)
    {
        Serial.println("BSEC warning code : " + String(bsec.status));
    }

    if (bsec.sensor.status < BME68X_OK)
    {
        Serial.println("BME68X error code : " + String(bsec.sensor.status));
        errLeds(); /* Halt in case of failure */
    }
    else if (bsec.sensor.status > BME68X_OK)
    {
        Serial.println("BME68X warning code : " + String(bsec.sensor.status));
    }
}
