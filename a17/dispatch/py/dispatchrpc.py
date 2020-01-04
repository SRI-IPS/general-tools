#!/usr/bin/env python3

import argparse
from concurrent.futures import ThreadPoolExecutor
import logging
from pydoc import locate
import subprocess

import capnp

# Needed until dispatch and capnp_msgs python installs become fullpath
# or completely migrate to bazel
# TODO(murt): cleanup after dispatch and capnp_msgs installs become fullpath
try:
    import a17.dispatch.py.dispatch as dispatch
except ImportError:
    # TODO: Properly namespace the dispatch module. Should be a17.dispatch.
    import dispatch


def _capnp_object_from_string(input_str, import_path, schema_file, capnp_type_name, capnp_type):
    if import_path == None:
        import_path = ""
    short_type_name = capnp_type_name.split(".")[-1]
    # TODO(kgreenek): Find a way to do this in code, without running a subprocess.
    binary = subprocess.run(["capnp", "encode", "-I" + import_path, schema_file, short_type_name],
                            stdout=subprocess.PIPE, input=input_str.encode("utf-8")).stdout
    return capnp_type.from_bytes(binary)


def main():
    logging.getLogger().setLevel(logging.INFO)
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", help="name of the dispatch device to connect to")
    parser.add_argument("--topic", help="name of the dispatch rpc endpoint")
    parser.add_argument("-I", "--import-path", help="search path for capnp schema files")
    parser.add_argument("--request-type", help="full python type of the capnp request")
    parser.add_argument("--reply-type", help="full python type of the capnp reply")
    parser.add_argument("--schema-file",
                        help="capnp file where the request and reply schemas are defined")
    parser.add_argument("--timeout", type=float,
                        help="max time in seconds to wait for request to complete")
    parser.add_argument("request", help="string representing capnp message to send")
    args = parser.parse_args()

    request_type = locate(args.request_type)
    if request_type == None:
        print("ERROR: Invalid request-type")
        exit(1)
    reply_type = locate(args.reply_type)
    if reply_type == None:
        print("ERROR: Invalid reply-type")
        exit(1)
    request = _capnp_object_from_string(args.request, args.import_path, args.schema_file,
                                        args.request_type, request_type)

    thread_pool = ThreadPoolExecutor(1)
    node = dispatch.Node("DispatchRpc")
    node_future = thread_pool.submit(node.start)

    request_client = dispatch.RpcRequestClient(node, args.device + "/" + args.topic, request_type,
                                               reply_type)
    rpc_future = request_client.execute(request, args.timeout)
    reply = rpc_future.result()

    if reply != None:
        print("Received reply:")
        print(reply)
    else:
        print("RPC error")

    node.stop()
    node_future.result()
    thread_pool.shutdown()


if __name__ == "__main__":
    main()
