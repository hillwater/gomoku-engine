#!/bin/bash

IMAGE=hillwater/gomoku-engine:2.0.0

CLOUDAMQP_URL=amqp://hillwater:hillwater@172.17.0.1:5672/gomoku

docker login

docker run -e "CLOUDAMQP_URL=$CLOUDAMQP_URL" -d $IMAGE
