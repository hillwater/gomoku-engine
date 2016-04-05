#!/bin/bash

IMAGE=hillwater/gomoku-engine:1.0.0

sudo docker build -t $IMAGE .

sudo docker push $IMAGE 
