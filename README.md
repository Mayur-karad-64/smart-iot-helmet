# smart-iot-helmet

# Smart IoT Safety Helmet for Coal Mine Worker Monitoring and Hazard Detection

## Overview

The Smart IoT Safety Helmet is a wearable industrial safety monitoring system designed to improve coal mine worker safety using IoT and embedded systems technology.

The system continuously monitors environmental and worker safety conditions including hazardous gas levels, temperature, humidity, and worker fall detection. Real-time telemetry data is transmitted wirelessly to a Flask-based IoT dashboard using ESP8266 Wi-Fi communication.

This project demonstrates how IoT-based wearable systems can improve industrial worker safety and emergency response in hazardous environments such as underground coal mines.

---

## Features

* Real-time gas monitoring using MQ135
* Temperature and humidity monitoring using DHT11
* Worker fall detection using MPU6050 tilt sensing
* Emergency buzzer alert system
* OLED live monitoring display
* Real-time Flask dashboard communication
* GPS-based worker tracking architecture
* Wireless telemetry transmission using ESP8266
* Industrial wearable safety monitoring architecture

---

## Hardware Components

* ESP8266 NodeMCU
* DHT11 Temperature and Humidity Sensor
* MQ135 Gas Sensor
* MPU6050 Accelerometer and Gyroscope
* 0.96 inch OLED I2C Display
* Passive Buzzer Module
* NEO-6M GPS Module
* Breadboard and Jumper Wires

---

## Software Used

* Arduino IDE
* Python
* Flask
* HTML
* CSS
* JavaScript

---

## Working Principle

The ESP8266 NodeMCU acts as the central controller and communication unit of the system.

* The DHT11 sensor monitors temperature and humidity.
* The MQ135 sensor detects harmful gases and air quality changes.
* The MPU6050 detects worker falls using tilt-angle monitoring.
* The OLED display shows live monitoring data and alerts.
* The buzzer activates during hazardous conditions.
* The ESP8266 sends real-time JSON telemetry data to a Flask-based IoT dashboard over Wi-Fi.

The system architecture also supports GPS-based worker tracking using the NEO-6M GPS module.

---

## Dashboard Features

* Live sensor monitoring
* Emergency alert visualization
* Worker status indication
* GPS location display
* Real-time telemetry updates
* Wireless monitoring architecture

---

## Applications

* Coal Mine Worker Safety
* Underground Mining Monitoring
* Industrial Safety Systems
* Hazardous Environment Monitoring
* Smart Wearable Safety Systems

---

## Future Scope

* SIM800L GSM Integration
* Real GPS Outdoor Tracking
* Cloud Database Integration
* Mobile Application Integration
* AI-Based Hazard Prediction
* Battery Optimization System
* Multi-Worker Monitoring Architecture

---

## Project Highlights

* Real-time IoT monitoring
* Embedded systems integration
* Industrial safety architecture
* Wearable safety monitoring
* Wireless telemetry communication
* Real-time dashboard visualization

---

## Authors

Developed as a team project for industrial worker safety monitoring using IoT and embedded systems technologies.

---

## License

This project is developed for educational and research purposes.
