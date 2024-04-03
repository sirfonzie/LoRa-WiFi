# ElasticSearch, LogStash, Kibana stack on Docker
> Woon Jun Wei (2200624)
 
This folder contains all configurations and files to run an ElasticSearch, LogSatsh and Kibana stack on Docker.

## Requirements

- Docker
- Docker Compose

## Instructions

### Mosquitto MQTT Broker

Modify the `mosquitto/mosquitto.conf` file to change the MQTT broker configuration.

### Filebeat

Modify `filebeat.yml` to change the Filebeat configuration. The preconfigured one is set to read logs from the MQTT broker.


## Run

Run this command to start the stack:

```bash
docker compose up -d
```

This will start the ELK stack.


To start the mosquitto MQTT broker, run the following command:
```bash
docker compose -f docker-compose-mosquitto.yml up -d
```

This will start the mosquitto MQTT broker.


### Access

The Kibana dashboard can be accessed at `http://localhost:5601`. The default user and password are `elastic` and `changeme` respectively.

# License

This project is licensed under the Apache License - see the [LICENSE](LICENSE) file for details.

# Analysis
Please view the [Analysis](Analysis.md) file for a detailed analysis of the project.


