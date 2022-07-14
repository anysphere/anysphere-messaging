//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { contextBridge, clipboard, contentTracing } from "electron";
import { promisify } from "util";
import grpc from "@grpc/grpc-js";
import daemonM from "../daemon/schema/daemon_pb";
import * as daemon_pb from "../daemon/schema/daemon_pb";
import {
  Friend,
  IncomingMessage,
  OutgoingMessage,
  protobufDateToDate,
  dateToProtobufDate,
} from "../types";

import { getDaemonClient } from "./daemon";
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

contextBridge.exposeInMainWorld(
  "getMessages",
  async (
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject
  ): Promise<daemon_pb.GetMessagesResponse.AsObject> => {
    if (FAKE_DATA) {
      if (
        getMessagesRequest.filter === daemon_pb.GetMessagesRequest.Filter.NEW
      ) {
        return {
          messagesList: [
            {
              uid: 1,
              message: "Hello! This is a test message.",
              fromUniqueName: "sualeh-asif",
              fromDisplayName: "Sualeh Asif",
              otherRecipientsList: [],
              deliveredAt: dateToProtobufDate(new Date()),
              seen: false,
              delivered: true,
            },
            {
              uid: 2,
              message:
                "This is a new message. It has been delivered, but not yet seen.",
              fromUniqueName: "stzh1555",
              fromDisplayName: "Shengtong Zhang",
              otherRecipientsList: [],
              deliveredAt: dateToProtobufDate(new Date()),
              seen: false,
              delivered: true,
            },
          ],
        };
      } else {
        return {
          messagesList: [
            {
              uid: 1,
              message: "Hello! This is a test message.",
              fromUniqueName: "sualeh-asif",
              fromDisplayName: "Sualeh Asif",
              otherRecipientsList: [],
              deliveredAt: dateToProtobufDate(new Date()),
              seen: false,
              delivered: true,
            },
            {
              uid: 2,
              message:
                "This is a new message. It has been delivered, but not yet seen.",
              fromUniqueName: "stzh1555",
              fromDisplayName: "Shengtong Zhang",
              otherRecipientsList: [],
              deliveredAt: dateToProtobufDate(new Date()),
              seen: false,
              delivered: true,
            },
            {
              uid: 3,
              message:
                "This is a new message. This message has been marked as seen!",
              fromUniqueName: "stzh1555",
              fromDisplayName: "Shengtong Zhang",
              otherRecipientsList: [],
              deliveredAt: dateToProtobufDate(new Date()),
              seen: true,
              delivered: true,
            },
          ],
        };
      }
    }
    const request = new daemonM.GetMessagesRequest();
    request.setFilter(getMessagesRequest.filter);
    const getMessages = promisify(daemonClient.getMessages).bind(daemonClient);
    return (
      (await getNewMessages(request)) as daemonM.GetMessagesResponse
    ).toObject();
  }
);

contextBridge.exposeInMainWorld(
  "getMessagesStreamed",
  (
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject,
    messageHandler: (_: IncomingMessage[]) => void
  ): (() => void) => {
    if (FAKE_DATA) {
      if (
        getMessagesRequest.filter === daemon_pb.GetMessagesRequest.Filter.NEW
      ) {
        const l: IncomingMessage[] = [
          {
            uid: 1,
            message:
              "Can you schedule a meeting with Srini for next week, please?",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: false,
            delivered: true,
          },
          {
            uid: 2,
            message:
              "I pushed some of my comments on the white paper draft to the repo, but I figured I should explain some of it here, too.",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: false,
            delivered: true,
          },
        ];
        messageHandler(l);
        return () => {
          console.log("Stopped streaming messages.");
        };
      } else {
        const l: IncomingMessage[] = [
          {
            uid: 1,
            message:
              "Can you schedule a meeting with Srini for next week, please?",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: false,
            delivered: true,
          },
          {
            uid: 2,
            message:
              "I pushed some of my comments on the white paper draft to the repo, but I figured I should explain some of it here, too.",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: false,
            delivered: true,
          },
          {
            uid: 3,
            message:
              "I was thinking about the thing you told me about the other day, and I think you're right.",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: true,
            delivered: true,
          },
          {
            uid: 4,
            message:
              "Can you take a look at the server branch and see if it works? I made some changes.",
            fromUniqueName: "sualeh-asif",
            fromDisplayName: "Sualeh Asif",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: true,
            delivered: true,
          },
          {
            uid: 5,
            message:
              "Hi Arvid,\n\nThank you so much for onboarding me to Anysphere! I am very excited to work with you.\n\nBest,\nShengtong",
            fromUniqueName: "stzh1555",
            fromDisplayName: "Shengtong Zhang",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: true,
            delivered: true,
          },
          {
            uid: 6,
            message:
              "Dear Arvid,\n\nThis is my first ever completely private message to you. No one will be able to read this message, find out when it was sent, or even suspect that I sent anything to you at all.\n\nHere's to a thoughtful, private and free future.\n\nYours truly,\nSualeh",
            fromUniqueName: "stzh1555",
            fromDisplayName: "Shengtong Zhang",
            otherRecipientsList: [],
            deliveredAt: dateToProtobufDate(new Date()),
            seen: true,
            delivered: true,
          },
        ];
        messageHandler(l);
        return () => {
          console.log("Stopped streaming messages.");
        };
      }
    }
    const request = new daemonM.GetMessagesRequest();
    request.setFilter(getMessagesRequest.filter);
    const call = daemonClient.getMessagesStreamed(request);
    call.on("data", function (r: daemonM.GetMessagesResponse) {
      try {
        const lm = r.getMessagesList();
        messageHandler(lm.map((m) => m.toObject()));
      } catch (e) {
        console.log(`error in getAllMessagesStreamed: ${e}`);
      }
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
  "getOutboxMessages",
  async (
    getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest.AsObject
  ): Promise<daemon_pb.GetOutboxMessagesResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        messagesList: [
          {
            uid: 1,
            message:
              "This is an outbox message that doesn't seem to want to be delivered...",
            toFriendsList: [
              {
                uniqueName: "sualeh-asif",
                displayName: "Sualeh Asif",
                delivered: false,
              },
            ],
            sentAt: dateToProtobufDate(new Date()),
          },
          {
            uid: 2,
            message: "I'm confused...",
            toFriendsList: [
              {
                uniqueName: "stzh1555",
                displayName: "Shengtong Zhang",
                delivered: false,
              },
            ],
            sentAt: dateToProtobufDate(new Date()),
          },
          {
            uid: 3,
            message: "I'm very confused...",
            toFriendsList: [
              {
                uniqueName: "stzh1555",
                displayName: "Shengtong Zhang",
                delivered: false,
              },
            ],
            sentAt: dateToProtobufDate(new Date()),
          },
        ],
      };
    }
    const request = new daemonM.GetOutboxMessagesRequest();
    const getOutboxMessages = promisify(daemonClient.getOutboxMessages).bind(
      daemonClient
    );
    return (
      (await getOutboxMessages(request)) as daemonM.GetOutboxMessagesResponse
    ).toObject();
  }
);

contextBridge.exposeInMainWorld(
  "getSentMessages",
  async (
    getSentMessagesRequest: daemon_pb.GetSentMessagesRequest.AsObject
  ): Promise<daemon_pb.GetSentMessagesResponse.AsObject> => {
    if (FAKE_DATA) {
      return {
        messagesList: [
          {
            uid: 1,
            message: "The draft looks good. Let me know if you need any help!",
            toFriendsList: [
              {
                uniqueName: "sualeh-asif",
                displayName: "Sualeh Asif",
                delivered: true,
                deliveredAt: dateToProtobufDate(new Date()),
              },
            ],
            sentAt: dateToProtobufDate(new Date()),
          },
          {
            uid: 2,
            message:
              "Welcome! We are extremely excited to have you as a part of our team.",
            toFriendsList: [
              {
                uniqueName: "shengtong-zhang",
                displayName: "Shengtong Zhang",
                delivered: true,
                deliveredAt: dateToProtobufDate(new Date()),
              },
            ],
            sentAt: dateToProtobufDate(new Date()),
          },
        ],
      };
    }

    const request = new daemonM.GetSentMessagesRequest();
    const getSentMessages = promisify(daemonClient.getSentMessages).bind(
      daemonClient
    );
    return (
      (await getSentMessages(request)) as daemonM.GetSentMessagesResponse
    ).toObject();
  }
);

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
