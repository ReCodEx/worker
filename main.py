#!/usr/bin/env python3

import pika
import sys
import os
import time

def callback(ch, method, properties, body):
    print(" [x] Received {}".format(body))
    time.sleep(10)
    print(" [x] Done")
    ch.basic_ack(delivery_tag = method.delivery_tag)

connection = pika.BlockingConnection(pika.ConnectionParameters(
    host = "localhost"
))
channel = connection.channel()

channel.exchange_declare(
    exchange = "tasks", 
    type = "direct", 
    durable = True,
    auto_delete = False
)

for env in sys.argv[1:]:
    channel.queue_declare(
        queue = env,
        durable = True,
        auto_delete = False
    )
    channel.queue_bind(
        exchange = "tasks", 
        queue = env
    )
    channel.basic_consume(callback, queue = env)

print(" [*] Waiting for messages. Press CTRL+C to exit.")

channel.basic_qos(prefetch_count = 1)

try:
    channel.start_consuming()
except KeyboardInterrupt:
    print("Interrupted")
