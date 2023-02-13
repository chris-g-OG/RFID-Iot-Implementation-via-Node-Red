# RFID-Iot-Implementation-via-Node-Red

The program is used to count the Duration of the presence of a person in a room.
This is an IoT impilimentation using ESP8266(on WeMos D1 R2) and RFID MFRC522 module.

Communication with Node-RED platform via MQTT Broker Mosquitto


# How the program works 
 
Setup : The program initialize the RFID module and connections to WIFI, MQTT broker and NTP server
The functions setup_MQTT() is used to connect to the broker and callback() is used to Subscribe to the message topic that we will receive from the Broker


In the Loop section we read a Card Uid and get the timestamp of the reading with getEpochTime()
Then we convert the Card-Uid and the timestamp to a json format with serializeJson(doc, payload) and publish the data to the Mqtt broker with client.publish(topic, payload)

# Node-Red Flow
The Node-Red flow is used to receive the data from the MQTT broker and store it in a Sqlite database,
return the duration of the presence of the card in the room and show the cards in the room in a UI table.

The flow is composed of 4 parts :
                                    1. MQTT input node
                                    2. Function node
                                    3. Sqlite node
                                    4. Sqlite output node

The flow checks if the card is already in the database and if not it adds it to the database 
If the card is already in the database it checks if the card is in the room or not
If the card is removed from the room it sends mqtt message with the duration of the presence
Else it writes the card in the database with the timestamp of the reading
The UI table shows us the cards in the room and the time they entered the room

Here is the flow :
![image](https://user-images.githubusercontent.com/56930795/218492979-2d19ab92-d89c-4219-9500-860f7afa15be.png)


# Note
I onrder to use getEpochTime() (to get the write timestamp format) you need to use https://github.com/taranais/NTPClient.git this library.

