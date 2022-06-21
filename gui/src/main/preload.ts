//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { contextBridge, clipboard } from "electron";
import { promisify } from "util";
import grpc from "@grpc/grpc-js";
import daemonM from "../daemon/schema/daemon_pb";
import {
  getDaemonClient,
  convertProtobufIncomingMessageToTypedMessage,
  convertProtobufOutgoingMessageToTypedMessage,
} from "./daemon";
import { Message } from "../types";
const daemonClient = getDaemonClient();

const FAKE_DATA = process.env.ASPHR_FAKE_DATA === "true";

contextBridge.exposeInMainWorld("send", async (message: string, to: string) => {
  if (FAKE_DATA) {
    return true;
  }
  const request = new daemonM.SendMessageRequest();
  request.setUniqueName(to);
  request.setMessage(message);
  const sendMessage = promisify(daemonClient.sendMessage).bind(daemonClient);
  try {
    const response = await sendMessage(request);
    console.log("send response", response);
    return true;
  } catch (e) {
    console.log(`error in send: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld("copyToClipboard", async (s: string) => {
  clipboard.writeText(s, "selection");
});

contextBridge.exposeInMainWorld("getNewMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "Alice",
        to: "me",
        message: "hello!\n\nthis is a test message\n\nbest,\nalice",
        timestamp: new Date(),
        type: "incoming",
      },
      {
        id: "2",
        from: "Bob",
        to: "me",
        message: "hi",
        timestamp: new Date(),
        type: "incoming",
      },
    ];
  }
  const request = new daemonM.GetMessagesRequest();
  request.setFilter(daemonM.GetMessagesRequest.Filter.NEW);
  const getNewMessages = promisify(daemonClient.getMessages).bind(daemonClient);
  try {
    const response = (await getNewMessages(
      request
    )) as daemonM.GetMessagesResponse;
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getNewMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld(
  "generateFriendKey",
  async (requestedFriend: string) => {
    if (FAKE_DATA) {
      return {
        friend: "sualeh",
        key: "6aFLPa03ldA9OyY-XlCRibbo3SG8Wsprw1iylnjvZIiFc",
      };
    }
    const request = new daemonM.GenerateFriendKeyRequest();
    request.setUniqueName(requestedFriend);
    const generateFriendKey = promisify(daemonClient.generateFriendKey).bind(
      daemonClient
    );
    try {
      const response = (await generateFriendKey(
        request
      )) as daemonM.GenerateFriendKeyResponse;
      return {
        friend: requestedFriend,
        key: response.getKey(),
      };
    } catch (e) {
      console.log(`error in generateFriendKey: ${e}`);
      return null;
    }
  }
);

contextBridge.exposeInMainWorld(
  "addFriend",
  async (requestedFriend: string, requestedFriendKey: string) => {
    const request = new daemonM.AddFriendRequest();
    request.setUniqueName(requestedFriend);
    request.setKey(requestedFriendKey);
    const addFriend = promisify(daemonClient.addFriend).bind(daemonClient);
    try {
      const response = await addFriend(request);
      console.log("addFriend response", response);
      return true;
    } catch (e) {
      console.log(`error in addFriend: ${e}`);
      return false;
    }
  }
);

contextBridge.exposeInMainWorld("getAllMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "Alice",
        to: "me",
        message: "hello",
        timestamp: new Date(),
        type: "incoming",
      },
      {
        id: "2",
        from: "Bob",
        to: "me",
        message: "hi",
        timestamp: new Date(),
        type: "incoming",
      },
      {
        id: "3",
        from: "Bob",
        to: "me",
        message: "hi this is my second message",
        timestamp: new Date(),
        type: "incoming",
      },
      {
        id: "4",
        from: "Bob",
        to: "me",
        message: "hi this is my first message",
        timestamp: new Date(),
        type: "incoming",
      },
    ];
  }
  const request = new daemonM.GetMessagesRequest();
  request.setFilter(daemonM.GetMessagesRequest.Filter.ALL);
  const getAllMessages = promisify(daemonClient.getMessages).bind(daemonClient);
  try {
    const response = (await getAllMessages(
      request
    )) as daemonM.GetMessagesResponse;
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getAllMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld(
  "getAllMessagesStreamed",
  (messageHandler: (_: Message[]) => void) => {
    if (FAKE_DATA) {
      const l: Message[] = [
        {
          id: 5,
          from: "Sualeh",
          to: "me",
          message:
            "Can you schedule a meeting with Srini for next week, please?",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 5,
          from: "Sualeh",
          to: "me",
          message:
            "I pushed some of my comments on the white paper draft to the repo, but I figured I should explain some of it here, too.",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 2,
          from: "Sualeh",
          to: "me",
          message:
            "I was thinking about the thing you told me about the other day, and I think you're right.",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 3,
          from: "Sualeh",
          to: "me",
          message:
            "Can you take a look at the server branch and see if it works? I made some changes.",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 4,
          from: "Shengtong",
          to: "me",
          message:
            "Hi Arvid,\n\nThank you so much for onboarding me to Anysphere! I am very excited to work with you.\n\nBest,\nShengtong",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 1,
          from: "Sualeh",
          to: "me",
          message:
            "Dear Arvid,\n\nThis is my first ever completely private message to you. No one will be able to read this message, find out when it was sent, or even suspect that I sent anything to you at all.\n\nHere's to a thoughtful, private and free future.\n\nYours truly,\nSualeh",
          timestamp: new Date(),
          type: "incoming",
        },
      ];
      messageHandler(l);
      return () => {};
    }
    const request = new daemonM.GetMessagesRequest();
    request.setFilter(daemonM.GetMessagesRequest.Filter.ALL);
    var call = daemonClient.getMessagesStreamed(request);
    call.on("data", function (r: daemonM.GetMessagesResponse) {
      try {
        const lm = r.getMessagesList();
        const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
        let l2: Message[] = [];
        for (const m of l) {
          if (m) {
            l2.push(m);
          }
        }
        messageHandler(l2);
      } catch (e) {
        console.log(`error in getAllMessagesStreamed: ${e}`);
      }
      console.log("got all messages streamed", r);
    });
    call.on("end", function () {
      // The server has finished sending
      console.log("getAllMessagesStreamed end");
    });
    call.on("error", function (e: Error) {
      // An error has occurred and the stream has been closed.
      console.log("getAllMessagesStreamed error", e);
    });
    call.on("status", function (status: grpc.StatusObject) {
      // process status
      console.log("getAllMessagesStreamed status", status);
    });
    return () => {
      console.log("cancelling grpc!");
      call.cancel();
    };
  }
);

contextBridge.exposeInMainWorld(
  "getNewMessagesStreamed",
  (messageHandler: (_: Message[]) => void) => {
    if (FAKE_DATA) {
      const l: Message[] = [
        {
          id: 5,
          from: "Sualeh",
          to: "me",
          message:
            "Can you schedule a meeting with Srini for next week, please?",
          timestamp: new Date(),
          type: "incoming",
        },
        {
          id: 5,
          from: "Sualeh",
          to: "me",
          message:
            "I pushed some of my comments on the white paper draft to the repo, but I figured I should explain some of it here, too.",
          timestamp: new Date(),
          type: "incoming",
        },
      ];
      messageHandler(l);
      return () => {};
    }
    const request = new daemonM.GetMessagesRequest();
    request.setFilter(daemonM.GetMessagesRequest.Filter.NEW);
    var call = daemonClient.getMessagesStreamed(request);
    call.on("data", function (r: daemonM.GetMessagesResponse) {
      try {
        const lm = r.getMessagesList();
        const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
        let l2: Message[] = [];
        for (const m of l) {
          if (m) {
            l2.push(m);
          }
        }
        messageHandler(l2);
      } catch (e) {
        console.log(`error in getNewMessagesStreamed: ${e}`);
      }
      console.log("got all messages streamed", r);
    });
    call.on("end", function () {
      // The server has finished sending
      console.log("getNewMessagesStreamed end");
    });
    call.on("error", function (e: Error) {
      // An error has occurred and the stream has been closed.
      console.log("getNewMessagesStreamed error", e);
    });
    call.on("status", function (status: grpc.StatusObject) {
      // process status
      console.log("getNewMessagesStreamed status", status);
    });
    return () => {
      console.log("cancelling grpc!");
      call.cancel();
    };
  }
);

contextBridge.exposeInMainWorld("getOutboxMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "me",
        to: "SUelAh",
        message: "hello!\n\nthis is a test message\n\nnot the best,\nSuAlEh",
        timestamp: new Date(),
        type: "outgoing",
      },
      {
        id: "2",
        from: "me",
        to: "HI",
        message: "HIHI",
        timestamp: new Date(),
        type: "outgoing",
      },
      {
        id: "3",
        from: "me",
        to: "Bob",
        message: "hi this is my second message",
        timestamp: new Date(),
        type: "outgoing",
      },
      {
        id: "4",
        from: "me",
        to: "Bob",
        message: "hi this is my first message",
        timestamp: new Date(),
        type: "outgoing",
      },
    ];
  }
  const request = new daemonM.GetOutboxMessagesRequest();
  const getOutboxMessages = promisify(daemonClient.getOutboxMessages).bind(
    daemonClient
  );
  try {
    const response = (await getOutboxMessages(
      request
    )) as daemonM.GetOutboxMessagesResponse;
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufOutgoingMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getOutboxMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("getSentMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "me",
        to: "Sualeh",
        message: "The draft looks good. Let me know if you need any help!",
        timestamp: new Date(),
        type: "outgoing",
      },
      {
        id: "2",
        from: "me",
        to: "Shengtong",
        message:
          "Welcome! We are extremely excited to have you as a part of our team.",
        timestamp: new Date(),
        type: "outgoing",
      },
    ];
  }
  const request = new daemonM.GetSentMessagesRequest();
  const getSentMessages = promisify(daemonClient.getSentMessages).bind(
    daemonClient
  );
  try {
    const response = (await getSentMessages(
      request
    )) as daemonM.GetSentMessagesResponse;
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufOutgoingMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getSentMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("getFriendList", async () => {
  if (FAKE_DATA) {
    return [
      {
        name: "Srini",
        status: "initiated",
      },
      {
        name: "Sualeh",
        status: "added",
      },
      {
        name: "Shengtong",
        status: "added",
      },
    ];
  }
  const request = new daemonM.GetFriendListRequest();
  const getFriendList = promisify(daemonClient.getFriendList).bind(
    daemonClient
  );
  try {
    const response = (await getFriendList(
      request
    )) as daemonM.GetFriendListResponse;
    const lm = response.getFriendInfosList();
    const l = lm.map((m) => {
      return {
        name: m.getUniqueName(),
        status: m.getEnabled() ? "added" : "initiated",
      };
    });
    return l;
  } catch (e) {
    console.log(`error in getFriendList: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("messageSeen", async (message_id: number) => {
  const request = new daemonM.MessageSeenRequest();
  request.setId(message_id);
  const messageSeen = promisify(daemonClient.messageSeen).bind(daemonClient);
  try {
    const response = await messageSeen(request);
    console.log("messageSeen response", response);
    return true;
  } catch (e) {
    console.log(`error in send: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld("hasRegistered", async () => {
  const request = new daemonM.GetStatusRequest();
  const getStatus = promisify(daemonClient.getStatus).bind(daemonClient);
  try {
    const response = (await getStatus(request)) as daemonM.GetStatusResponse;
    return response.getRegistered();
  } catch (e) {
    console.log(`error in hasRegistered: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld(
  "register",
  async (username: string, accessKey: string) => {
    const request = new daemonM.RegisterUserRequest();
    request.setName(username);
    request.setBetaKey(accessKey);
    const register = promisify(daemonClient.registerUser).bind(daemonClient);
    try {
      const response = await register(request);
      console.log("register response", response);
      return true;
    } catch (e) {
      console.log(`error in register: ${e}`);
    }
    return false;
  }
);

contextBridge.exposeInMainWorld("isPlatformMac", () => {
  return process.platform === "darwin";
});
