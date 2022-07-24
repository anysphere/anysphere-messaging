//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as daemon_pb from "./daemon/schema/daemon_pb";
import * as google_protobuf_timestamp_pb from "google-protobuf/google/protobuf/timestamp_pb";

export type IncomingMessage = daemon_pb.IncomingMessage.AsObject;
export type OutgoingMessage = daemon_pb.OutgoingMessage.AsObject;
export type Friend = daemon_pb.FriendInfo.AsObject;
export type OutgoingAsyncInvitation =
  daemon_pb.GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo.AsObject;
export type IncomingAsyncInvitation =
  daemon_pb.GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo.AsObject;
export type OutgoingSyncInvitation =
  daemon_pb.GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo.AsObject;

// TODO(sualeh): move these to utils
export function protobufDateToDate(
  date: google_protobuf_timestamp_pb.Timestamp.AsObject
): Date {
  return new Date(date.seconds * 1e3 + date.nanos / 1e6);
}

export function dateToProtobufDate(
  date: Date
): google_protobuf_timestamp_pb.Timestamp.AsObject {
  return {
    seconds: Math.floor(date.getTime() / 1e3),
    nanos: (date.getTime() % 1e3) * 1e6,
  };
}
