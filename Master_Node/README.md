# Master Node Connector README

> Woon Jun Wei (2200624)
## Project Overview  

This project is a master node for a multi-protocol mesh with reactive switching function. The master node is connected to a Raspberry Pi 4B via USB. The data is serialized into JSON and published via MQTT to a broker.  ## Prerequisites  
 
- Python 3.7 or higher
- pip (Python package installer)
- A Raspberry Pi 4B with USB connection
- An MQTT broker

## Installation  
1. Clone the repository to your local machine.  
2. Navigate to the project directory.  
3. It is recommended to create a virtual environment to isolate the project dependencies. You can do this by running:  
```bash python3 -m venv venv ```  
4. Activate the virtual environment:
- On Windows, run: `venv\Scripts\activate`
- On Unix or MacOS, run: `source venv/bin/activate`

5. Install the necessary packages using pip:
```bash pip install -r requirements.txt ```  

## Usage  
Ensure that the Raspberry Pi 4B is connected to your machine via USB.  
Run the `connector.py` script to perform the serialization and publishing to the MQTT broker:  
```bash python connector.py ```  
This script reads data from the serial port, serializes it into JSON, and publishes it to an MQTT broker. The MQTT broker settings are loaded from a `config.ini` file.  

Please ensure that the `config.ini` file is properly configured with your MQTT broker settings.  

## Configuration  
The `config.ini` file should contain the following sections and options:  
```ini 
[mqtt] 
broker_host = <your_broker_host> 
broker_port = <your_broker_port> 
username = <your_username> password = <your_password> 
```  

Replace `<your_broker_host>`, `<your_broker_port>`, `<your_username>`, and `<your_password>` with your actual MQTT broker settings.  

## Note  
The project includes C++ code for the master node, which is responsible for the multi-protocol mesh with reactive switching function. 
This code is intended to be uploaded to the master node device and is not run directly as part of the Python script. 