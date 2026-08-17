#!/usr/bin/env python3
"""Simple talker node that publishes Chatter messages at 10 Hz.

Usage:
    # In one terminal:
    python talker.py

    # In another terminal:
    python listener.py
"""
import time

from absl import app
from absl import flags
from absl import logging
import capnp

from a17.capnp_msgs.dispatch_nodes import chatter_capnp
from a17.dispatch.py import dispatch

FLAGS = flags.FLAGS
flags.DEFINE_string("node_name", "TALKER", "Node name for this talker")


def main(argv):
    node = dispatch.Node(FLAGS.node_name)
    topic = node.topic("CHATTER")
    pub = node.register_publisher(topic, chatter_capnp.Chatter)

    count = [0]

    def publish(arg):
        count[0] += 1
        msg_vec, msg = dispatch.newMessage(chatter_capnp.Chatter)
        msg.timestamp = int(time.time() * 1e6)
        msg.sender = FLAGS.node_name
        msg.message = "Hello world #{}".format(count[0])
        pub.send(msg_vec)
        logging.info("[%s] published message #%d", FLAGS.node_name, count[0])

    node.register_repeater(100, publish)

    logging.info("[%s] publishing on topic: %s", FLAGS.node_name, topic)
    try:
        node.run()
    except (KeyboardInterrupt, SystemExit):
        pass


if __name__ == "__main__":
    app.run(main)
