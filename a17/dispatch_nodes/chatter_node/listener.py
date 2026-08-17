#!/usr/bin/env python3
"""Simple listener node that receives and prints Chatter messages.

Usage:
    # In one terminal:
    python talker.py

    # In another terminal:
    python listener.py
"""
from absl import app
from absl import flags
from absl import logging
import capnp

from a17.capnp_msgs.dispatch_nodes import chatter_capnp
from a17.dispatch.py import dispatch

FLAGS = flags.FLAGS
flags.DEFINE_string("node_name", "LISTENER", "Node name for this listener")


def main(argv):
    node = dispatch.Node(FLAGS.node_name)
    topic = node.topic("CHATTER")

    def on_message(msg):
        chatter = dispatch.parse(msg, chatter_capnp.Chatter)
        logging.info("[listener] received: sender=%s message=%s timestamp=%d",
                     chatter.sender, chatter.message, chatter.timestamp)

    sub = node.register_subscriber(topic, chatter_capnp.Chatter, on_message)

    logging.info("[%s] listening on topic: %s", FLAGS.node_name, topic)
    try:
        node.run()
    except (KeyboardInterrupt, SystemExit):
        pass


if __name__ == "__main__":
    app.run(main)
