# TODO_BUGFIX

Backlog of bug theories recorded with the evidence available at the time. Revisit each item
when more evidence is available (e.g. from working dispatch node examples) before making changes.

---

## C++ listener crash when the publisher node is killed

**Status:** Minimal fix applied (2026-10-08): `Client::disconnect` in a17/dispatch/client.cpp now
uses the azmq non-throwing overload and logs failures instead of aborting. Scope was trimmed after
re-analysis -- only this function is implicated by the crash log.

### Symptoms

Running the C++ `dispatch_talker` and `dispatch_listener` (a17 dispatch nodes), killing the talker
while the listener is running crashes the listener:

```
[listener] received: sender=TALKER message=Hello world #171 timestamp=84097422791
[2026-10-08 05:29:02.639] [Socket](info) Subscriber [TALKER/CHATTER] !@ tcp://127.0.0.1:35067
terminate called after throwing an instance of 'boost::system::system_error'
  what():  Invalid argument
Aborted (core dumped)
```

The `!@ <address>` log is from `Client::disconnect()` (a17/dispatch/client.cpp).
The listener catches the topic via the directory (`DISCOVERY_EXIT`); the directory exit triggers
`DirectoryTopicStore::evict(guid)` -> `callObservers` -> `Client::onDirectoryTopicsChanged` ->
`Client::disconnect()`, all running on the listener's io_service handler.

### Working theory

The azmq (v1.0.3) throwing overloads wrap libzmq `zmq_disconnect`, which returned `-1`/`EINVAL`
("Invalid argument") at the moment the publisher's TCP pipe was torn down. The resulting
`boost::system::system_error` escapes the io_service handler and propagates through
`ios_.run()` -> `Node::run()` -> `std::terminate` (core dump).

Open questions:

- Exactly why `zmq_disconnect` returns EINVAL here is unconfirmed. The subscriber was successfully
  receiving (so `zmq_connect` previously succeeded and the address should be in the socket's
  endpoint set). Possibly a libzmq 4.2.2 behavior when the peer pipe is torn down/reconnecting.
- Whether this reproduces in-process (both nodes in one test binary) is unconfirmed; CI can only
  exercise the in-process path.
- Existing dispatch nodes reportedly do not hit this. Need to review working dispatch node examples
  for how they observe/disconnect (address-less direct connect vs directory-based `Client`) and
  whether publisher shutdown ever occurs in those topologies.

### Evidence (2026-10-08, REFERENCE_CODE/estop)

The estop reference node is a production dispatch node that uses the SAME machinery as the
listener: multiple `registerCapnpSubscriber`s on cross-node topics plus a
`RequestClient(node->service(), node->directory(), topic)`. Both go through
`Client::connect/disconnect` and `RequestClient::connect/disconnect` -- the crash sites above. It
does not crash in production, plausibly because:

- Watched publishers (PAX6 estimator, AGV_BRIDGE) are always-on and never torn down while being
  observed, so the latent dead-publisher disconnect path is rarely exercised.
- AGV nodes likely run under a supervisor that restarts crashed nodes, masking any remaining crash.

Precedent validating the planned fix style: `emergency_stop_dispatch_node.cpp:179-184` already uses
the error_code overload (`state_publisher_->send(msg, ec)`) and handles errors instead of throwing.

### Fix applied (minimal)

- a17/dispatch/client.cpp `Client::disconnect`: use the azmq non-throwing overload
  (`azmqsocket_.disconnect(address, ec)`), log the error as a warning instead of throwing, keep
  the existing `addresses_` erase and `on_disconnect_` ordering. This is the only function
  implicated by the crash log (`Subscriber [topic] !@ ...` log format exists solely there).

Deliberately NOT changed (no evidence they are involved -- backlog only if ever hit):

- a17/dispatch/client.cpp `Client::connect`: a connect failure would have aborted at connect time,
  which never happened (messages were flowing).
- a17/dispatch/request_client.cpp: same latent class, but not exercised by the chatter listener.
- a17/dispatch/server.cpp `Server::unbind`: unbinds the node's own endpoint; different path, not
  implicated by the crash log.
- Regression test in chatter_node_test.cpp: in-process teardown likely does not reproduce the EINVAL
  (the endpoint is registered on the listener's own socket; peer teardown alone may not remove it),
  so the test could pass on unfixed code. The real reproducer is the cross-process kill.

### Revisit checklist

- [x] Inspect other working dispatch nodes (C++ and how they subscribe/disconnect) -- estop reviewed
      (REFERENCE_CODE/estop, evidence above).
- [x] Apply minimal fix to the proven crash site (`Client::disconnect` only).
- [ ] Confirm the EINVAL trigger in libzmq 4.2.2 (informational; the fix is correct regardless of
      why zmq refuses the disconnect).
- [ ] Validate: cross-process kill of the talker no longer aborts the listener; CI passes.