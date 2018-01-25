# Gomoku Engine

## Overview
it's a backend gomoku engine, 
the front side is https://github.com/hillwater/bamboo-ai 
you can have a play on http://www.hillwater.xyz/gomoku 



## how to run

````
export CLOUDAMQP_URL=amqp://localhost

./run.sh
````

## what's inside
it's a node js service.
consume message from AMQP, such as rabbitmq.
the message is a gomoku board state.
this service use an C++ addon module to calculate the target smart position.
send the result to AMQP.
this service only do calcuation, so it's stateless.

The front side, should handle the UI request, and send gomoku board state to AMQP, and receive messages.
the JS side every 1 second to poll the caculation result.
