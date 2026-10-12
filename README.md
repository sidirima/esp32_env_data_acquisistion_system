# ESP-32 Environmental Data Acquisition System
This project was my Integrated Master's Thesis in Department of Informatics & Electronics Engineering of International Hellenic University (IHU), alongside Androniki Kolete. 
Her part of the project (database, web interface) can be found [here]().
I was responsible for the hardware engineering part of the project, including the ESP-32, designing the sensors and the PCB and also the power supply.

## Purpose - Key Objective
The purpose of the project is to design and implement an IoT system capable of measuring environmental parameters in situ. The parameters chosen were those conducive to the optimal management of arable land. Data are then collected via an ESP-32 microcontroller and subsequently transmitted from the node to a gateway, where they are stored in a database. This allows for subsequent data processing and visualization via a web interface.

Design of the web interface prioritizes the User Experience (UX), creating a user-friendly website which offers many visualization preferences, notifications and statistics about the stored data.

## Full System Block Diagram
In the Figure below we can see the 3 distinct parts of the project. 

![alt text](https://github.com/sidirima/esp32_env_data_acquisistion_system/blob/main/Block_Diagrams/Full_project.png)

The first one is the **_Transmitter Node_**, which is responsible for measuring and transmitting the data to a base station. This node is solar powered via a power supply unit consisting of a Solar Power Manager, a typical 18650 battery and a 5W PV Panel.
The project is scalable, so we can use multiple Transmitter nodes in a field, or in different fields, provided that there is a base station nearby.

Second part is the **_Receiver_** which is the base station for all transmitter nodes. We establish communication between transmitter nodes and the receiver via LoRa protocol. Using LoRa, we can establish a maximum distance of 10 km between a node and the base station and it is also very energy efficient. Placement of the receiver must be done somewhere that there is an available powerline and an internet connection. Reason for that is because data from the receiver are then stored to the database using Wi-Fi.

Third part of the project are the **_Web Services_**, including Frontend and Backend of our Web application. All measured data are formatted and stored in the database for processing using Business Logic.
Afterwards, data visualization or extraction in CSV format is available using the website. 

## Measured Data
The system is capable of measuring 8 different environmental parameters both atmospheric and soil.

#### Atmospheric Data
- Air Temperature: BME-280 sensor
- Air Humidity: BME-280 sensor
- Barometric Pressure: BME-280 sensor
- Rain Depth: Own design, using tipping bucket principle and Hall Effect Sensor A3144
- Wind Speed: Own design, using cup anemometer principle and Hall Effect Sensor A3144
- Wind Direction: Own design, using Hall Effect Sensors A3144

#### Soil Data
- Soil Temperature: Own design, analog sensor using NTC thermistor probe, Wheatstone bridge and instrumentation amplifier topology.
- Soil Moisture: Own design, analog sensor based on Capacitive Soil Moisture Sensors.

## Analog Sensors Working Principles
### Soil Temperature Sensor
As shown in the block diagram below, temperature of soil is calculated using an NTC Thermistor, which is a special kind of resistor capable of varying its resistance according to temperature fluctuations.
We prefer to use a passive resistive element instead of an RTD due to its low power consumption.
<p align = "center">
  <img src= "https://github.com/sidirima/esp32_env_data_acquisistion_system/blob/main/Block_Diagrams/Soil_temperature_sensor.png">
</p>
To measure temperature fluctuations of the NTC Thermistor we use a **Wheatstone bridge** topology, consisting of 4 resistors connected in 2 parallel branches, each consisting of 2 series resistors. We connect a DC voltage source (5V) at one end of the branch as shown in the picture and the other side is connected to ground. When the two branches of the bridge are balanced, the output voltage is 0V.

The differential output of the bridge is then connected to an **Instrumentation Amplifier**, a special Operational Amplifier topology frequently used in analog sensors. This circuit is designed to amplify the very sensitive signals of passive analog sensors. Reason for that is that the differential signals are quite smaller than common mode signals, therefore a common operational amplifier could not offer the desired amplification. Although we can find many instrumentation amplifier ICs ready to use, we prefer to use a simple LMC662 operational amplifier IC due to its significantly lower price. It is important to use a rail-to-rail operational amplifier IC so as to measure most accurate voltage readings.

### Soil Moisture Sensor
On the diagram below, the working principle of our capacitive soil moisture sensor is depicted. The sensor probe consists of two electrodes tucked in soil, creating an electrostatic field. Fluctuations of water content in soil also fluctuate its relative permitivity. Those fluctuatuations are then translated as changes in the capacitance of the sensor. We need a way of measuring that capacitance to determine the water content of soil.

![alt text](https://github.com/sidirima/esp32_env_data_acquisistion_system/blob/main/Block_Diagrams/Soil_moisture_sensor.png)

In order to achieve this, we connect the first of the electrodes with a simple 400 kHz **square wave generator**. The most effective way of creating a desired frequency square wave generatior is using a 555 timer IC (TLC555) connected as an **Astable Multivibrator** circuit. Amplitude of square wave pulse sigmal is 5V due to DC value of the power supply. 

The electrodes of the sensor are depicted as capacitor **C_SOIL** in the block diagram. As the capacitance value fluctuates, cutoff frequency of the RC low - pass filter also fluctuates. 

Using a simple silicon diode (1N4148) as a **peak detector** circuit, we can determine the capacitance value of soil using the cutoff frequency.

## Choosing an Analog-to-Digital Converter (ADC)
The ESP-32 family of microcontrollers has an internal 12-bit **Analog-to-Digital Converter**, although it is recommended to use an external one due to its poor linearity when dealing with accurate sensor signals, specifically when using wireless connections, due to electromagnetic noise. 

In our project, we decided to use the ADS1015, which is a 12-bit resolution ADC as well. This ADC is very commonly used in ESP-32 projects and there are many libraries in Arduino IDE supporting it. This specific IC supports both I²C and SPI serial communication protocols.

## Project Index
#### 1. [Block_Diagrams](/Block_Diagrams)
Contains block diagrams for every hardware part of the project, including a block-diagram of the full project principle.

#### 2. [ESP32_code](/ESP32_code)
Contains the ESP-32 Arduino IDE programming files for Transmitter & Receiver parts of the project.

#### 3. [Schematics](/Schematics)
Contains PDF files of Altium Designer Schematics of Transmitter and Receiver parts.

#### 4. [Altium_PCB_Design](/Altium_PCB_Design)
Contains photos of the PCB and also the Gerber files extracted from Altium Designer to use for PCB manufacturing.

#### 5. [Final_Product_Photos](/Final_Product_Photos)
Contains photos of the final product (transmitter / receiver) and video presentation of Web Interface usability.
