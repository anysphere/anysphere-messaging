//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as daemon_pb from "./daemon/schema/daemon_pb";
import { Message, Friend } from "./types";

declare global {
  interface Window {
    copyToClipboard(s: string): void;
    isPlatformMac(): boolean;

    registerUser(
      registerUserRequest: daemon_pb.RegisterUserRequest.AsObject
    ): Promise<boolean>;

    getMyPublicID(): Promise<daemon_pb.GetMyPublicIDResponse.AsObject>;

    getFriendList(): Promise<Friend[]>;

    removeFriend(
      removeFriendRequest: daemon_pb.RemoveFriendRequest.AsObject
    ): Promise<boolean>;

    addSyncFriend(
      addSyncFriendRequest: daemon_pb.AddSyncFriendRequest.AsObject
    ): Promise<void>;

    addAsyncFriend(
      addAsyncFriendRequest: daemon_pb.AddAsyncFriendRequest.AsObject
    ): Promise<boolean>;

    getOutgoingSyncInvitations(): Promise<daemon_pb.GetOutgoingSyncInvitationsResponse.AsObject>;

    getOutgoingAsyncInvitations(
      getOutgoingAsyncInvitationsRequest: daemon_pb.GetOutgoingAsyncInvitationsRequest.AsObject
    ): Promise<daemon_pb.GetOutgoingAsyncInvitationsResponse.AsObject>;

    getIncomingAsyncInvitations(
      getIncomingAsyncInvitationsRequest: daemon_pb.GetIncomingAsyncInvitationsRequest.AsObject
    ): Promise<daemon_pb.GetIncomingAsyncInvitationsResponse.AsObject>;

    acceptAsyncInvitation(
      acceptAsyncInvitationRequest: daemon_pb.AcceptAsyncInvitationRequest.AsObject
    ): Promise<boolean>;

    rejectAsyncInvitation(
      rejectAsyncInvitationRequest: daemon_pb.RejectAsyncInvitationRequest.AsObject
    ): Promise<daemon_pb.RejectAsyncInvitationResponse.AsObject>;

    sendMessage(
      sendMessageRequest: daemon_pb.SendMessageRequest.AsObject
    ): Promise<daemon_pb.SendMessageResponse.AsObject>;

    getMessages(
      getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject
    ): Promise<daemon_pb.GetMessagesResponse.AsObject>;

    getMessagesStreamed(
      getMessagesRequest: daemon_pb.GetMessagesRequest.AsObject
    ): Promise<daemon_pb.GetMessagesResponse.AsObject>;

    getOutboxMessages(
      getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest.AsObject
    ): Promise<daemon_pb.GetOutboxMessagesResponse>;

    getSentMessages(
      getSentMessagesRequest: daemon_pb.GetSentMessagesRequest.AsObject
    ): Promise<daemon_pb.GetSentMessagesResponse.AsObject>;

    messageSeen(
      messageSeenRequest: daemon_pb.MessageSeenRequest.AsObject
    ): Promise<daemon_pb.MessageSeenResponse.AsObject>;

    getStatus(
      getStatusRequest: daemon_pb.GetStatusRequest.AsObject
    ): Promise<daemon_pb.GetStatusResponse.AsObject>;

    getLatency(
      getLatencyRequest: daemon_pb.GetLatencyRequest.AsObject
    ): Promise<daemon_pb.GetLatencyResponse.AsObject>;

    changeLatency(
      changeLatencyRequest: daemon_pb.ChangeLatencyRequest.AsObject
    ): Promise<daemon_pb.ChangeLatencyResponse.AsObject>;

    kill(
      killRequest: daemon_pb.KillRequest.AsObject
    ): Promise<daemon_pb.KillResponse.AsObject>;

    send(message: string, to: string): Promise<boolean>;
    getNewMessages(): Promise<Message[]>;
    getAllMessages(): Promise<Message[]>;
    getNewMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;
    getAllMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;

    hasRegistered(): Promise<boolean>;
    register(username: string, accessKey: string): Promise<boolean>;
    getOutboxMessagesOLD(): Promise<Message[]>;
    getSentMessagesOLD(): Promise<Message[]>;
  }
}

export {};
