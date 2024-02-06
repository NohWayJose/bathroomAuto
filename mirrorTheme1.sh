#!/bin/bash
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/nightLightCol" -m "3b0000"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/BGLightCol" -m "fff2f3"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/fiveMinTickCol" -m "ffaaff"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/fifteenMinTickCol" -m "a200ff"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/minCol" -m "55ff00"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/hourCol" -m "005500"
mosquitto_pub -h 192.168.7.245 -p 1883 -t "bathroom/mirror/light" -m "OFF"
