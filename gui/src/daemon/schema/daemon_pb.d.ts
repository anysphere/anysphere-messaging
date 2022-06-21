// @generated

// package: asphrdaemon
// file: schema/daemon.proto

import * as jspb from "google-protobuf";
import * as google_protobuf_timestamp_pb from "google-protobuf/google/protobuf/timestamp_pb";

export class RegisterUserRequest extends jspb.Message {
  getName(): string;
  setName(value: string): void;

  getBetaKey(): string;
  setBetaKey(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RegisterUserRequest.AsObject;
  static toObject(includeInstance: boolean, msg: RegisterUserRequest): RegisterUserRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RegisterUserRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RegisterUserRequest;
  static deserializeBinaryFromReader(message: RegisterUserRequest, reader: jspb.BinaryReader): RegisterUserRequest;
}

export namespace RegisterUserRequest {
  export type AsObject = {
    name: string,
    betaKey: string,
  }
}

export class RegisterUserResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RegisterUserResponse.AsObject;
  static toObject(includeInstance: boolean, msg: RegisterUserResponse): RegisterUserResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RegisterUserResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RegisterUserResponse;
  static deserializeBinaryFromReader(message: RegisterUserResponse, reader: jspb.BinaryReader): RegisterUserResponse;
}

export namespace RegisterUserResponse {
  export type AsObject = {
  }
}

export class GetFriendListRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetFriendListRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetFriendListRequest): GetFriendListRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetFriendListRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetFriendListRequest;
  static deserializeBinaryFromReader(message: GetFriendListRequest, reader: jspb.BinaryReader): GetFriendListRequest;
}

export namespace GetFriendListRequest {
  export type AsObject = {
  }
}

export class FriendInfo extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  getEnabled(): boolean;
  setEnabled(value: boolean): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): FriendInfo.AsObject;
  static toObject(includeInstance: boolean, msg: FriendInfo): FriendInfo.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: FriendInfo, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): FriendInfo;
  static deserializeBinaryFromReader(message: FriendInfo, reader: jspb.BinaryReader): FriendInfo;
}

export namespace FriendInfo {
  export type AsObject = {
    uniqueName: string,
    displayName: string,
    enabled: boolean,
  }
}

export class GetFriendListResponse extends jspb.Message {
  clearFriendInfosList(): void;
  getFriendInfosList(): Array<FriendInfo>;
  setFriendInfosList(value: Array<FriendInfo>): void;
  addFriendInfos(value?: FriendInfo, index?: number): FriendInfo;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetFriendListResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetFriendListResponse): GetFriendListResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetFriendListResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetFriendListResponse;
  static deserializeBinaryFromReader(message: GetFriendListResponse, reader: jspb.BinaryReader): GetFriendListResponse;
}

export namespace GetFriendListResponse {
  export type AsObject = {
    friendInfosList: Array<FriendInfo.AsObject>,
  }
}

export class GenerateFriendKeyRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GenerateFriendKeyRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GenerateFriendKeyRequest): GenerateFriendKeyRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GenerateFriendKeyRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GenerateFriendKeyRequest;
  static deserializeBinaryFromReader(message: GenerateFriendKeyRequest, reader: jspb.BinaryReader): GenerateFriendKeyRequest;
}

export namespace GenerateFriendKeyRequest {
  export type AsObject = {
    uniqueName: string,
    displayName: string,
  }
}

export class GenerateFriendKeyResponse extends jspb.Message {
  getKey(): string;
  setKey(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GenerateFriendKeyResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GenerateFriendKeyResponse): GenerateFriendKeyResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GenerateFriendKeyResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GenerateFriendKeyResponse;
  static deserializeBinaryFromReader(message: GenerateFriendKeyResponse, reader: jspb.BinaryReader): GenerateFriendKeyResponse;
}

export namespace GenerateFriendKeyResponse {
  export type AsObject = {
    key: string,
  }
}

export class AddFriendRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getKey(): string;
  setKey(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddFriendRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AddFriendRequest): AddFriendRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddFriendRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddFriendRequest;
  static deserializeBinaryFromReader(message: AddFriendRequest, reader: jspb.BinaryReader): AddFriendRequest;
}

export namespace AddFriendRequest {
  export type AsObject = {
    uniqueName: string,
    key: string,
  }
}

export class AddFriendResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddFriendResponse.AsObject;
  static toObject(includeInstance: boolean, msg: AddFriendResponse): AddFriendResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddFriendResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddFriendResponse;
  static deserializeBinaryFromReader(message: AddFriendResponse, reader: jspb.BinaryReader): AddFriendResponse;
}

export namespace AddFriendResponse {
  export type AsObject = {
  }
}

export class RemoveFriendRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RemoveFriendRequest.AsObject;
  static toObject(includeInstance: boolean, msg: RemoveFriendRequest): RemoveFriendRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RemoveFriendRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RemoveFriendRequest;
  static deserializeBinaryFromReader(message: RemoveFriendRequest, reader: jspb.BinaryReader): RemoveFriendRequest;
}

export namespace RemoveFriendRequest {
  export type AsObject = {
    uniqueName: string,
  }
}

export class RemoveFriendResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RemoveFriendResponse.AsObject;
  static toObject(includeInstance: boolean, msg: RemoveFriendResponse): RemoveFriendResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RemoveFriendResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RemoveFriendResponse;
  static deserializeBinaryFromReader(message: RemoveFriendResponse, reader: jspb.BinaryReader): RemoveFriendResponse;
}

export namespace RemoveFriendResponse {
  export type AsObject = {
  }
}

export class SendMessageRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getMessage(): string;
  setMessage(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): SendMessageRequest.AsObject;
  static toObject(includeInstance: boolean, msg: SendMessageRequest): SendMessageRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: SendMessageRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): SendMessageRequest;
  static deserializeBinaryFromReader(message: SendMessageRequest, reader: jspb.BinaryReader): SendMessageRequest;
}

export namespace SendMessageRequest {
  export type AsObject = {
    uniqueName: string,
    message: string,
  }
}

export class SendMessageResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): SendMessageResponse.AsObject;
  static toObject(includeInstance: boolean, msg: SendMessageResponse): SendMessageResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: SendMessageResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): SendMessageResponse;
  static deserializeBinaryFromReader(message: SendMessageResponse, reader: jspb.BinaryReader): SendMessageResponse;
}

export namespace SendMessageResponse {
  export type AsObject = {
  }
}

export class BaseMessage extends jspb.Message {
  getId(): number;
  setId(value: number): void;

  getMessage(): string;
  setMessage(value: string): void;

  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): BaseMessage.AsObject;
  static toObject(includeInstance: boolean, msg: BaseMessage): BaseMessage.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: BaseMessage, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): BaseMessage;
  static deserializeBinaryFromReader(message: BaseMessage, reader: jspb.BinaryReader): BaseMessage;
}

export namespace BaseMessage {
  export type AsObject = {
    id: number,
    message: string,
    uniqueName: string,
    displayName: string,
  }
}

export class IncomingMessage extends jspb.Message {
  hasM(): boolean;
  clearM(): void;
  getM(): BaseMessage | undefined;
  setM(value?: BaseMessage): void;

  hasDeliveredAt(): boolean;
  clearDeliveredAt(): void;
  getDeliveredAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
  setDeliveredAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

  getSeen(): boolean;
  setSeen(value: boolean): void;

  getDelivered(): boolean;
  setDelivered(value: boolean): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): IncomingMessage.AsObject;
  static toObject(includeInstance: boolean, msg: IncomingMessage): IncomingMessage.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: IncomingMessage, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): IncomingMessage;
  static deserializeBinaryFromReader(message: IncomingMessage, reader: jspb.BinaryReader): IncomingMessage;
}

export namespace IncomingMessage {
  export type AsObject = {
    m?: BaseMessage.AsObject,
    deliveredAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    seen: boolean,
    delivered: boolean,
  }
}

export class OutgoingMessage extends jspb.Message {
  hasM(): boolean;
  clearM(): void;
  getM(): BaseMessage | undefined;
  setM(value?: BaseMessage): void;

  hasSentAt(): boolean;
  clearSentAt(): void;
  getSentAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
  setSentAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

  getDelivered(): boolean;
  setDelivered(value: boolean): void;

  hasDeliveredAt(): boolean;
  clearDeliveredAt(): void;
  getDeliveredAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
  setDeliveredAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): OutgoingMessage.AsObject;
  static toObject(includeInstance: boolean, msg: OutgoingMessage): OutgoingMessage.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: OutgoingMessage, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): OutgoingMessage;
  static deserializeBinaryFromReader(message: OutgoingMessage, reader: jspb.BinaryReader): OutgoingMessage;
}

export namespace OutgoingMessage {
  export type AsObject = {
    m?: BaseMessage.AsObject,
    sentAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    delivered: boolean,
    deliveredAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
  }
}

export class GetMessagesRequest extends jspb.Message {
  getFilter(): GetMessagesRequest.FilterMap[keyof GetMessagesRequest.FilterMap];
  setFilter(value: GetMessagesRequest.FilterMap[keyof GetMessagesRequest.FilterMap]): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetMessagesRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetMessagesRequest): GetMessagesRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetMessagesRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetMessagesRequest;
  static deserializeBinaryFromReader(message: GetMessagesRequest, reader: jspb.BinaryReader): GetMessagesRequest;
}

export namespace GetMessagesRequest {
  export type AsObject = {
    filter: GetMessagesRequest.FilterMap[keyof GetMessagesRequest.FilterMap],
  }

  export interface FilterMap {
    ALL: 0;
    NEW: 1;
  }

  export const Filter: FilterMap;
}

export class GetMessagesResponse extends jspb.Message {
  clearMessagesList(): void;
  getMessagesList(): Array<IncomingMessage>;
  setMessagesList(value: Array<IncomingMessage>): void;
  addMessages(value?: IncomingMessage, index?: number): IncomingMessage;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetMessagesResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetMessagesResponse): GetMessagesResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetMessagesResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetMessagesResponse;
  static deserializeBinaryFromReader(message: GetMessagesResponse, reader: jspb.BinaryReader): GetMessagesResponse;
}

export namespace GetMessagesResponse {
  export type AsObject = {
    messagesList: Array<IncomingMessage.AsObject>,
  }
}

export class GetOutboxMessagesRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutboxMessagesRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutboxMessagesRequest): GetOutboxMessagesRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutboxMessagesRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutboxMessagesRequest;
  static deserializeBinaryFromReader(message: GetOutboxMessagesRequest, reader: jspb.BinaryReader): GetOutboxMessagesRequest;
}

export namespace GetOutboxMessagesRequest {
  export type AsObject = {
  }
}

export class GetOutboxMessagesResponse extends jspb.Message {
  clearMessagesList(): void;
  getMessagesList(): Array<OutgoingMessage>;
  setMessagesList(value: Array<OutgoingMessage>): void;
  addMessages(value?: OutgoingMessage, index?: number): OutgoingMessage;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutboxMessagesResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutboxMessagesResponse): GetOutboxMessagesResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutboxMessagesResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutboxMessagesResponse;
  static deserializeBinaryFromReader(message: GetOutboxMessagesResponse, reader: jspb.BinaryReader): GetOutboxMessagesResponse;
}

export namespace GetOutboxMessagesResponse {
  export type AsObject = {
    messagesList: Array<OutgoingMessage.AsObject>,
  }
}

export class GetSentMessagesRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetSentMessagesRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetSentMessagesRequest): GetSentMessagesRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetSentMessagesRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetSentMessagesRequest;
  static deserializeBinaryFromReader(message: GetSentMessagesRequest, reader: jspb.BinaryReader): GetSentMessagesRequest;
}

export namespace GetSentMessagesRequest {
  export type AsObject = {
  }
}

export class GetSentMessagesResponse extends jspb.Message {
  clearMessagesList(): void;
  getMessagesList(): Array<OutgoingMessage>;
  setMessagesList(value: Array<OutgoingMessage>): void;
  addMessages(value?: OutgoingMessage, index?: number): OutgoingMessage;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetSentMessagesResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetSentMessagesResponse): GetSentMessagesResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetSentMessagesResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetSentMessagesResponse;
  static deserializeBinaryFromReader(message: GetSentMessagesResponse, reader: jspb.BinaryReader): GetSentMessagesResponse;
}

export namespace GetSentMessagesResponse {
  export type AsObject = {
    messagesList: Array<OutgoingMessage.AsObject>,
  }
}

export class MessageSeenRequest extends jspb.Message {
  getId(): number;
  setId(value: number): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): MessageSeenRequest.AsObject;
  static toObject(includeInstance: boolean, msg: MessageSeenRequest): MessageSeenRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: MessageSeenRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): MessageSeenRequest;
  static deserializeBinaryFromReader(message: MessageSeenRequest, reader: jspb.BinaryReader): MessageSeenRequest;
}

export namespace MessageSeenRequest {
  export type AsObject = {
    id: number,
  }
}

export class MessageSeenResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): MessageSeenResponse.AsObject;
  static toObject(includeInstance: boolean, msg: MessageSeenResponse): MessageSeenResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: MessageSeenResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): MessageSeenResponse;
  static deserializeBinaryFromReader(message: MessageSeenResponse, reader: jspb.BinaryReader): MessageSeenResponse;
}

export namespace MessageSeenResponse {
  export type AsObject = {
  }
}

export class GetStatusRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetStatusRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetStatusRequest): GetStatusRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetStatusRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetStatusRequest;
  static deserializeBinaryFromReader(message: GetStatusRequest, reader: jspb.BinaryReader): GetStatusRequest;
}

export namespace GetStatusRequest {
  export type AsObject = {
  }
}

export class GetStatusResponse extends jspb.Message {
  getRegistered(): boolean;
  setRegistered(value: boolean): void;

  getReleaseHash(): string;
  setReleaseHash(value: string): void;

  getLatencySeconds(): number;
  setLatencySeconds(value: number): void;

  getServerAddress(): string;
  setServerAddress(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetStatusResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetStatusResponse): GetStatusResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetStatusResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetStatusResponse;
  static deserializeBinaryFromReader(message: GetStatusResponse, reader: jspb.BinaryReader): GetStatusResponse;
}

export namespace GetStatusResponse {
  export type AsObject = {
    registered: boolean,
    releaseHash: string,
    latencySeconds: number,
    serverAddress: string,
  }
}

export class GetLatencyRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetLatencyRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetLatencyRequest): GetLatencyRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetLatencyRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetLatencyRequest;
  static deserializeBinaryFromReader(message: GetLatencyRequest, reader: jspb.BinaryReader): GetLatencyRequest;
}

export namespace GetLatencyRequest {
  export type AsObject = {
  }
}

export class GetLatencyResponse extends jspb.Message {
  getLatencySeconds(): number;
  setLatencySeconds(value: number): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetLatencyResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetLatencyResponse): GetLatencyResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetLatencyResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetLatencyResponse;
  static deserializeBinaryFromReader(message: GetLatencyResponse, reader: jspb.BinaryReader): GetLatencyResponse;
}

export namespace GetLatencyResponse {
  export type AsObject = {
    latencySeconds: number,
  }
}

export class ChangeLatencyRequest extends jspb.Message {
  getLatencySeconds(): number;
  setLatencySeconds(value: number): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): ChangeLatencyRequest.AsObject;
  static toObject(includeInstance: boolean, msg: ChangeLatencyRequest): ChangeLatencyRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: ChangeLatencyRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): ChangeLatencyRequest;
  static deserializeBinaryFromReader(message: ChangeLatencyRequest, reader: jspb.BinaryReader): ChangeLatencyRequest;
}

export namespace ChangeLatencyRequest {
  export type AsObject = {
    latencySeconds: number,
  }
}

export class ChangeLatencyResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): ChangeLatencyResponse.AsObject;
  static toObject(includeInstance: boolean, msg: ChangeLatencyResponse): ChangeLatencyResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: ChangeLatencyResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): ChangeLatencyResponse;
  static deserializeBinaryFromReader(message: ChangeLatencyResponse, reader: jspb.BinaryReader): ChangeLatencyResponse;
}

export namespace ChangeLatencyResponse {
  export type AsObject = {
  }
}

export class KillRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): KillRequest.AsObject;
  static toObject(includeInstance: boolean, msg: KillRequest): KillRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: KillRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): KillRequest;
  static deserializeBinaryFromReader(message: KillRequest, reader: jspb.BinaryReader): KillRequest;
}

export namespace KillRequest {
  export type AsObject = {
  }
}

export class KillResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): KillResponse.AsObject;
  static toObject(includeInstance: boolean, msg: KillResponse): KillResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: KillResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): KillResponse;
  static deserializeBinaryFromReader(message: KillResponse, reader: jspb.BinaryReader): KillResponse;
}

export namespace KillResponse {
  export type AsObject = {
  }
}
