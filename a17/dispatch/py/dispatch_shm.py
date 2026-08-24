#!/usr/bin/env python3
"""dispatch_shm.py — Zero-copy shared memory pub/sub for the dispatch library.

Coordinates with the C++ ShmPublisher/ShmSubscriber via POSIX shared memory.
The SharedMemoryHandle Cap'n Proto message is serialized over ZeroMQ (matching
the C++ wire format exactly), while the actual payload stays in shared memory.

Requirements (already in requirements.txt):
    pycapnp, pyzmq, numpy

Python 3.8+ required for multiprocessing.shared_memory.
"""

import struct
import logging
import numpy as np

from multiprocessing.shared_memory import SharedMemory

import capnp  # noqa: F401  (required for pycapnp schema loading side-effects)

# Load the generated pycapnp schema.  pycapnp searches sys.path for .capnp
# files; the install step copies shm.capnp alongside this module.
import capnp
import os as _os
_schema_path = _os.path.join(_os.path.dirname(__file__), '..', '..', 'capnp_msgs', 'shm.capnp')
shm_capnp = capnp.load(_os.path.realpath(_schema_path))

logger = logging.getLogger(__name__)


# ---------------------------------------------------------------------------
# Wire helpers — must match the C++ SmartCapnpBuilder / SmartCapnpReader
# convention: [uint64 id frame, capnp bytes frame]
# ---------------------------------------------------------------------------

def _capnp_id(schema_type) -> int:
    """Returns the uint64 Cap'n Proto type id for a schema struct."""
    return schema_type.schema.node.id


def _build_wire_msg(capnp_obj) -> list[bytes]:
    """Serialise a capnp object into a two-frame ZMQ multipart message."""
    id_frame = struct.pack('<Q', _capnp_id(type(capnp_obj)))
    body_frame = capnp_obj.to_bytes()
    return [id_frame, body_frame]


def _parse_wire_msg(frames: list[bytes]):
    """Parse a two-frame ZMQ multipart message into (id, capnp_bytes)."""
    if len(frames) < 2:
        raise ValueError(f'Expected 2 ZMQ frames, got {len(frames)}')
    msg_id = struct.unpack('<Q', frames[0])[0]
    return msg_id, frames[1]


# ---------------------------------------------------------------------------
# ShmPublisher
# ---------------------------------------------------------------------------

class ShmPublisher:
    """Allocates a POSIX shared memory segment and publishes its handle over ZMQ.

    Usage::

        pub = ShmPublisher('/dispatch_cam_0', size=640*480*3)
        # Write payload directly into the numpy view (zero-copy).
        np.copyto(pub.array(np.uint8, (480, 640, 3)), frame)
        pub.publish(zmq_socket)   # sends SharedMemoryHandle over wire
        # Call pub.close() when done or use as a context manager.

    The shared memory segment is created with ``create=True`` and unlinked
    when ``close()`` is called, matching the C++ ``ShmPool`` owner semantics.
    """

    def __init__(self, name: str, size: int):
        """
        Args:
            name: POSIX shm name, e.g. '/dispatch_cam_0'. Must start with '/'.
            size: Payload size in bytes.
        """
        if not name.startswith('/'):
            raise ValueError(f"shm name must start with '/': {name!r}")
        self._name = name
        self._size = size
        # create=True allocates and owns the segment (unlinked on close).
        self._shm = SharedMemory(name=name.lstrip('/'), create=True, size=size)
        logger.debug('ShmPublisher: created segment %s (%d bytes)', name, size)

    # ------------------------------------------------------------------
    # Payload access
    # ------------------------------------------------------------------

    @property
    def buf(self) -> memoryview:
        """Zero-copy memoryview of the entire shared buffer."""
        return self._shm.buf

    def array(self, dtype, shape) -> np.ndarray:
        """Return a numpy array backed by the shared memory (zero-copy).

        Args:
            dtype: numpy dtype, e.g. np.uint8.
            shape: tuple, e.g. (480, 640, 3).
        """
        return np.ndarray(shape, dtype=dtype, buffer=self._shm.buf)

    # ------------------------------------------------------------------
    # Publishing
    # ------------------------------------------------------------------

    def publish(self, zmq_socket) -> None:
        """Send the SharedMemoryHandle over the given ZMQ socket (NOBLOCK).

        Args:
            zmq_socket: A ``zmq.Socket`` (ZMQ_PUB or any send-capable type).
        """
        handle = shm_capnp.SharedMemoryHandle.new_message()
        handle.name = self._name
        handle.size = self._size
        wire = _build_wire_msg(handle)
        zmq_socket.send_multipart(wire)
        logger.debug('ShmPublisher: published handle for %s', self._name)

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------

    def close(self) -> None:
        """Release and unlink the shared memory segment."""
        self._shm.close()
        self._shm.unlink()
        logger.debug('ShmPublisher: closed and unlinked %s', self._name)

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()


# ---------------------------------------------------------------------------
# ShmSubscriber
# ---------------------------------------------------------------------------

class ShmSubscriber:
    """Receives a SharedMemoryHandle from ZMQ and maps the shared memory.

    Usage::

        sub = ShmSubscriber()

        def on_frame(arr: np.ndarray):
            process(arr)          # arr is a zero-copy view; valid only here

        # Inside your ZMQ receive loop:
        frames = zmq_socket.recv_multipart()
        sub.receive(frames, callback=on_frame, dtype=np.uint8, shape=(480, 640, 3))

    The mapping is created, the callback is invoked, and the mapping is
    immediately released — matching the C++ ShmSubscriber stateless design.
    """

    def __init__(self):
        pass

    def receive(self, frames: list[bytes], callback, dtype=np.uint8,
                shape: tuple = None) -> None:
        """Deserialise the handle, map the segment, invoke callback, then unmap.

        Args:
            frames:   ZMQ multipart frames (list of bytes).
            callback: Called as ``callback(arr)`` where ``arr`` is a numpy
                      array backed by the shared memory. The array is only
                      valid for the duration of the callback.
            dtype:    numpy dtype for the returned array. Default: np.uint8.
            shape:    Optional reshape tuple. If None, a flat 1-D array of
                      ``size`` bytes is passed.
        """
        expected_id = _capnp_id(shm_capnp.SharedMemoryHandle)
        msg_id, body = _parse_wire_msg(frames)
        if msg_id != expected_id:
            raise ValueError(
                f'Expected SharedMemoryHandle id {expected_id:#x}, got {msg_id:#x}')

        handle = shm_capnp.SharedMemoryHandle.from_bytes(body)
        shm_name: str = handle.name   # e.g. '/dispatch_cam_0'
        shm_size: int = handle.size

        # Attach read-only (create=False). Strip the leading '/' that POSIX
        # requires but multiprocessing.SharedMemory does not expect.
        shm = SharedMemory(name=shm_name.lstrip('/'), create=False, size=shm_size)
        logger.debug('ShmSubscriber: mapped %s (%d bytes)', shm_name, shm_size)
        try:
            # Build a zero-copy numpy view directly into the shared buffer.
            arr = np.ndarray(
                shape if shape is not None else (shm_size,),
                dtype=dtype,
                buffer=shm.buf,
            )
            callback(arr)
        finally:
            # Always unmap — do NOT call unlink(); only the publisher owns it.
            shm.close()
            logger.debug('ShmSubscriber: released mapping for %s', shm_name)
