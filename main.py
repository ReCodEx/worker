#!/usr/bin/env python3

import pika
import sys
import os
import time
import json
import yaml
import subprocess

class Environment:
    def __init__(self, data):
        self.data = data

    def get_source_name(self):
        return self.data["source_name"]

    def __getitem__(self, key):
        return self.data[key]

    def __contains__(self, key):
        return key in self.data

    @classmethod
    def from_yaml_file(self, path):
        with open(path) as env_file:
            data = yaml.load(env_file)
            return Environment(data)

def isolate(*args):
    return subprocess.call(
        ["isolate"] + list(args),
        stdout = subprocess.DEVNULL,
        stderr = subprocess.DEVNULL
    )

def callback(ch, method, properties, body):
    print(" [x] Received a message")
    message = json.loads(body.decode())
    environment = Environment.from_yaml_file("env/{}.yml".format(message["environment"]))

    isolate("--cg", "--init")
    source_path = "/tmp/box/0/box/{}".format(environment.get_source_name())
    
    with open(source_path, "w") as source_file:
        source_file.write(message["source"])

    if "build" in environment:
        isolate("--run", "-e", "-p", "--", *environment["build"].split())
    if "run" in environment:
        isolate("--run", "-o", "data.out", "-e", "--", *environment["run"].split())
        with open("/tmp/box/0/box/data.out", "r") as output_file:
            if output_file.read().strip() == "hello":
                print("OK")
            else:
                print("Not OK")

    isolate("--cleanup")

    print(" [x] Done")
    ch.basic_ack(delivery_tag = method.delivery_tag)

if __name__ == "__main__":
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
