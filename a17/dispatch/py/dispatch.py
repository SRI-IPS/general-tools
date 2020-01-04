#!/usr/bin/env python3
import concurrent
import datetime
import functools
import logging
import os
import signal
import socket
import struct
import threading
import time
import uuid

from absl import flags
# TODO(kgreenek): Is this needed?
import capnp  # pylint: disable=unused-import
import netifaces
import zmq
from zmq.eventloop import ioloop, zmqstream


FLAGS = flags.FLAGS
flags.DEFINE_string("device_name", os.environ.get("A17_DEVICE_NAME"),
                    "Hardware-assigned device id. Defaults to A17_DEVICE_NAME env var")


# TODO(kgreenek): Non-const globals are not allowed. Do this differently.
zmq_context = zmq.Context()

# Keep global list of all nodes for signal handling.
_nodes = []


class Error(Exception):
    pass


class TimeoutError(Error):
    pass


class ConnectionError(Error):
    pass


def _signalHandler(s, f):
    for node in _nodes:
        node.stop()


# TODO(kgreenek): This should not be done here, because it overrides any existing logger
# configuration in the importing application.
logging.basicConfig(
    format='[%(asctime)s] [%(module)s](%(levelname)s) %(message)s', level=logging.INFO)


# This is a helper function to create a fully-formed dispatch topic from the given the device_name
# and topic.
def topic_with_device_name(device_name, topic):
    """Creates fully-formed topic name from device_name (string) and topic (string)"""
    if device_name:
        topic = device_name + "/" + topic
    return topic


def topic_str(device_name, node_name, topic):
    topic_str = ""
    if device_name:
        topic_str += device_name
    if node_name:
        if topic_str and not topic_str.endswith("/"):
            topic_str += "/"
        topic_str += node_name
    if topic:
        if topic_str and not topic_str.endswith("/"):
            topic_str += "/"
        topic_str += topic
    return topic_str


# Returns the device name provided via command-line flags. If not specified, returns the value of
# the A17_DEVICE_NAME environment variable.
# IMPORTANT: This will raise if you did not start your app using absl.app.run()!
def device_name():
    return FLAGS.device_name


def wire_msg_from_msg(msg):
    """Converts the given msg to wire format.

    Args:
        msg ([uint64_t, capnp_obj]): A 2-element list, where the first element is the capnp id of
            the message, and the second element is the capnp object.
    """
    wire_msg = []
    # Pack first element as uint64_t.
    wire_msg.append(struct.pack("Q", msg[0]))
    # Pack next element as bytes.
    wire_msg.append(msg[1].to_bytes())
    return wire_msg


class InterfaceInfo(object):
    """Determines the address of the local interface to use for sockets"""

    def __init__(self):
        logger = logging.getLogger(self.__class__.__name__)

        self.address = "127.0.0.1"
        self.broadcast = self.address
        self.netmask = "255.255.255.255"
        self.mask = struct.unpack('!I', socket.inet_aton(self.netmask))[0]
        self.network = struct.unpack('!I', socket.inet_aton(self.address))[0] & self.mask

        interface = os.environ.get('A17_DISPATCH_INTERFACE')

        if interface:
            if interface in netifaces.interfaces():
                for iface in netifaces.interfaces():
                    try:
                        ifaddrs = netifaces.ifaddresses(iface)
                    except ValueError:
                        continue

                    if iface == interface and socket.AF_INET in ifaddrs:
                        for addr in ifaddrs[socket.AF_INET]:
                            if 'broadcast' in addr:
                                self.address = addr['addr']
                                self.broadcast = addr['broadcast']
                                self.netmask = addr['netmask']
                                self.mask = struct.unpack('!I', socket.inet_aton(self.netmask))[0]
                                self.network = struct.unpack('!I', socket.inet_aton(self.address))[
                                    0] & self.mask
                                break
            else:
                logger.warning("Interface {} not found, defaulting to localhost".format(interface))

        else:
            logger.warning('A17_DISPATCH_INTERFACE not specified, defaulting to localhost')

        logger.info('Selected interface {} | ip {}'.format(interface, self.address))

    def on_network(self, address):
        """Checks if an address is on the same network as interface"""
        addr = struct.unpack('!I', socket.inet_aton(address))[0]
        return (addr & self.mask) == self.network


class SocketAddress(object):
    """Addressable identity of a network location"""

    def __init__(self, protocol="tcp", ip=None, port=0, interface=None):
        self.protocol = protocol
        self.ip = ip
        self.port = port

        if interface:
            self.ip = interface.address

    def from_string(self, address_str):
        protocol = "tcp"
        port = 0

        parts = address_str.split("://", 1)
        if len(parts) == 2:
            protocol, ip = parts
        else:
            ip = parts[0]

        ip_port_parts = ip.split(":", 1)
        if len(ip_port_parts) == 2:
            ip, port = ip_port_parts
            port = int(port)

        self.protocol = protocol
        self.ip = ip
        self.port = port

        return self

    def __str__(self):
        if self.port > 0:
            return '{}://{}:{}'.format(self.protocol, self.ip, self.port)
        else:
            return '{}://{}'.format(self.protocol, self.ip)


class Node(object):
    """Decentralized wrapper holding the io loop and the directory
    Args:
      name (string): The name of the node
    """

    def __init__(self, name):
        self.name = name
        self.logger = logging.getLogger(name)
        self.active = False
        self.io_loop = ioloop.IOLoop.instance()
        self.directory = Directory(self.io_loop, self.name)

        # ensure that we only call the signal handler from the main thread
        # if the node is started on a background thread, it is assumed that the main thread will
        # implement its own signal handling
        # TODO(pickledgator): maybe better to make this a param/flag instead
        if threading.current_thread().__class__.__name__ == '_MainThread':
            signal.signal(signal.SIGINT, _signalHandler)
            signal.signal(signal.SIGTERM, _signalHandler)
            signal.signal(signal.SIGHUP, _signalHandler)

        _nodes.append(self)

    def __del__(self):
        self.stop()

    def __repr__(self):
        return 'Node(' + self.name + ')'

    def run(self):
        if not self.active:
            self.active = True
            self.directory.run()
            self.logger.info("Node ({}) is starting".format(self.name))
            self.io_loop.start()
        else:
            self.logger.warn("Node already running")

    def topic(self, topic_name):
        """Returns the fully-qualified topic name (include device name and node name) for the given
            topic_name. E.g. "my_device/my_node/[topic_name]".

        Args:
            topic_name (str): The name of the topic.
        """
        return topic_str(device_name(), self.name, topic_name)

    def register_reply_server(self, topic, input_type, output_type, handler):
        return ReplyServer(self, topic, input_type, output_type, handler)

    def register_publisher(self, topic, message_type):
        return Publisher(self, topic, message_type)

    def register_subscriber(self, topic, message_type, handler, onConnect=None, onDisconnect=None):
        return Subscriber(self, topic, handler, onConnect, onDisconnect)

    def register_repeater(self, interval, callback, callback_arg=None, autostart=True):
        """Registers a repeater, where callback is called every interval ms."""
        return Repeater(self, interval, callback, callback_arg, autostart)

    def new_request_client(self, topic):
        return RequestClient(self, topic)

    def new_rpc_request_client(self, topic, request_type, reply_type):
        return RpcRequestClient(self, topic, request_type, reply_type)

    def start(self):
        # start ioloop on a background thread
        thread = threading.Thread(name=self.name, target=self.run)
        thread.setDaemon(True)
        thread.start()
        return thread

    def stop(self):
        # stop the node
        if self.active:
            self.active = False
            self.directory.stop()
            self.logger.info("Node ({}) is stopping".format(self.name))
            self.io_loop.stop()

    def scheduleOnce(self, delay, callback, *args):
        """Schedule a single callback on ioloop after delay
        Args:
          delay (int): Milliseconds in the future
          handler (function): Callback function to execute when timeout occurs
          args: Arguments that are passed to the callback function
        """
        def scheduleOnceCallback():
            self.io_loop.add_timeout(datetime.timedelta(milliseconds=delay), callback, *args)
        self.io_loop.add_callback(scheduleOnceCallback)


class Socket(object):
    """Base class for sockets, super to various types (eg, zmq.publisher, etc.)
    Args:
      node: Node instance containing the ioloop and directory
      socket_type:
    """

    def __init__(self, node, socket_type):
        self.node = node
        self.zsocket = zmq_context.socket(socket_type)

    def __del__(self):
        self.closeSocket()

    def closeSocket(self):
        self.zsocket.close()

    def send(self, msg):
        """Sends a serialized message over the socket
        Args:
          msg: Message object list
        """

        def _send(self, msg):
            self.zsocket.send_multipart(wire_msg_from_msg(msg), zmq.NOBLOCK)

        if len(msg) != 2:
            raise ValueError(
                "Message must contain 2 elements: {} elements sent".format(len(msg)))
        self.node.io_loop.add_callback(functools.partial(_send, self, msg))


class Server(Socket):
    """Server socket type
    Socket is bound to an address that is advertised with the directory
    Args:
      node: Node instance containing the ioloop and directory
      socket_type: zmq socket type for the server
      topic: String containing either the name or a fixed address
    """

    def __init__(self, node, socket_type, topic, input_type=None, output_type=None):
        Socket.__init__(self, node, socket_type)
        self.topic = topic
        self.logger = logging.getLogger('{}({})'.format(self.__class__.__name__, str(topic)))

        interface_address = SocketAddress(interface=InterfaceInfo())
        port = self.zsocket.bind_to_random_port(str(interface_address))
        addr = str(SocketAddress(interface_address.protocol, interface_address.ip, port))
        self.logger.info('{} [{}] @ {}'.format(
            node.directory.socket_type_strings[socket_type], topic, addr))

        # convert types to strings of type ids
        self.input_type = typeOf(input_type)
        self.output_type = typeOf(output_type)

        node.directory.add(Topic(topic, socket_type, addr, input_type,
                                 output_type, node.directory.my_guid, time.time()))

    def close(self):
        self.node.directory.remove(self.topic)
        Socket.closeSocket(self)


class Client(Socket):
    """Client socket type
    Socket connects to a bound server, with address found through the directory
    Args:
      node:
      socket_type:
      topic:
      on_connect (optional):
      on_discconect (optional):
    """

    def __init__(self, node, socket_type, topic, on_connect=None, on_disconnect=None):

        Socket.__init__(self, node, socket_type)
        self.logger = logging.getLogger('{0}({1})'.format(self.__class__.__name__, str(topic)))
        self.addresses = []
        self.on_connect = on_connect
        self.on_disconnect = on_disconnect

        self.directory = node.directory
        self.topic = topic
        self.connected_topic = None
        self.observe_topic(topic, self.topic_map_updated)

    def topic_map_updated(self, topic_map):
        new_addresses = [topic.address for topic in topic_map.values()]
        for new_address in new_addresses:
            if not new_address in self.addresses:
                self.connect(new_address, sync=True)
        for address in self.addresses:
            if not address in new_addresses:
                self.disconnect(address, sync=True)
        self.addresses = new_addresses

    def observe_topic(self, topic, found_topic_callback):
        def _observe_topic(self, topic, found_topic_callback):
            self.observer_ref = self.directory.observe(topic, found_topic_callback)
        self.node.io_loop.add_callback(functools.partial(_observe_topic, self, topic,
                                                         found_topic_callback))

    def unobserve_topic(self):
        def _unobserve_topic(self):
            if self.observer_ref:
                self.directory.unobserve(self.topic, self.observer_ref)
        self.node.io_loop.add_callback(functools.partial(_unobserve_topic, self))

    def close(self):
        self.unobserve_topic()
        Socket.closeSocket(self)

    def connect(self, address, sync=False):
        """Connect socket to the address"""

        def _connect():
            self.logger.debug("Connecting ({} @ {})".format(self.topic, address))
            self.zsocket.connect(address)
            if self.on_connect:
                # zmq takes a bit to actually establish the connection
                # TODO(pickledgator): can we guarantee this was enough time?
                self.node.scheduleOnce(100, functools.partial(self.on_connect, self))

        if sync:
            _connect()
        else:
            self.node.scheduleOnce(0, _connect)

    def disconnect(self, address, sync=False):
        """Disconnect socket from the address"""

        def _disconnect():
            self.logger.debug("Disconnecting ({} @ {})".format(self.topic, address))
            self.zsocket.disconnect(address)
            if self.on_disconnect:
                self.on_disconnect(address)

        if sync:
            _disconnect()
        else:
            self.node.scheduleOnce(0, _disconnect)

    def isConnected(self):
        """Returns whether the socket is connected"""
        return len(self.addresses) > 0


class Listener:
    """Waits for socket responses on the io loop
    Args:
      node:
      handler:
    """

    def __init__(self, node, handler):
        def on_receive(msg):
            self.logger.debug('Received {0}'.format(msg))
            handler(msg)

        self.stream = zmqstream.ZMQStream(self.zsocket, node.io_loop)
        self.stream.on_recv(on_receive)

    def close(self):
        if not self.stream.closed():
            self.stream.stop_on_recv()
            self.stream.close()


class Publisher(Server):
    """Publishes messages over a zmq socket as a server
    Use with Subscriber
    Args:
      node:
      topic:
      output_type:
    """

    def __init__(self, node, topic, output_type):
        Server.__init__(self, node, zmq.PUB, topic, None, output_type)


class Subscriber(Client, Listener):
    """Receives messages over a zmq socket connected as client to a server
    Use with Publisher
    Args:
      node:
      topic:
      handler:
      on_connect (optional):
      on_discconect (optional):
    """

    def __init__(self, node, topic, handler, on_connect=None, on_disconnect=None):
        Client.__init__(self, node, zmq.SUB, topic, on_connect, on_disconnect)
        Listener.__init__(self, node, handler)
        # Accept all messages at the zmq layer.
        # This change was affected by the move to uint64 id as msg[0] instead of a string since
        # zmq.SUBSCRIBE filter only accepts strings.
        self.zsocket.setsockopt(zmq.SUBSCRIBE, "".encode('UTF-8'))

    def close(self):
        Listener.close(self)
        Client.close(self)


class PubClient(Client):
    """Publishes message to a established server socket
    Use with SubServer
    Args:
      node:
      topic:
      on_connect (optional):
      on_discconect (optional):
    """

    def __init__(self, node, topic, on_connect=None, on_disconnect=None):
        Client.__init__(self, node, zmq.PUB, topic, on_connect, on_disconnect)


class SubServer(Server, Listener):
    """Receives messages over a zmq socket as a server
    Use with PubClient
    Args:
      node:
      topic:
      topic_type:
      handler:
    """

    def __init__(self, node, topic, input_type, handler):
        Server.__init__(self, node, zmq.SUB, topic, input_type, None)
        Listener.__init__(self, node, handler)

        # Accept all messages at the zmq layer
        # This change was affected by the move to uint64 id as msg[0] instead of a string since
        # zmq.SUBSCRIBE filter only accepts strings but we are packing a uint64
        self.zsocket.setsockopt(zmq.SUBSCRIBE, "".encode('UTF-8'))

    def close(self):
        Listener.close(self)
        Server.close(self)


class ReplyServer(Server, Listener):
    """Listens on a topic, and replies with a response sent from the handler
    Use with RequestClient
    Args:
      node:
      topic:
      input_type:
      output_type:
      reply_handler:
    """

    def __init__(self, node, topic, input_type, output_type, reply_handler):
        self._reply_handler = reply_handler
        Server.__init__(self, node, zmq.ROUTER, topic, input_type, output_type)
        Listener.__init__(self, node, self.handler)

    def handler(self, msg):
        def _send_wrapper(user_msg):
            wire_msg = []
            # The first frame from the original zmq message contains the router ID for the sender.
            # By setting this here, we cause the router to send the reply to the correct sender.
            wire_msg.append(msg[0])
            # This is just an empty frame. It is required by the router to separate the sender ID
            # from the actual message frames.
            wire_msg.append(msg[1])
            wire_msg.extend(wire_msg_from_msg(user_msg))
            self.router_send(wire_msg)

        # Ignore the first 2 frames. They are internal ROUTER frames.
        self._reply_handler(msg[2:], _send_wrapper)

    def router_send(self, msg):
        """Sends a serialized message over the socket. msg is expected to contain the additional
            router header info.

        Args:
          msg: Message object list with router header.
        """

        def _send(self, msg):
            self.zsocket.send_multipart(msg, zmq.NOBLOCK)

        if len(msg) != 4:
            raise ValueError(
                "Message must contain 4 elements: {} elements sent".format(len(msg)))
        self.node.io_loop.add_callback(functools.partial(_send, self, msg))


class RequestClient(object):

    def __init__(self, node, topic, on_connect=None, on_disconnect=None):
        self.logger = logging.getLogger('{0}({1})'.format(self.__class__.__name__, str(topic)))
        self.node = node
        self.topic = topic
        self.on_connect = on_connect
        self.on_disconnect = on_disconnect

        self.stream = None
        self.connected_topic = None
        self.directory = node.directory
        self.observe_topic(topic, self.topic_map_updated)

    def send(self, msg):
        """Sends a serialized message over the socket
        Args:
          msg: Message object list
        """

        def _send():
            assert(self.stream)
            self.stream.send_multipart(wire_msg_from_msg(msg), zmq.NOBLOCK)

        if len(msg) != 2:
            raise ValueError(
                "Message must contain 2 elements: {} elements sent".format(len(msg)))
        self.node.io_loop.add_callback(_send)

    def observe_topic(self, topic, found_topic_callback):
        def _observe_topic(self, topic, found_topic_callback):
            self.observer_ref = self.directory.observe(topic, found_topic_callback)
        self.node.io_loop.add_callback(functools.partial(_observe_topic, self, topic,
                                                         found_topic_callback))

    def unobserve_topic(self):
        def _unobserve_topic(self):
            if self.observer_ref:
                self.directory.unobserve(self.topic, self.observer_ref)
        self.node.io_loop.add_callback(functools.partial(_unobserve_topic, self))

    def topic_map_updated(self, topic_map):
        # NOTE: This is always called on the dispatch thread.
        newest_topic = _newest_topic_in_topic_map(topic_map)
        if not newest_topic:
            self.disconnect()
            return
        # TODO(kgreenek): Handle this in a nicer way in the Directory protocol itself.
        # If a server has gone down and then come back up (possibly multiple times while this node
        # has been running), it will contain old and stale addresses. The work-around here is to
        # check the timestamp and always use the newest Topic. That way if the server goes down and
        # is restarted, this client will connect to the new server gracefully and the stale entries
        # are ignored.
        if not self.connected_topic or (newest_topic.address != self.connected_topic.address and
                                        newest_topic.timestamp > self.connected_topic.timestamp):
            self.connect(newest_topic, sync=True)

    def connect(self, topic, sync=False):

        def _connect():
            self.disconnect(sync=True)
            self.connected_topic = topic
            self.logger.debug("Connecting ({} @ {})".format(topic.name, topic.address))
            socket = zmq_context.socket(zmq.REQ)
            # This configures the socket to not attempt to re-send failed messages after the
            # socket is closed.
            socket.setsockopt(zmq.LINGER, 0)
            socket.connect(topic.address)
            # NOTE: Stream takes ownership of the socket object. It closes the underlying socket
            # when the stream is closed and keeps a reference.
            self.stream = zmqstream.ZMQStream(socket, self.node.io_loop)
            if self.on_connect:
                # zmq takes a bit to actually establish the connection
                # TODO(pickledgator): can we gauruntee this was enough time?
                self.node.scheduleOnce(100, functools.partial(self.on_connect, self))

        if sync:
            _connect()
        else:
            self.node.scheduleOnce(0, _connect)

    def disconnect(self, sync=False):
        """Disconnect socket from the address"""

        def _disconnect():
            if self.connected_topic:
                self.logger.debug("Disconnecting ({} @ {})".format(
                    self.connected_topic.name, self.connected_topic.address))
                self.connected_topic = None
                self.close_stream()

        if sync:
            _disconnect()
        else:
            self.node.io_loop.add_callback(_disconnect)

    def isConnected(self):
        """Returns whether the socket is connected"""
        return self.stream is not None

    def request(self, msg, handler):
        """Application layer accessible to send a message over the socket to the service
        Establishes a zmqstream to wait for the reply from the service
        """
        assert(self.stream)
        self.stream.on_recv(handler)
        self.send(msg)

    def request_blocking(self, msg, timeout):
        """Sends the given request and blocks until a reply is received. Returns the reply."""
        reply_condition = threading.Condition()
        reply = None
        exited = False

        def reply_callback(_reply):
            nonlocal reply_condition
            nonlocal reply
            nonlocal exited
            try:
                reply_condition.acquire()
                if not exited:
                    # This will happen if we receive a reply after the original blocking call has
                    # already returned.
                    reply = _reply
                reply_condition.notify_all()
            finally:
                reply_condition.release()

        try:
            reply_condition.acquire()
            self.request(msg, reply_callback)
            reply_condition.wait(timeout)
            exited = True
            if not reply:
                raise TimeoutError("Request timed out")
            return reply
        finally:
            reply_condition.release()

    def close_stream(self):
        if self.stream:
            self.stream.stop_on_recv()
            self.stream.close()
            self.stream = None
        if self.on_disconnect:
            self.on_disconnect(self)

    def close(self):
        self.unobserve_topic()
        self.close_stream()


class RpcRequestClient:
    """Smarter request client
    Use with ReplyServer that implements standard RPC interface
    Args:
      node: the node to use for issuing the request. must be run by the calling code
      topic: topic name string
      request_type: type of the capnp request object
      reply_type: expected capnp type for the reply object
    """

    def __init__(self, node, topic, request_type, reply_type):
        self._node = node
        self._topic = topic
        self._request_type = request_type
        self._reply_type = reply_type
        self._future = None
        self._timeout_scheduled = False
        self._timeout_handle = None

    def execute(self, request, timeout=0):
        """Executes the request client. This can only be done once.

        Args:
            request (request_type): The request to send.
            timeout (float): Timeout in seconds to wait for a reply.
        """
        if self._future != None:
            raise AssertionError("RpcRequestClient.execute() called more than once")
        if timeout:
            self._schedule_timeout(int(timeout * 1000), self._on_timeout)
        self._future = concurrent.futures.Future()
        self._future.set_running_or_notify_cancel()
        self._request = request
        self._request_client = RequestClient(
            self._node, self._topic, self._on_connect, self._on_disconnect)
        return self._future

    def _schedule_timeout(self, timeout_ms, callback):

        def __schedule_timeout():
            if not self._timeout_scheduled:
                return
            self._timeout_handle = self._node.io_loop.add_timeout(
                datetime.timedelta(milliseconds=timeout_ms), callback)

        self._timeout_scheduled = True
        self._node.io_loop.add_callback(__schedule_timeout)

    def _cancel_timeout(self):
        self._timeout_scheduled = False
        if self._timeout_handle:
            self._node.io_loop.remove_timeout(self._timeout_handle)

    def _on_reply(self, reply_message):
        self._cleanup()
        try:
            # NOTE: The first "part" of the message is the id. Ignore it.
            self._future.set_result(self._reply_type.from_bytes(reply_message[1]))
        except Exception as e:
            self._node.logger.info("Exception in RpcRequestClient.on_reply", exc_info=True)

    def _on_connect(self, arg):
        if not self._request_client:
            return
        request_message, request_builder = newMessage(self._request_type)
        request_builder.from_dict(self._request.to_dict())
        self._request_client.request(request_message, self._on_reply)

    def _on_disconnect(self, arg):
        if not self._request_client:
            return
        self._cleanup()
        if not self._future.done():
            self._future.set_exception(ConnectionError("Connection error"))

    def _on_timeout(self):
        # This shouldn't be possible. This would happen if _on_reply was somehow called before
        # this was called, but didn't properly cancel the timeout.
        assert(self._request_client)
        self._cleanup()
        if not self._future.done():
            self._future.set_exception(TimeoutError("Timed out"))

    def _cleanup(self):
        # Set self._request_client to None before calling close, because it can trigger a call to
        # _on_disconnect, which calls cleanup... So that would be an infinite recursion loop.
        request_client = self._request_client
        self._request_client = None
        self._cancel_timeout()
        request_client.close()


class Repeater:
    """Repeats a callback on a fixed interval
    Args:
      node:
      interval: number milliseconds to wait between callbacks
      callback: function to execute
      callback_arg [None]: parameters passed to the callback function
      autostart [true]: flag to automatically start the timer at init
    """

    def __init__(self, node, interval, callback, callback_arg=None, autostart=True):
        def operation():
            self.callback(self.callback_arg)

        self.callback = callback
        self.callback_arg = callback_arg
        self.timer = ioloop.PeriodicCallback(operation, interval, node.io_loop)
        if autostart:
            self.timer.start()

    def start(self):
        self.timer.start()

    def stop(self):
        self.timer.stop()


def idOf(cls):
    # return uint64_t capnproto type id for the given type
    try:
        idval = cls._nodeSchema.get_proto().id
    except AttributeError:
        return cls
    return idval


def typeOf(cls):
    # return type string for the capnp type
    return str(idOf(cls))


def isTypeOf(msg, cls):
    # returns whether the message id matches the provided capnp type
    if not msg:
        return False
    return idFromSmartMessage(msg) == idOf(cls)


def idFromSmartMessage(msg):
    val = None
    try:
        val = struct.unpack('Q', msg[0])[0]
    except:
        raise ValueError("Could not unpack id bytes from message")
    return val


def newMessage(cls):
    """Initialize a new multipart message list from a capnproto type
    Populates the first array element with the correct type info,
    and the remaining elements with initialized capnproto objects.

    Returns a tuple where the first item is the whole message array,
    and the remaining items are references to the capnproto objects in the array.
    This makes it easier to populate the capnp template and to send the msg vec

    Example usage:
    msg_vec, capnp_X = dispatch.newMessage(X)
    capnp_X.apple = ...
    capnp_X.orange = ...
    socket.send(msg_vec)

    Args:
      cls: Capnp class of new message type
    """
    msg_vec = [idOf(cls)]
    msg_vec.append(cls.new_message())
    return (msg_vec, msg_vec[1])


def messageFromCapnp(obj, cls):
    """Returns a azmq message vector with the given capnp object
    Args:
      obj: The capnp object itself
      cls: The class of the capnp object
    """
    msg_vec = [idOf(cls)]
    msg_vec.append(obj)
    return msg_vec


def parse(msg, cls):
    """Parse a multipart message into its capnp type
    Args:
      msg: The multipart serialized message off the wire
      cls: The expected capnp type to parse from the data
    """
    if not isTypeOf(msg, cls):
        raise ValueError('Message id {} does not match requested type {}'.format(
            idFromSmartMessage(msg), typeOf(cls)))

    capnp = None
    try:
        capnp = cls.from_bytes(msg[1])
    except:
        raise ValueError(
            "Serialized msg content could not be deserialized into type: {}".format(typeOf(cls)))

    return capnp


class Topic:
    """Object containing information about an individual topic
    Args:
      topic_name (str): String representing the topic (e.g. HH1/PAX6/POSE).
      socket_type (zmq.SOCKET_TYPE): Type of the zmq socket (e.g. zmq.ROUTER).
      address (str): IP address and port of the host.
      input_type (str): Unique id of the input capnp message type.
      output_type (str): Unique id of the output capnp message type.
      guid (str): Unique string representing the remote host and topic.
      timestamp (int): Unix timestamp when the topic was created.
    """

    def __init__(self, topic_name, socket_type, address, input_type, output_type, guid, timestamp):
        self.name = topic_name
        self.socket_type = socket_type
        self.address = address
        self.input_type = input_type
        self.output_type = output_type
        self.guid = guid
        self.timestamp = timestamp


class TopicMap:
    """Data store to maps topics to addresses"""

    def __init__(self, my_guid):
        self.my_guid = my_guid
        self.local_topics = {}
        self.network_topics = {}
        self.observers = {}

    def add(self, topic):
        same_guid = topic.guid == self.my_guid
        if same_guid:
            if topic.name in self.local_topics:
                raise ValueError('directory {0} already contains topic {1}'.format(
                    self.my_guid, topic.name))

            self.local_topics[topic.name] = topic

        if topic.name in self.network_topics:
            guid_to_topic = self.network_topics[topic.name]
            if topic.guid not in guid_to_topic:
                guid_to_topic[topic.guid] = topic
                self.call_observers(topic.name, guid_to_topic)
            elif guid_to_topic[topic.guid].address != topic.address:
                guid_to_topic[topic.guid] = topic
                self.call_observers(topic.name, guid_to_topic)
        else:
            guid_to_topic = {topic.guid: topic}
            self.network_topics[topic.name] = guid_to_topic
            self.call_observers(topic.name, guid_to_topic)

        return same_guid

    def remove(self, guid, topic_name):
        removed_local = False
        if guid == self.my_guid and topic_name in self.local_topics:
            del self.local_topics[topic_name]
            removed_local = True

        if topic_name in self.network_topics:
            topic_to_guid = self.network_topics[topic_name]
            if guid in topic_to_guid:
                del topic_to_guid[guid]
                self.call_observers(topic_name, topic_to_guid)

            if not topic_to_guid:
                del self.network_topics[topic_name]

        return removed_local

    def remove_all(self, guid):
        for name in list(self.network_topics.keys()):
            topic_to_guid = self.network_topics[name]
            if guid in topic_to_guid:
                topic = topic_to_guid[guid]
                del topic_to_guid[guid]
                if not len(topic_to_guid):
                    del self.network_topics[name]
                self.call_observers(topic.name, topic_to_guid)

    def observe(self, topic_name, handler):
        if topic_name not in self.observers:
            self.observers[topic_name] = {}

        uuid_ref = str(uuid.uuid1())
        self.observers[topic_name][uuid_ref] = handler

        if topic_name in self.network_topics:
            handler(self.network_topics[topic_name])

        return uuid_ref

    def call_observers(self, topic_name, guid_to_topic):
        if topic_name in self.observers:
            for handler in self.observers[topic_name].values():
                handler(guid_to_topic)

    def unobserve(self, topic_name, uuid_ref):
        if topic_name in self.observers:
            if uuid_ref in self.observers[topic_name]:
                del self.observers[topic_name][uuid_ref]

    def missing(self):
        topics = []

        for topic_name in self.observers.keys():
            if topic_name not in self.network_topics or len(self.network_topics[topic_name]) == 0:
                topics.append(topic_name)

        return topics

    def __len__(self):
        return len(self.network_topics)


class Directory:

    def __init__(self, io_loop, name, port=int(os.getenv('DISPATCH_PORT', '8888')),
                 address=os.getenv('DISPATCH_MULTICAST', '224.0.88.1')):
        self.io_loop = io_loop
        self.multicast_endpoint = (address, port)
        self.interface_info = InterfaceInfo()
        self.my_address = self.interface_info.address
        self.my_guid = name + "_" + self.my_address + "_" + str(uuid.uuid1())
        self.logger = logging.getLogger(self.my_guid)

        self.topics = TopicMap(self.my_guid)
        # re-broadcast timer, incase of non-connected topics
        self.broadcast_timer = None
        # starting interval for re-broadcasts
        self.broadcast_interval = 1
        # max interval for re-broadcasts
        self.broadcast_interval_max = 16
        # max number of topic names to pack into a single discovery broadcast
        self.broadcast_grouping_max = 4
        self.logger.info('Directory running on {}:{}'.format(
            self.multicast_endpoint[0], self.multicast_endpoint[1]))

        self.mc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.mc_socket.setblocking(False)

        self.mc_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.mc_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        except AttributeError:
            pass

        mc_ip = struct.pack("=4sl", socket.inet_aton(self.multicast_endpoint[0]), socket.INADDR_ANY)
        self.mc_socket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mc_ip)
        self.mc_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
        self.mc_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
        self.mc_socket.bind(('', self.multicast_endpoint[1]))

        self.socket_type_strings = {zmq.PUB: 'PUB', zmq.SUB: 'SUB',
                                    zmq.REQ: 'REQ', zmq.REP: 'REP', zmq.ROUTER: 'ROUTER'}
        self.socket_type_values = {v: k for k, v in self.socket_type_strings.items()}

    def observe(self, topic_name, callback):
        ref = self.topics.observe(topic_name, callback)
        self.query_poll()
        return ref

    def unobserve(self, topic_name, ref):
        self.topics.unobserve(topic_name, ref)

    def add(self, topic):
        if self.topics.add(topic):
            self.logger.debug('Registered {} [{}] @ {}'.format(
                self.socket_type_strings[topic.socket_type], topic.name, topic.address))
            self.send('A', self.build_broadcast_string(topic))
        else:
            self.logger.debug('Added {} [{}] @ {} from {}'.format(self.socket_type_strings[
                              topic.socket_type], topic.name, topic.address, topic.guid))

    def remove(self, topic_name):
        if self.topics.remove(self.my_guid, topic_name):
            self.logger.info('Removed topic {0}'.format(topic_name))
            self.send('R', topic_name)

    def run(self):
        def _add_handler(self):
            self.io_loop.add_handler(self.mc_socket.fileno(), self.onReceive, self.io_loop.READ)
        self.io_loop.add_callback(functools.partial(_add_handler, self))

    def stop(self):
        def _stop(self):
            self.io_loop.remove_handler(self.mc_socket.fileno())
            self.send('X')
        self.io_loop.add_callback(functools.partial(_stop, self))

    def onReceive(self, fd, events):
        (line, source) = self.mc_socket.recvfrom(4096)
        try:
            line = line.decode('UTF-8')
        except:
            self.logger.debug('Ignoring unrecognized data: {}'.format(line))
            return

        parts = line.split(' ', 3)
        # ensure that we got a udp packet with our tag
        if len(parts) < 3 or parts[0] != 'DISPATCH':
            self.logger.debug('Ignoring unrecognized data: {}'.format(line))
            return

        guid = parts[1]
        # ignore loopbacks
        if guid == self.my_guid:
            return

        self.logger.debug('Received {}'.format(line))

        event = parts[2]
        if event == 'X':
            self.discovery_exiting(guid)
        elif len(parts) > 3 and event == 'A':
            self.discovery_available(guid, parts[3])
        elif len(parts) > 3 and event == 'R':
            self.discovery_remove(guid, parts[3])
        elif len(parts) > 3 and event == 'S':
            self.discovery_search(guid, parts[3])
        else:
            self.logger.debug('Unrecognized event: {}'.format(event))

    def discovery_exiting(self, guid):
        self.logger.debug("Removing all topics from guid: {}".format(guid))
        self.topics.remove_all(guid)
        self.query_poll()

    def discovery_available(self, guid, args):
        parts = args.split(' ')
        if len(parts) < 5:
            self.logger.debug('Ignoring invalid topic: {}'.format(args))
            return

        name = parts[0]
        socket_type = parts[1]
        addr = parts[2]

        address = SocketAddress().from_string(addr)
        if not self.interface_info.on_network(address.ip):
            self.logger.debug('Ignoring topic {} from other network {}'.format(name, addr))
            return

        in_type = None if parts[3] == "." else parts[3]
        out_type = None if parts[4] == "." else parts[4]

        topic = Topic(name, self.socket_type_values[socket_type], addr, in_type, out_type, guid,
                      time.time())
        self.add(topic)

    def discovery_remove(self, guid, name):
        self.topics.remove(guid, name)
        self.query_poll()

    def discovery_search(self, guid, namestr):
        names = namestr.split()
        for name in names:
            # res = name in self.topics.local_topics
            if name in self.topics.local_topics:
                self.send('A', self.build_broadcast_string(self.topics.local_topics[name]))

    def send(self, command, args=''):
        def _send(self, command):
            line = '{} {} {}'.format('DISPATCH', self.my_guid, command)
            if args:
                line += ' ' + args
            self.logger.debug('Sending {}'.format(line))
            self.mc_socket.sendto(line.encode('UTF-8'), self.multicast_endpoint)
        self.io_loop.add_callback(functools.partial(_send, self, command))

    def build_broadcast_string(self, topic):
        in_type = typeOf(topic.input_type) if topic.input_type else "."
        out_type = typeOf(topic.output_type) if topic.output_type else "."
        brdcst_str = '{} {} {} {} {}'.format(topic.name, self.socket_type_strings[
                                             topic.socket_type], topic.address, in_type, out_type)
        return brdcst_str

    def find_missing_topics(self):
        missing_topics = self.topics.missing()
        if not missing_topics:
            self.broadcast_timer = None
        else:
            buf = ''
            count = 0

            for topic_name in missing_topics:
                if count >= self.broadcast_grouping_max or len(buf) + 1 + len(topic_name) > 1400:
                    self.send('S', buf)
                    buf = ''
                    count = 0

                if buf:
                    buf += ' '
                buf += topic_name
                count += 1

            if buf:
                self.send('S', buf)

            self.broadcast_timer = self.io_loop.call_later(
                self.broadcast_interval, self.find_missing_topics)
            if self.broadcast_interval < self.broadcast_interval_max:
                self.broadcast_interval *= 2

    def query_poll(self):

        def _start_polling(self):
            if self.broadcast_timer:
                self.io_loop.remove_timeout(self.broadcast_timer)
            self.broadcast_interval = 0.1
            self.find_missing_topics()

        self.io_loop.add_callback(functools.partial(_start_polling, self))


def _newest_topic_in_topic_map(topic_map):
    if not topic_map or not topic_map.values():
        return None
    newest_topic = list(topic_map.values())[0]
    for topic in topic_map.values():
        if topic.timestamp > newest_topic.timestamp:
            newest_topic = topic
    return newest_topic
