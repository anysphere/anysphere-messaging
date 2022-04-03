//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { Message, Friend } from "../types";

declare global {
  interface Window {
    send(message: string, to: string): Promise<boolean>;
    copyToClipboard(s: string): void;
    getNewMessages(): Promise<Message[]>;
    getAllMessages(): Promise<Message[]>;
    getNewMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;
    getAllMessagesStreamed(messageHandler: (_: Message[]) => void): () => void;
    generateFriendKey(
      requestedFriend: string
    ): Promise<null | { friend: string; key: string }>;
    addFriend(
      requestedFriend: string,
      requestedFriendKey: string
    ): Promise<boolean>;
    messageSeen(message_id: string): Promise<boolean>;
    hasRegistered(): Promise<boolean>;
    register(username: string, accessKey: string): Promise<boolean>;
    isPlatformMac(): boolean;
    getFriendList(): Promise<Friend[]>;
    getOutboxMessages(): Promise<Message[]>;
    getSentMessages(): Promise<Message[]>;
  }
}

export {};
