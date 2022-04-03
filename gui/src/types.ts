//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

export type Message = {
  id: string;
  from: string;
  to: string;
  message: string;
  timestamp: Date;
  type: "outgoing" | "incoming";
};

export type Friend = {
  name: string;
  status: "initiated" | "added";
};
