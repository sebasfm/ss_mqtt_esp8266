#ifndef STRUCTS_H
#define STRUCTS_H

struct mqttConfig {
    char mqtt_server[40];
    char mqtt_port[6];
    char mqtt_id[40];
    char mqtt_user[40];
    char mqtt_pass[40];
};

#endif