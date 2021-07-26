#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "client.h"
#include "json.h"


extern struct player players[NUM_PLAYERS];


void send_data(int fd, char *format, ...);

void send_json_data(int fd, json_object *object);

void broadcast(int sender_fd, char *format, ...);

void broadcast_json(int sender_fd, json_object *object);

void start_server(uint16_t port);
