//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as grpc from "@grpc/grpc-js";
import * as daemon_pb from "../daemon/schema/daemon_pb";
import { Friend, IncomingMessage, dateToProtobufDate } from "../types";
import daemonS from "../daemon/schema/daemon_grpc_pb";
import path from "path";
import { promisify } from "util";
import { RELEASE_COMMIT_HASH } from "./constants";

const FAKE_DATA = process.env["ASPHR_FAKE_DATA"] === "true";

function getSocketPath(): string {
  if (process.env["XDG_RUNTIME_DIR"] != null) {
    return path.join(
      process.env["XDG_RUNTIME_DIR"],
      "anysphere",
      "anysphere.sock"
    );
  } else if (process.env["XDG_CONFIG_HOME"] != null) {
    return path.join(
      process.env["XDG_CONFIG_HOME"],
      "anysphere",
      "run",
      "anysphere.sock"
    );
  } else if (process.env["HOME"] != null) {
    return path.join(
      process.env["HOME"],
      ".anysphere",
      "run",
      "anysphere.sock"
    );
  } else {
    process.stderr.write(
      "$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set! Cannot find socket, aborting. :("
    );
    throw new Error("$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set!");
  }
}

function getSocketAddress(): string {
  return "unix://" + getSocketPath();
}

export function getDaemonClient(): daemonS.DaemonClient {
  return new daemonS.DaemonClient(
    getSocketAddress(),
    grpc.credentials.createInsecure()
  );
}

export function truncate(str: string, maxLength: number): string {
  if (str.length <= maxLength) {
    return str;
  }
  return str.substring(0, maxLength - 3) + "...";
}

export interface Daemon {
  registerUser(
    registerUserRequest: daemon_pb.RegisterUserRequest.AsObject
  ): Promise<daemon_pb.RegisterUserResponse.AsObject>;

  getMyPublicID(): Promise<daemon_pb.GetMyPublicIDResponse.AsObject>;

  getFriendList(): Promise<Friend[]>;

  removeFriend(
    removeFriendRequest: daemon_pb.RemoveFriendRequest.AsObject
  ): Promise<daemon_pb.RemoveFriendResponse.AsObject>;

  addSyncFriend(
    addSyncFriendRequest: daemon_pb.AddSyncFriendRequest.AsObject
  ): Promise<daemon_pb.AddSyncFriendResponse.AsObject>;

  addAsyncFriend(
    addAsyncFriendRequest: daemon_pb.AddAsyncFriendRequest.AsObject
  ): Promise<daemon_pb.AddAsyncFriendResponse.AsObject>;

  getOutgoingSyncInvitations(
    getOutgoingSyncInvitationsRequest: daemon_pb.GetOutgoingSyncInvitationsRequest.AsObject
  ): Promise<daemon_pb.GetOutgoingSyncInvitationsResponse.AsObject>;

  getOutgoingAsyncInvitations(
    getOutgoingAsyncInvitationsRequest: daemon_pb.GetOutgoingAsyncInvitationsRequest.AsObject
  ): Promise<daemon_pb.GetOutgoingAsyncInvitationsResponse.AsObject>;

  getIncomingAsyncInvitations(
    getIncomingAsyncInvitationsRequest: daemon_pb.GetIncomingAsyncInvitationsRequest.AsObject
  ): Promise<daemon_pb.GetIncomingAsyncInvitationsResponse.AsObject>;

  acceptAsyncInvitation(
    acceptAsyncInvitationRequest: daemon_pb.AcceptAsyncInvitationRequest.AsObject
  ): Promise<daemon_pb.AcceptAsyncInvitationResponse.AsObject>;

  rejectAsyncInvitation(
    rejectAsyncInvitationRequest: daemon_pb.RejectAsyncInvitationRequest.AsObject
  ): Promise<daemon_pb.RejectAsyncInvitationResponse.AsObject>;

  sendMessage(
    sendMessageRequest: daemon_pb.SendMessageRequest.AsObject
  ): Promise<daemon_pb.SendMessageResponse.AsObject>;

  getMessages(
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject
  ): Promise<daemon_pb.GetMessagesResponse.AsObject>;

  getOutboxMessages(
    getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest.AsObject
  ): Promise<daemon_pb.GetOutboxMessagesResponse.AsObject>;

  getSentMessages(
    getSentMessagesRequest: daemon_pb.GetSentMessagesRequest.AsObject
  ): Promise<daemon_pb.GetSentMessagesResponse.AsObject>;

  messageSeen(
    messageSeenRequest: daemon_pb.MessageSeenRequest.AsObject
  ): Promise<daemon_pb.MessageSeenResponse.AsObject>;

  //
  // not super necessary in the UI
  //

  getStatus(
    getStatusRequest: daemon_pb.GetStatusRequest.AsObject
  ): Promise<daemon_pb.GetStatusResponse.AsObject>;

  // getLatency(
  //   getLatencyRequest: daemon_pb.GetLatencyRequest.AsObject
  // ): Promise<daemon_pb.GetLatencyResponse.AsObject>;

  // changeLatency(
  //   changeLatencyRequest: daemon_pb.ChangeLatencyRequest.AsObject
  // ): Promise<daemon_pb.ChangeLatencyResponse.AsObject>;

  // kill(
  //   killRequest: daemon_pb.KillRequest.AsObject
  // ): Promise<daemon_pb.KillResponse.AsObject>;

  getMessagesStreamed(
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject,
    messageHandler: (_: IncomingMessage[]) => void
  ): () => void;

  hasRegistered(): Promise<boolean>;
}

export class DaemonImpl implements Daemon {
  private readonly client: daemonS.DaemonClient;

  public constructor() {
    this.client = getDaemonClient();
  }

  public async sendMessage(
    sendMessageRequest: daemon_pb.SendMessageRequest.AsObject
  ): Promise<daemon_pb.SendMessageResponse.AsObject> {
    if (FAKE_DATA) {
      console.log("Sending message:", sendMessageRequest);
      return {};
    }
    const request = new daemon_pb.SendMessageRequest();
    request.setUniqueNameList(sendMessageRequest.uniqueNameList);
    request.setMessage(sendMessageRequest.message);

    const boundSendMessage: (
      argument: daemon_pb.SendMessageRequest,
      callback: grpc.requestCallback<daemon_pb.SendMessageResponse>
    ) => grpc.ClientUnaryCall = this.client.sendMessage.bind(this.client);
    const promisifiedSendMessage = promisify(boundSendMessage);
    const response = await promisifiedSendMessage(request);

    if (response === undefined) {
      throw new Error("sendMessage returned undefined");
    }
    return response.toObject();
  }

  public async getStatus(
    getStatusRequest: daemon_pb.GetStatusRequest.AsObject
  ): Promise<daemon_pb.GetStatusResponse.AsObject> {
    if (FAKE_DATA) {
      console.log("Getting status:", getStatusRequest);
      return {
        registered: true,
        releaseHash: RELEASE_COMMIT_HASH,
        latencySeconds: 60,
        serverAddress: "server1.anysphere.co:443",
      };
    }
    const request = new daemon_pb.GetStatusRequest();

    const boundGetStatus: (
      argument: daemon_pb.GetStatusRequest,
      callback: grpc.requestCallback<daemon_pb.GetStatusResponse>
    ) => grpc.ClientUnaryCall = this.client.getStatus.bind(this.client);
    const promisifiedGetStatus = promisify(boundGetStatus);
    const response = await promisifiedGetStatus(request);

    if (response === undefined) {
      throw new Error("getStatus returned undefined");
    }

    return response.toObject();
  }

  public async getMessages(
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject
  ): Promise<daemon_pb.GetMessagesResponse.AsObject> {
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
    const request = new daemon_pb.GetMessagesRequest();
    request.setFilter(getMessagesRequest.filter);

    const boundGetMessages: (
      argument: daemon_pb.GetMessagesRequest,
      callback: grpc.requestCallback<daemon_pb.GetMessagesResponse>
    ) => grpc.ClientUnaryCall = this.client.getMessages.bind(this.client);
    const promisifiedGetMessages = promisify(boundGetMessages);
    const response = await promisifiedGetMessages(request);

    if (response === undefined) {
      throw new Error("getMessages returned undefined");
    }
    return response.toObject();
  }

  public getMessagesStreamed(
    getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject,
    messageHandler: (_: IncomingMessage[]) => void
  ): () => void {
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
    const request = new daemon_pb.GetMessagesRequest();
    request.setFilter(getMessagesRequest.filter);

    const call = this.client.getMessagesStreamed(request);

    call.on("data", function (r: daemon_pb.GetMessagesResponse) {
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

  public async getOutboxMessages(
    getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest.AsObject
  ): Promise<daemon_pb.GetOutboxMessagesResponse.AsObject> {
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
    const request = new daemon_pb.GetOutboxMessagesRequest();

    const boundGetOutboxMessages: (
      argument: daemon_pb.GetOutboxMessagesRequest,
      callback: grpc.requestCallback<daemon_pb.GetOutboxMessagesResponse>
    ) => grpc.ClientUnaryCall = this.client.getOutboxMessages.bind(this.client);
    const promisifiedGetOutboxMessages = promisify(boundGetOutboxMessages);
    const response = await promisifiedGetOutboxMessages(request);

    if (response === undefined) {
      throw new Error("getOutboxMessages returned undefined");
    }
    return response.toObject();
  }

  public async getSentMessages(
    getSentMessagesRequest: daemon_pb.GetSentMessagesRequest.AsObject
  ): Promise<daemon_pb.GetSentMessagesResponse.AsObject> {
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

    const request = new daemon_pb.GetSentMessagesRequest();

    const boundGetSentMessages: (
      argument: daemon_pb.GetSentMessagesRequest,
      callback: grpc.requestCallback<daemon_pb.GetSentMessagesResponse>
    ) => grpc.ClientUnaryCall = this.client.getSentMessages.bind(this.client);
    const promisifiedGetSentMessages = promisify(boundGetSentMessages);
    const response = await promisifiedGetSentMessages(request);

    if (response === undefined) {
      throw new Error("getSentMessages returned undefined");
    }
    return response.toObject();
  }

  public async messageSeen(
    messageSeenRequest: daemon_pb.MessageSeenRequest.AsObject
  ): Promise<daemon_pb.MessageSeenResponse.AsObject> {
    const request = new daemon_pb.MessageSeenRequest();
    request.setId(messageSeenRequest.id);

    const boundMessageSeen: (
      argument: daemon_pb.MessageSeenRequest,
      callback: grpc.requestCallback<daemon_pb.MessageSeenResponse>
    ) => grpc.ClientUnaryCall = this.client.messageSeen.bind(this.client);
    const promisifiedMessageSeen = promisify(boundMessageSeen);
    const response = await promisifiedMessageSeen(request);

    if (response === undefined) {
      throw new Error("messageSeen returned undefined");
    }
    return response.toObject();
  }

  public async hasRegistered(): Promise<boolean> {
    if (FAKE_DATA) {
      return true;
    }
    const request = new daemon_pb.GetStatusRequest();

    const boundGetStatus: (
      argument: daemon_pb.GetStatusRequest,
      callback: grpc.requestCallback<daemon_pb.GetStatusResponse>
    ) => grpc.ClientUnaryCall = this.client.getStatus.bind(this.client);
    const promisifiedGetStatus = promisify(boundGetStatus);
    const response = await promisifiedGetStatus(request);

    if (response === undefined) {
      throw new Error("hasRegistered returned undefined");
    }

    return response.getRegistered();
  }

  public async registerUser(
    registerUserRequest: daemon_pb.RegisterUserRequest.AsObject
  ): Promise<daemon_pb.RegisterUserResponse.AsObject> {
    const request = new daemon_pb.RegisterUserRequest();
    request.setName(registerUserRequest.name);
    request.setBetaKey(registerUserRequest.betaKey);

    const boundRegisterUser: (
      argument: daemon_pb.RegisterUserRequest,
      callback: grpc.requestCallback<daemon_pb.RegisterUserResponse>
    ) => grpc.ClientUnaryCall = this.client.registerUser.bind(this.client);
    const promisifiedRegisterUser = promisify(boundRegisterUser);
    const response = await promisifiedRegisterUser(request);

    if (response === undefined) {
      throw new Error("registerUser returned undefined");
    }
    return response.toObject();
  }

  public async getFriendList(): Promise<Friend[]> {
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

    const request = new daemon_pb.GetFriendListRequest();

    const boundGetFriendList: (
      argument: daemon_pb.GetFriendListRequest,
      callback: grpc.requestCallback<daemon_pb.GetFriendListResponse>
    ) => grpc.ClientUnaryCall = this.client.getFriendList.bind(this.client);
    const promisifiedGetFriendList = promisify(boundGetFriendList);
    const response = await promisifiedGetFriendList(request);

    if (response === undefined) {
      throw new Error("getFriendList returned undefined");
    }

    return response
      .getFriendInfosList()
      .map((friendInfo) => friendInfo.toObject());
  }

  public async removeFriend(
    removeFriendRequest: daemon_pb.RemoveFriendRequest.AsObject
  ): Promise<daemon_pb.RemoveFriendResponse.AsObject> {
    if (FAKE_DATA) {
      return {};
    }

    const request = new daemon_pb.RemoveFriendRequest();
    request.setUniqueName(removeFriendRequest.uniqueName);

    const boundRemoveFriend: (
      argument: daemon_pb.RemoveFriendRequest,
      callback: grpc.requestCallback<daemon_pb.RemoveFriendResponse>
    ) => grpc.ClientUnaryCall = this.client.removeFriend.bind(this.client);
    const promisifiedRemoveFriend = promisify(boundRemoveFriend);
    const response = await promisifiedRemoveFriend(request);

    if (response === undefined) {
      throw new Error("removeFriend returned undefined");
    }

    return response.toObject();
  }

  public async addAsyncFriend(
    addAsyncFriendRequest: daemon_pb.AddAsyncFriendRequest.AsObject
  ): Promise<daemon_pb.AddAsyncFriendResponse.AsObject> {
    if (FAKE_DATA) {
      return {};
    }

    const request = new daemon_pb.AddAsyncFriendRequest();
    request.setUniqueName(addAsyncFriendRequest.uniqueName);
    request.setDisplayName(addAsyncFriendRequest.displayName);
    request.setPublicId(addAsyncFriendRequest.publicId);
    request.setMessage(addAsyncFriendRequest.message);

    const boundAddAsyncFriend: (
      argument: daemon_pb.AddAsyncFriendRequest,
      callback: grpc.requestCallback<daemon_pb.AddAsyncFriendResponse>
    ) => grpc.ClientUnaryCall = this.client.addAsyncFriend.bind(this.client);
    const promisifiedAddAsyncFriend = promisify(boundAddAsyncFriend);
    const response = await promisifiedAddAsyncFriend(request);

    if (response === undefined) {
      throw new Error("addAsyncFriend returned undefined");
    }

    return response.toObject();
  }

  public async addSyncFriend(
    addSyncFriendRequest: daemon_pb.AddSyncFriendRequest.AsObject
  ): Promise<daemon_pb.AddAsyncFriendResponse.AsObject> {
    if (FAKE_DATA) {
      return {};
    }

    const request = new daemon_pb.AddSyncFriendRequest();
    request.setUniqueName(addSyncFriendRequest.uniqueName);
    request.setDisplayName(addSyncFriendRequest.displayName);
    request.setStory(addSyncFriendRequest.story);

    const boundAddSyncFriend: (
      argument: daemon_pb.AddSyncFriendRequest,
      callback: grpc.requestCallback<daemon_pb.AddSyncFriendResponse>
    ) => grpc.ClientUnaryCall = this.client.addSyncFriend.bind(this.client);
    const promisifiedAddSyncFriend = promisify(boundAddSyncFriend);
    const response = await promisifiedAddSyncFriend(request);

    if (response === undefined) {
      throw new Error("addSyncFriend returned undefined");
    }

    return response.toObject();
  }

  public async getOutgoingSyncInvitations(
    getOutgoingSyncInvitationsRequest: daemon_pb.GetOutgoingSyncInvitationsRequest.AsObject
  ): Promise<daemon_pb.GetOutgoingSyncInvitationsResponse.AsObject> {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            uniqueName: "arvid",
            displayName: "Arvid Lunnemark",
            story:
              "Haphazard ambassador hides gradient. Cool mark resolves ambush. Eerie kiss revises distinction. Fashionable film fosters age. New customer views proprietor. Boiled plant kicks truck. Spectacular commission complies triumph. Faint.",
            sentAt: dateToProtobufDate(new Date()),
          },
        ],
      };
    }

    const request = new daemon_pb.GetOutgoingSyncInvitationsRequest();

    const boundGetOutgoingSyncInvitations: (
      argument: daemon_pb.GetOutgoingSyncInvitationsRequest,
      callback: grpc.requestCallback<daemon_pb.GetOutgoingSyncInvitationsResponse>
    ) => grpc.ClientUnaryCall = this.client.getOutgoingSyncInvitations.bind(
      this.client
    );
    const promisifiedGetOutgoingSyncInvitations = promisify(
      boundGetOutgoingSyncInvitations
    );
    const response = await promisifiedGetOutgoingSyncInvitations(request);

    if (response === undefined) {
      throw new Error("getOutgoingSyncInvitations returned undefined");
    }

    return response.toObject();
  }

  public async getOutgoingAsyncInvitations(): Promise<daemon_pb.GetOutgoingAsyncInvitationsResponse.AsObject> {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            uniqueName: "OutgoingSualeh",
            displayName: "Sualeh Asif ooooo",
            publicId: "asdfasdf",
            message: "hihihihihihihihih",
            sentAt: dateToProtobufDate(new Date()),
          },
          {
            uniqueName: "OutgoingShengtong",
            displayName: "Shengtong aaaaaa",
            publicId: "asdfasdf",
            message: "hihihihihihihihih",
            sentAt: dateToProtobufDate(new Date()),
          },
        ],
      };
    }

    const request = new daemon_pb.GetOutgoingAsyncInvitationsRequest();

    const boundGetOutgoingAsyncInvitations: (
      argument: daemon_pb.GetOutgoingAsyncInvitationsRequest,
      callback: grpc.requestCallback<daemon_pb.GetOutgoingAsyncInvitationsResponse>
    ) => grpc.ClientUnaryCall = this.client.getOutgoingAsyncInvitations.bind(
      this.client
    );
    const promisifiedGetOutgoingAsyncInvitations = promisify(
      boundGetOutgoingAsyncInvitations
    );
    const response = await promisifiedGetOutgoingAsyncInvitations(request);

    if (response === undefined) {
      throw new Error("getOutgoingAsyncInvitations returned undefined");
    }

    return response.toObject();
  }

  public async getIncomingAsyncInvitations(): Promise<daemon_pb.GetIncomingAsyncInvitationsResponse.AsObject> {
    if (FAKE_DATA) {
      return {
        invitationsList: [
          {
            message: "Im First",
            publicId: "asdfasdf",
            receivedAt: dateToProtobufDate(new Date()),
          },
          {
            message: "Im Second",
            publicId: "asdfasdf",
            receivedAt: dateToProtobufDate(new Date()),
          },
        ],
      };
    }

    const request = new daemon_pb.GetIncomingAsyncInvitationsRequest();

    const boundGetIncomingAsyncInvitations: (
      argument: daemon_pb.GetIncomingAsyncInvitationsRequest,
      callback: grpc.requestCallback<daemon_pb.GetIncomingAsyncInvitationsResponse>
    ) => grpc.ClientUnaryCall = this.client.getIncomingAsyncInvitations.bind(
      this.client
    );
    const promisifiedGetIncomingAsyncInvitations = promisify(
      boundGetIncomingAsyncInvitations
    );
    const response = await promisifiedGetIncomingAsyncInvitations(request);

    if (response === undefined) {
      throw new Error("getIncomingAsyncInvitations returned undefined");
    }

    return response.toObject();
  }

  public async acceptAsyncInvitation(
    acceptAsyncInvitationRequest: daemon_pb.AcceptAsyncInvitationRequest.AsObject
  ): Promise<daemon_pb.AcceptAsyncInvitationResponse.AsObject> {
    if (FAKE_DATA) {
      return {};
    }

    const request = new daemon_pb.AcceptAsyncInvitationRequest();
    request.setPublicId(acceptAsyncInvitationRequest.publicId);
    request.setUniqueName(acceptAsyncInvitationRequest.uniqueName);
    request.setDisplayName(acceptAsyncInvitationRequest.displayName);

    const boundAcceptAsyncInvitation: (
      argument: daemon_pb.AcceptAsyncInvitationRequest,
      callback: grpc.requestCallback<daemon_pb.AcceptAsyncInvitationResponse>
    ) => grpc.ClientUnaryCall = this.client.acceptAsyncInvitation.bind(
      this.client
    );
    const promisifiedAcceptAsyncInvitation = promisify(
      boundAcceptAsyncInvitation
    );
    const response = await promisifiedAcceptAsyncInvitation(request);

    if (response === undefined) {
      throw new Error("acceptAsyncInvitation returned undefined");
    }

    return response.toObject();
  }

  public async rejectAsyncInvitation(
    rejectAsyncInvitationRequest: daemon_pb.RejectAsyncInvitationRequest.AsObject
  ): Promise<daemon_pb.RejectAsyncInvitationResponse.AsObject> {
    if (FAKE_DATA) {
      return {};
    }

    const request = new daemon_pb.RejectAsyncInvitationRequest();
    request.setPublicId(rejectAsyncInvitationRequest.publicId);

    const boundRejectAsyncInvitation: (
      argument: daemon_pb.RejectAsyncInvitationRequest,
      callback: grpc.requestCallback<daemon_pb.RejectAsyncInvitationResponse>
    ) => grpc.ClientUnaryCall = this.client.rejectAsyncInvitation.bind(
      this.client
    );
    const promisifiedRejectAsyncInvitation = promisify(
      boundRejectAsyncInvitation
    );
    const response = await promisifiedRejectAsyncInvitation(request);

    if (response === undefined) {
      throw new Error("rejectAsyncInvitation returned undefined");
    }

    return response.toObject();
  }

  public async getMyPublicID(): Promise<daemon_pb.GetMyPublicIDResponse.AsObject> {
    if (FAKE_DATA) {
      return {
        publicId:
          "3X2riNETQqUuFTqVrWqkGF8EsepzoarMYrNASmRJAzKwr9XB4Z7amnFNrgGm9wuq3wXAvvhPpu47eHWv29ikv5fbbzeeyfRvcgYqT",
        story:
          "Haphazard ambassador hides gradient. Cool mark resolves ambush. Eerie kiss revises distinction. Fashionable film fosters age. New customer views proprietor. Boiled plant kicks truck. Spectacular commission complies triumph. Faint.",
      };
    }

    const request = new daemon_pb.GetMyPublicIDRequest();

    const boundGetMyPublicID: (
      argument: daemon_pb.GetMyPublicIDRequest,
      callback: grpc.requestCallback<daemon_pb.GetMyPublicIDResponse>
    ) => grpc.ClientUnaryCall = this.client.getMyPublicID.bind(this.client);
    const promisifiedGetMyPublicID = promisify(boundGetMyPublicID);
    const response = await promisifiedGetMyPublicID(request);

    if (response === undefined) {
      throw new Error("getMyPublicID returned undefined");
    }

    return response.toObject();
  }
}
