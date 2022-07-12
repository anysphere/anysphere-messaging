//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

const grpc = require("@grpc/grpc-js");
import daemonS from "../daemon/schema/daemon_grpc_pb";
import * as daemon_pb from "../daemon/schema/daemon_pb";
import { Message } from "../types";
import path from "path";

function get_socket_path() {
  if (process.env.XDG_RUNTIME_DIR) {
    return path.join(
      process.env.XDG_RUNTIME_DIR,
      "anysphere",
      "anysphere.sock"
    );
  } else if (process.env.XDG_CONFIG_HOME) {
    return path.join(
      process.env.XDG_CONFIG_HOME,
      "anysphere",
      "run",
      "anysphere.sock"
    );
  } else if (process.env.HOME) {
    return path.join(process.env.HOME, ".anysphere", "run", "anysphere.sock");
  } else {
    process.stderr.write(
      "$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set! Cannot find socket, aborting. :("
    );
    throw new Error("$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set!");
  }
}

function get_socket_address() {
  return "unix://" + get_socket_path();
}

export function getDaemonClient(): daemonS.DaemonClient {
  return new daemonS.DaemonClient(
    get_socket_address(),
    grpc.credentials.createInsecure()
  );
}

export function convertProtobufIncomingMessageToTypedMessage(
  m: daemon_pb.IncomingMessage
): Message | null {
  const msg = m.toObject();
  console.log("seconds", msg.deliveredAt?.seconds);
  let d: Date;
  if (msg.deliveredAt) {
    d = new Date(msg.deliveredAt.seconds * 1e3 + msg.deliveredAt.nanos / 1e6);
  } else {
    d = new Date();
  }

  // TODO(sualeh): clean this up.
  const M = m.getM();
  return M
    ? {
        id: M.getId(),
        from: M.getUniqueName(),
        to: "me",
        message: M.getMessage(),
        timestamp: d,
        type: "incoming",
      }
    : null;
}

export function convertProtobufOutgoingMessageToTypedMessage(
  m: daemon_pb.OutgoingMessage
): Message | null {
  const msg = m.toObject();
  console.log("seconds", msg.sentAt?.seconds);

  let d: Date;
  if (msg.sentAt) {
    d = new Date(msg.sentAt.seconds * 1e3 + msg.sentAt.nanos / 1e6);
  } else {
    d = new Date();
  }

  const M = m.getM();
  return M
    ? {
        id: M.getId(),
        from: "me",
        to: M.getUniqueName(),
        message: M.getMessage(),
        timestamp: d,
        type: "outgoing",
      }
    : null;
}

export function truncate(str: string, maxLength: number) {
  if (str.length <= maxLength) {
    return str;
  }
  return str.substring(0, maxLength - 3) + "...";
}
