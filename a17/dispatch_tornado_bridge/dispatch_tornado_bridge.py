import datetime
import json
import logging

import tornado.escape
import tornado.gen
import tornado.web
import tornado.websocket

from a17.dispatch.py import dispatch


class DispatchSubscriberWebSocketHandler(tornado.websocket.WebSocketHandler):
    """Subscribes to a dispatch publisher, and republishes messages over a websocket as json."""

    def initialize(self, node, topic, capnp_type):
        """Called when the handler is created.

        Args:
            node (dispatch.Node): Dispatch node for communicating with internal dispatch services.
            topic (string): The dispatch topic to subscribe to, and republish over the websocket.
            capnp_type (type): The type of the capnp dispatch message for the subscriber.
        """
        self._logger = logging.getLogger()
        self._node = node
        self._topic = topic
        self._capnp_type = capnp_type

    def open(self):
        self._logger.info("WebSocket opened for dispatch topic (%s)", self._topic)
        self._dispatch_subscriber = self._node.register_subscriber(
            self._topic, self._capnp_type, self._on_dispatch_message)

    def on_close(self):
        self._logger.info("WebSocket closed for dispatch topic (%s)", self._topic)
        self._dispatch_subscriber.close()
        self._dispatch_subscriber = None

    def handle_dispatch_message(self, message):
        """Subclasses can override this to perform logic whenever a message is received.
            If message is modified, then the modified data will be forwarded via json.

        Args:
            message (capnp_type): The parsed capnp message, of the type that this class was
                initialized with.

        Returns:
            The message to be forwarded as json, or None if it should be dropped.
        """
        return message

    def _on_dispatch_message(self, message):
        if not self._dispatch_subscriber:
            self._logger.warn("Received message after WebSocket closed (%s)", self._topic)
            return
        try:
            capnp_obj = self.handle_dispatch_message(dispatch.parse(message, self._capnp_type))
            if capnp_obj is not None:
                json_obj = json.dumps(capnp_obj.to_dict())
                self._logger.debug("Received dispatch message: %s", json_obj)
                self.write_message(json_obj)
        except:
            self._logger.warn("Exception in _on_dispatch_message (%s)", self._topic, exc_info=True)


class DispatchRequestHandler(tornado.web.RequestHandler):
    """Converts an incoming request from json into the specified request_capnp_type, sends it to the
        specified topic, and converts the reply into the specified reply_capnp_type.
    """

    def initialize(self, node, topic, request_capnp_type, reply_capnp_type, timeout):
        """Called when the handler is created.

        Args:
            node (dispatch.Node): Dispatch node for communicating with internal dispatch services.
            topic (string): The dispatch topic to forward requests to.
            request_capnp_type (type): The type of the capnp dispatch request message.
            reply_capnp_type (type): The type of the capnp dispatch reply message.
            timeout (float): Timeout in seconds to wait for internal dispatch request.
        """
        self._logger = logging.getLogger()
        self._node = node
        self._topic = topic
        self._request_capnp_type = request_capnp_type
        self._reply_capnp_type = reply_capnp_type
        self._timeout = timeout

    @tornado.gen.coroutine
    def post(self):
        self._logger.debug("Request: %s", self.request.body)
        request = None
        try:
            request_json = tornado.escape.json_decode(self.request.body)
            request = self._request_capnp_type.new_message(**request_json)
        except Exception:
            self._logger.info("Error converting json request to request_capnp_type")
            raise
        request_client = dispatch.RpcRequestClient(
            self._node, self._topic, self._request_capnp_type, self._reply_capnp_type)
        request_future = request_client.execute(request)
        reply = None
        try:
            # This is a workaround for a bug in tornado, where it doesn't properly handle a yielded
            # concurrent.futures.Future with python >3.2. The server will hang until the future
            # times out if we simply yield request_future.result() here.
            # See: https://github.com/tornadoweb/tornado/issues/1595
            total_timeout = 1
            while not request_future.done():
                try:
                    yield tornado.gen.with_timeout(datetime.timedelta(seconds=1), request_future)
                except tornado.gen.TimeoutError:
                    if total_timeout > self._timeout:
                        raise TimeoutError()
                total_timeout += 1
            reply = request_future.result()
        except TimeoutError:
            self._logger.info("Dispatch request timed out (%s)\"}", self._topic)
            raise
        self.write(json.dumps(reply.to_dict()))
