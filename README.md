# BasicWorker

A daemon that consumes assignments from the message queue, builds and runs them 
and then sends results back.

## How to run it

- Install and start RabbitMQ (the daemon is currently hardwired to localhost)
- Install Python 3, python-yaml and the pika library
- Install isolate
- Run `main.py`
