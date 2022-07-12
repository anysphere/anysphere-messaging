//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as daemon_pb from "./daemon/schema/daemon_pb";

export type Message = {
  id: number;
  from: string;
  to: string;
  message: string;
  timestamp: Date;
  type: "outgoing" | "incoming";
};

export type Friend = daemon_pb.FriendInfo.AsObject;
