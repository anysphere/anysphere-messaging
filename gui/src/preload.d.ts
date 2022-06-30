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
      registerUserRequest: daemon_pb.RegisterUserRequest
    ): Promise<boolean>;

    getMyPublicID(): Promise<daemon_pb.GetMyPublicIDResponse.AsObject>;

    getFriendList(): Promise<Friend[]>;

    removeFriend(
      removeFriendRequest: daemon_pb.RemoveFriendRequest
    ): Promise<boolean>;

    addSyncFriend(
      addSyncFriendRequest: daemon_pb.AddSyncFriendRequest
    ): Promise<boolean>;

    addAsyncFriend(
      addAsyncFriendRequest: daemon_pb.AddAsyncFriendRequest
    ): Promise<boolean>;

    getOutgoingSyncInvitations(): Promise<daemon_pb.GetOutgoingSyncInvitationsResponse>;

    getOutgoingAsyncInvitations(
      getOutgoingAsyncInvitationsRequest: daemon_pb.GetOutgoingAsyncInvitationsRequest
    ): Promise<daemon_pb.GetOutgoingAsyncInvitationsResponse>;

    getIncomingAsyncInvitations(
      getIncomingAsyncInvitationsRequest: daemon_pb.GetIncomingAsyncInvitationsRequest
    ): Promise<daemon_pb.GetIncomingAsyncInvitationsResponse>;

    acceptAsyncInvitation(
      acceptAsyncInvitationRequest: daemon_pb.AcceptAsyncInvitationRequest
    ): Promise<boolean>;

    rejectAsyncInvitation(
      rejectAsyncInvitationRequest: daemon_pb.RejectAsyncInvitationRequest
    ): Promise<daemon_pb.RejectAsyncInvitationResponse>;

    sendMessage(
      sendMessageRequest: daemon_pb.SendMessageRequest
    ): Promise<daemon_pb.SendMessageResponse>;

    getMessages(
      getMessagesRequest: daemon_pb.GetMessagesRequest
    ): Promise<daemon_pb.GetMessagesResponse>;

    getMessagesStreamed(
      getMessagesRequest: daemon_pb.GetMessagesRequest
    ): Promise<daemon_pb.GetMessagesResponse>;

    getOutboxMessages(
      getOutboxMessagesRequest: daemon_pb.GetOutboxMessagesRequest
    ): Promise<daemon_pb.GetOutboxMessagesResponse>;

    getSentMessages(
      getSentMessagesRequest: daemon_pb.GetSentMessagesRequest
    ): Promise<daemon_pb.GetSentMessagesResponse>;

    messageSeen(
      messageSeenRequest: daemon_pb.MessageSeenRequest
    ): Promise<daemon_pb.MessageSeenResponse>;

    getStatus(
      getStatusRequest: daemon_pb.GetStatusRequest
    ): Promise<daemon_pb.GetStatusResponse>;

    getLatency(
      getLatencyRequest: daemon_pb.GetLatencyRequest
    ): Promise<daemon_pb.GetLatencyResponse>;

    changeLatency(
      changeLatencyRequest: daemon_pb.ChangeLatencyRequest
    ): Promise<daemon_pb.ChangeLatencyResponse>;

    kill(killRequest: daemon_pb.KillRequest): Promise<daemon_pb.KillResponse>;

    // getNewMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;
    // getAllMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;

    send(message: string, to: string): Promise<boolean>;
    getNewMessages(): Promise<Message[]>;
    getAllMessages(): Promise<Message[]>;
    getNewMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;
    getAllMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;

    messageSeenOLD(message_id: number): Promise<boolean>;
    hasRegistered(): Promise<boolean>;
    register(username: string, accessKey: string): Promise<boolean>;
    getOutboxMessagesOLD(): Promise<Message[]>;
    getSentMessagesOLD(): Promise<Message[]>;
  }
}

export {};
