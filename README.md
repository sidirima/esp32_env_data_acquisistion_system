# ESP-32 Environmental Data Acquisition System

The purpose of the project is to design and implement an IoT system capable of measuring environmental parameters in situ. The parameters chosen were those conducive to the optimal management of arable land. Data are then collected via an ESP-32 microcontroller and subsequently transmitted from the node to a gateway, where they are stored in a database. This allows for subsequent data processing and visualization via a web interface.

## Full System Block Diagram
In the Figure below we can see the 3 distinct parts of the project. 
<p align = "center">
  <img src ="www.github.com/sidirima/esp32_env_data_acquisition_system/main/Block_Diagrams/Full_project.png">
</p>
The first one is the **Transmitter Node**, which is responsible for measuring and transmitting the data to a base station.

This project was my Integrated Master's Thesis in Department of Informatics & Electronics Engineering of International Hellenic University (IHU).
I was a contributor to this thesis, alongside Androniki Kolete. Her part of the project (database, web interface) can be found [here]().
I was responsible for the hardware engineering part of the project, including the ESP-32, designing the sensors and the PCB and also the power supply.


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
