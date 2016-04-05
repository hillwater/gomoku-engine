#!/bin/bash

IMAGE=hillwater/gomoku-engine:1.0.0

# should set environment variable CLOUDAMQP_URL to rabbitmq url

sudo docker run -e "CLOUDAMQP_URL=$CLOUDAMQP_URL" -d $IMAGE
