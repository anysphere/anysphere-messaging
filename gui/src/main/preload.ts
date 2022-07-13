//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { contextBridge, clipboard, contentTracing } from "electron";
import { promisify } from "util";
import grpc from "@grpc/grpc-js";
import daemonM from "../daemon/schema/daemon_pb";
import * as daemon_pb from "../daemon/schema/daemon_pb";
import { Friend, Message } from "../types";

import {
  getDaemonClient,
  convertProtobufIncomingMessageToTypedMessage,
  convertProtobufOutgoingMessageToTypedMessage,
} from "./daemon";
import { Timestamp } from "google-protobuf/google/protobuf/timestamp_pb";

const daemonClient = getDaemonClient();

const FAKE_DATA = process.env["ASPHR_FAKE_DATA"] === "true";

contextBridge.exposeInMainWorld("copyToClipboard", (s: string) => {
  clipboard.writeText(s, "selection");
});

contextBridge.exposeInMainWorld("isPlatformMac", () => {
  return process.platform === "darwin";
});

contextBridge.exposeInMainWorld(
  "sendMessage",
  async (
    sendMessageRequest: daemon_pb.SendMessageRequest.AsObject
  ): Promise<daemon_pb.SendMessageResponse.AsObject> => {
    if (FAKE_DATA) {
      console.log("Sending message:", sendMessageRequest);
      return {};
    }
    const request = new daemonM.SendMessageRequest();
    request.setUniqueNameList(sendMessageRequest.uniqueNameList);
    request.setMessage(sendMessageRequest.message);
    const sendMessage = promisify(daemonClient.sendMessage).bind(daemonClient);
    return (
      (await sendMessage(request)) as daemon_pb.SendMessageResponse
    ).toObject();
  }
);

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
    const call = daemonClient.getMessagesStreamed(request);
    call.on("data", function (r: daemonM.GetMessagesResponse) {
      try {
        const lm = r.getMessagesList();
        const l = lm.map(convertProtobufIncomingMessageToTypedMessage);
        const l2: Message[] = [];
        for (const m of l) {
          if (m) {
            l2.push(m);
          }
        }
        messageHandler(l2);
      } catch (e) {
        console.log(`error in getAllMessagesStreamed: ${e}`);
      }
      // console.log("got all messages streamed", r);
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
      // console.log("got all messages streamed", r);
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

contextBridge.exposeInMainWorld(
  "getOutboxMessages",
  async (
    getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest.AsObject
  ): Promise<daemon_pb.GetOutboxMessagesResponse> => {
    // unimplemented
    console.error("getOutboxMessages not implemented");
    throw new Error("getOutboxMessages not implemented");
  }
);

contextBridge.exposeInMainWorld("getOutboxMessagesOLD", async () => {
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

contextBridge.exposeInMainWorld("getSentMessagesOLD", async () => {
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

contextBridge.exposeInMainWorld(
  "messageSeen",
  async (messageSeenRequest: daemon_pb.MessageSeenRequest.AsObject) => {
    const request = new daemonM.MessageSeenRequest();
    request.setId(messageSeenRequest.id);
    const messageSeen = promisify(daemonClient.messageSeen).bind(daemonClient);
    try {
      const response = await messageSeen(request);
      console.log("messageSeen response", response);
      return true;
    } catch (e) {
      console.log(`error in send: ${e}`);
      return false;
    }
  }
);

contextBridge.exposeInMainWorld("hasRegistered", async () => {
  if (FAKE_DATA) {
    return true;
  }
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
  "registerUser",
  async (registerUserRequest: daemon_pb.RegisterUserRequest.AsObject) => {
    const request = new daemonM.RegisterUserRequest();
    request.setName(registerUserRequest.name);
    request.setBetaKey(registerUserRequest.betaKey);
    const register = promisify(daemonClient.registerUser).bind(daemonClient);
    await register(request);
  }
);

contextBridge.exposeInMainWorld(
  "getFriendList",
  async (): Promise<Friend[]> => {
    if (FAKE_DATA) {
      return [
        {
          uniqueName: "Sualeh",
          displayName: "Sualeh Asif",
          publicId: "asdfasdf",
          invitationProgress: daemon_pb.InvitationProgress.COMPLETE,
        },
        {
          uniqueName: "Shengtong",
          displayName: "Shengtong",
          publicId: "asdfasdf",
          invitationProgress: daemon_pb.InvitationProgress.COMPLETE,
        },
        {
          uniqueName: "Bob",
          displayName: "Bob",
          publicId: "asdfasdf",
          invitationProgress: daemon_pb.InvitationProgress.COMPLETE,
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
        // TODO: fix this when the schema is cleaned up.
        return {
          uniqueName: m.getUniqueName(),
          displayName: m.getDisplayName(),
          publicId: m.getPublicId(),
          invitationProgress: m.getInvitationProgress(),
        };
      });
      return l;
    } catch (e) {
      console.log(`error in getFriendList: ${e}`);
      return [];
    }
  }
);

// removeFriend
contextBridge.exposeInMainWorld(
  "removeFriend",
  async (uniqueName: string): Promise<boolean> => {
    if (FAKE_DATA) {
      return true;
    }

    const request = new daemonM.RemoveFriendRequest();
    request.setUniqueName(uniqueName);
    const removeFriend = promisify(daemonClient.removeFriend).bind(
      daemonClient
    );

    try {
      await removeFriend(request);
      return true;
    } catch (e) {
      console.log(`error in removeFriend: ${e}`);
      return false;
    }
  }
);

// addAsyncFriend
contextBridge.exposeInMainWorld(
  "addAsyncFriend",
  async (
    addAsyncFriendRequest: daemon_pb.AddAsyncFriendRequest.AsObject
  ): Promise<boolean> => {
    if (FAKE_DATA) {
      return true;
    }

    const request = new daemonM.AddAsyncFriendRequest();
    request.setUniqueName(addAsyncFriendRequest.uniqueName);
    request.setDisplayName(addAsyncFriendRequest.displayName);
    request.setPublicId(addAsyncFriendRequest.publicId);
    request.setMessage(addAsyncFriendRequest.message);

    const addAsyncFriend = promisify(daemonClient.addAsyncFriend).bind(
      daemonClient
    );

    try {
      await addAsyncFriend(request);
      return true;
    } catch (e) {
      console.log(`error in addAsyncFriend: ${e}`);
      return false;
    }
  }
);

// addSyncFriend
contextBridge.exposeInMainWorld(
  "addSyncFriend",
  async (
    addSyncFriendRequest: daemon_pb.AddSyncFriendRequest.AsObject
  ): Promise<void> => {
    if (FAKE_DATA) {
      return;
    }

    const request = new daemonM.AddSyncFriendRequest();
    request.setUniqueName(addSyncFriendRequest.uniqueName);
    request.setDisplayName(addSyncFriendRequest.displayName);
    request.setStory(addSyncFriendRequest.story);

    const addSyncFriend = promisify(daemonClient.addSyncFriend).bind(
      daemonClient
    );

    try {
      await addSyncFriend(request);
    } catch (e) {
      console.log(`error in addSyncFriend: ${e}`);
      throw e;
    }
  }
);

// getOutgoingSyncInvitations
contextBridge.exposeInMainWorld(
  "getOutgoingSyncInvitations",
  async (): Promise<daemon_pb.GetOutgoingSyncInvitationsResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            uniqueName: "arvid",
            displayName: "Arvid Lunnemark",
            story:
              "Haphazard ambassador hides gradient. Cool mark resolves ambush. Eerie kiss revises distinction. Fashionable film fosters age. New customer views proprietor. Boiled plant kicks truck. Spectacular commission complies triumph. Faint.",
            sentAt: Timestamp.fromDate(new Date()).toObject(),
          },
        ],
      };
    }

    const request = new daemonM.GetOutgoingSyncInvitationsRequest();

    const getOutgoingSyncInvitations = promisify(
      daemonClient.getOutgoingSyncInvitations
    ).bind(daemonClient);

    try {
      const response = (await getOutgoingSyncInvitations(
        request
      )) as daemonM.GetOutgoingSyncInvitationsResponse;

      return response.toObject();
    } catch (e) {
      console.log(`error in getOutgoingSyncInvitations: ${e}`);
      return {
        invitationsList: [],
      };
    }
  }
);

// getOutgoingAsyncInvitations
contextBridge.exposeInMainWorld(
  "getOutgoingAsyncInvitations",
  async (): Promise<daemon_pb.GetOutgoingAsyncInvitationsResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            uniqueName: "OutgoingSualeh",
            displayName: "Sualeh Asif ooooo",
            publicId: "asdfasdf",
            message: "hihihihihihihihih",
            sentAt: Timestamp.fromDate(new Date()).toObject(),
          },
          {
            uniqueName: "OutgoingShengtong",
            displayName: "Shengtong aaaaaa",
            publicId: "asdfasdf",
            message: "hihihihihihihihih",
            sentAt: Timestamp.fromDate(new Date()).toObject(),
          },
        ],
      };
    }

    const request = new daemonM.GetOutgoingAsyncInvitationsRequest();

    const getOutgoingAsyncInvitations = promisify(
      daemonClient.getOutgoingAsyncInvitations
    ).bind(daemonClient);

    try {
      const response = (await getOutgoingAsyncInvitations(
        request
      )) as daemonM.GetOutgoingAsyncInvitationsResponse;

      return response.toObject();
    } catch (e) {
      console.log(`error in getOutgoingAsyncInvitations: ${e}`);
      return {
        invitationsList: [],
      };
    }
  }
);

// getIncomingAsyncInvitations
contextBridge.exposeInMainWorld(
  "getIncomingAsyncInvitations",
  async (): Promise<daemon_pb.GetIncomingAsyncInvitationsResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            message: "Im First",
            publicId: "asdfasdf",
            receivedAt: Timestamp.fromDate(new Date()).toObject(),
          },
          {
            message: "Im Second",
            publicId: "asdfasdf",
            receivedAt: Timestamp.fromDate(new Date()).toObject(),
          },
        ],
      };
    }

    const request = new daemonM.GetIncomingAsyncInvitationsRequest();

    const getIncomingAsyncInvitations = promisify(
      daemonClient.getIncomingAsyncInvitations
    ).bind(daemonClient);

    try {
      const response = (await getIncomingAsyncInvitations(
        request
      )) as daemonM.GetIncomingAsyncInvitationsResponse;

      return response.toObject();
    } catch (e) {
      console.log(`error in getIncomingAsyncInvitations: ${e}`);
      return {
        invitationsList: [],
      };
    }
  }
);

// acceptAsyncInvitation
contextBridge.exposeInMainWorld(
  "acceptAsyncInvitation",
  async (
    acceptAsyncInvitationRequest: daemon_pb.AcceptAsyncInvitationRequest.AsObject
  ): Promise<boolean> => {
    if (FAKE_DATA) {
      return true;
    }

    const request = new daemonM.AcceptAsyncInvitationRequest();
    request.setPublicId(acceptAsyncInvitationRequest.publicId);
    request.setUniqueName(acceptAsyncInvitationRequest.uniqueName);
    request.setDisplayName(acceptAsyncInvitationRequest.displayName);

    const acceptAsyncInvitation = promisify(
      daemonClient.acceptAsyncInvitation
    ).bind(daemonClient);

    try {
      await acceptAsyncInvitation(request);
      return true;
    } catch (e) {
      console.log(`error in acceptAsyncInvitation: ${e}`);
      return false;
    }
  }
);

// rejectAsyncInvitation
contextBridge.exposeInMainWorld(
  "rejectAsyncInvitation",
  async (
    rejectAsyncInvitationRequest: daemon_pb.RejectAsyncInvitationRequest.AsObject
  ): Promise<boolean> => {
    if (FAKE_DATA) {
      return true;
    }

    const request = new daemonM.RejectAsyncInvitationRequest();
    request.setPublicId(rejectAsyncInvitationRequest.publicId);

    const rejectAsyncInvitation = promisify(
      daemonClient.rejectAsyncInvitation
    ).bind(daemonClient);

    try {
      await rejectAsyncInvitation(request);
      return true;
    } catch (e) {
      console.log(`error in rejectAsyncInvitation: ${e}`);
      return false;
    }
  }
);

// getMyPublicID
contextBridge.exposeInMainWorld(
  "getMyPublicID",
  async (): Promise<daemon_pb.GetMyPublicIDResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        publicId:
          "3X2riNETQqUuFTqVrWqkGF8EsepzoarMYrNASmRJAzKwr9XB4Z7amnFNrgGm9wuq3wXAvvhPpu47eHWv29ikv5fbbzeeyfRvcgYqT",
        story:
          "Haphazard ambassador hides gradient. Cool mark resolves ambush. Eerie kiss revises distinction. Fashionable film fosters age. New customer views proprietor. Boiled plant kicks truck. Spectacular commission complies triumph. Faint.",
      };
    }

    const request = new daemonM.GetMyPublicIDRequest();

    const getMyPublicID = promisify(daemonClient.getMyPublicID).bind(
      daemonClient
    );

    try {
      const response = (await getMyPublicID(
        request
      )) as daemonM.GetMyPublicIDResponse;

      return response.toObject();
    } catch (e) {
      // TODO: maybe this error is unrecoverable? maybe we want to try to restart the daemon?
      console.log(`error in getMyPublicID: ${e}`);
      throw e;
    }
  }
);

/**
 * @deprecated
 * @param requestedFriend the friend to get messages for
 */
contextBridge.exposeInMainWorld(
  "generateFriendKey",
  async (requestedFriend: string) => {
    try {
      return {
        friend: requestedFriend,
        key: "6aFLPa03ldA9OyY-XlCRibbo3SG8Wsprw1iylnjvZIiFc",
      };
    } catch (e) {
      console.log(`error in generateFriendKey: ${e}`);
      return null;
    }
  }
);

/**
 * @deprecated
 * @param requestedFriend the friend to add.
 */
contextBridge.exposeInMainWorld(
  "addFriend",
  async (_requestedFriend: string, _requestedFriendKey: string) => {
    try {
      return true;
    } catch (e) {
      console.log(`error in addFriend: ${e}`);
      return false;
    }
  }
);

contextBridge.exposeInMainWorld("sendAsyncFriendRequest", async () => {});
