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

  getPublicId(): string;
  setPublicId(value: string): void;

  getInvitationProgress(): InvitationProgressMap[keyof InvitationProgressMap];
  setInvitationProgress(value: InvitationProgressMap[keyof InvitationProgressMap]): void;

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
    publicId: string,
    invitationProgress: InvitationProgressMap[keyof InvitationProgressMap],
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

export class GetMyPublicIDRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetMyPublicIDRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetMyPublicIDRequest): GetMyPublicIDRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetMyPublicIDRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetMyPublicIDRequest;
  static deserializeBinaryFromReader(message: GetMyPublicIDRequest, reader: jspb.BinaryReader): GetMyPublicIDRequest;
}

export namespace GetMyPublicIDRequest {
  export type AsObject = {
  }
}

export class GetMyPublicIDResponse extends jspb.Message {
  getPublicId(): string;
  setPublicId(value: string): void;

  getStory(): string;
  setStory(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetMyPublicIDResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetMyPublicIDResponse): GetMyPublicIDResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetMyPublicIDResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetMyPublicIDResponse;
  static deserializeBinaryFromReader(message: GetMyPublicIDResponse, reader: jspb.BinaryReader): GetMyPublicIDResponse;
}

export namespace GetMyPublicIDResponse {
  export type AsObject = {
    publicId: string,
    story: string,
  }
}

export class AddSyncFriendRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  getStory(): string;
  setStory(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddSyncFriendRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AddSyncFriendRequest): AddSyncFriendRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddSyncFriendRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddSyncFriendRequest;
  static deserializeBinaryFromReader(message: AddSyncFriendRequest, reader: jspb.BinaryReader): AddSyncFriendRequest;
}

export namespace AddSyncFriendRequest {
  export type AsObject = {
    uniqueName: string,
    displayName: string,
    story: string,
  }
}

export class AddSyncFriendResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddSyncFriendResponse.AsObject;
  static toObject(includeInstance: boolean, msg: AddSyncFriendResponse): AddSyncFriendResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddSyncFriendResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddSyncFriendResponse;
  static deserializeBinaryFromReader(message: AddSyncFriendResponse, reader: jspb.BinaryReader): AddSyncFriendResponse;
}

export namespace AddSyncFriendResponse {
  export type AsObject = {
  }
}

export class AddAsyncFriendRequest extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  getPublicId(): string;
  setPublicId(value: string): void;

  getMessage(): string;
  setMessage(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddAsyncFriendRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AddAsyncFriendRequest): AddAsyncFriendRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddAsyncFriendRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddAsyncFriendRequest;
  static deserializeBinaryFromReader(message: AddAsyncFriendRequest, reader: jspb.BinaryReader): AddAsyncFriendRequest;
}

export namespace AddAsyncFriendRequest {
  export type AsObject = {
    uniqueName: string,
    displayName: string,
    publicId: string,
    message: string,
  }
}

export class AddAsyncFriendResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AddAsyncFriendResponse.AsObject;
  static toObject(includeInstance: boolean, msg: AddAsyncFriendResponse): AddAsyncFriendResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AddAsyncFriendResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AddAsyncFriendResponse;
  static deserializeBinaryFromReader(message: AddAsyncFriendResponse, reader: jspb.BinaryReader): AddAsyncFriendResponse;
}

export namespace AddAsyncFriendResponse {
  export type AsObject = {
  }
}

export class GetOutgoingSyncInvitationsRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutgoingSyncInvitationsRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutgoingSyncInvitationsRequest): GetOutgoingSyncInvitationsRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutgoingSyncInvitationsRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutgoingSyncInvitationsRequest;
  static deserializeBinaryFromReader(message: GetOutgoingSyncInvitationsRequest, reader: jspb.BinaryReader): GetOutgoingSyncInvitationsRequest;
}

export namespace GetOutgoingSyncInvitationsRequest {
  export type AsObject = {
  }
}

export class GetOutgoingSyncInvitationsResponse extends jspb.Message {
  clearInvitationsList(): void;
  getInvitationsList(): Array<GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo>;
  setInvitationsList(value: Array<GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo>): void;
  addInvitations(value?: GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo, index?: number): GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutgoingSyncInvitationsResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutgoingSyncInvitationsResponse): GetOutgoingSyncInvitationsResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutgoingSyncInvitationsResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutgoingSyncInvitationsResponse;
  static deserializeBinaryFromReader(message: GetOutgoingSyncInvitationsResponse, reader: jspb.BinaryReader): GetOutgoingSyncInvitationsResponse;
}

export namespace GetOutgoingSyncInvitationsResponse {
  export type AsObject = {
    invitationsList: Array<GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo.AsObject>,
  }

  export class OutgoingSyncInvitationInfo extends jspb.Message {
    getUniqueName(): string;
    setUniqueName(value: string): void;

    getDisplayName(): string;
    setDisplayName(value: string): void;

    getStory(): string;
    setStory(value: string): void;

    hasSentAt(): boolean;
    clearSentAt(): void;
    getSentAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
    setSentAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): OutgoingSyncInvitationInfo.AsObject;
    static toObject(includeInstance: boolean, msg: OutgoingSyncInvitationInfo): OutgoingSyncInvitationInfo.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: OutgoingSyncInvitationInfo, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): OutgoingSyncInvitationInfo;
    static deserializeBinaryFromReader(message: OutgoingSyncInvitationInfo, reader: jspb.BinaryReader): OutgoingSyncInvitationInfo;
  }

  export namespace OutgoingSyncInvitationInfo {
    export type AsObject = {
      uniqueName: string,
      displayName: string,
      story: string,
      sentAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    }
  }
}

export class GetOutgoingAsyncInvitationsRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutgoingAsyncInvitationsRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutgoingAsyncInvitationsRequest): GetOutgoingAsyncInvitationsRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutgoingAsyncInvitationsRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutgoingAsyncInvitationsRequest;
  static deserializeBinaryFromReader(message: GetOutgoingAsyncInvitationsRequest, reader: jspb.BinaryReader): GetOutgoingAsyncInvitationsRequest;
}

export namespace GetOutgoingAsyncInvitationsRequest {
  export type AsObject = {
  }
}

export class GetOutgoingAsyncInvitationsResponse extends jspb.Message {
  clearInvitationsList(): void;
  getInvitationsList(): Array<GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo>;
  setInvitationsList(value: Array<GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo>): void;
  addInvitations(value?: GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo, index?: number): GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetOutgoingAsyncInvitationsResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetOutgoingAsyncInvitationsResponse): GetOutgoingAsyncInvitationsResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetOutgoingAsyncInvitationsResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetOutgoingAsyncInvitationsResponse;
  static deserializeBinaryFromReader(message: GetOutgoingAsyncInvitationsResponse, reader: jspb.BinaryReader): GetOutgoingAsyncInvitationsResponse;
}

export namespace GetOutgoingAsyncInvitationsResponse {
  export type AsObject = {
    invitationsList: Array<GetOutgoingAsyncInvitationsResponse.OutgoingAsyncInvitationInfo.AsObject>,
  }

  export class OutgoingAsyncInvitationInfo extends jspb.Message {
    getUniqueName(): string;
    setUniqueName(value: string): void;

    getDisplayName(): string;
    setDisplayName(value: string): void;

    getPublicId(): string;
    setPublicId(value: string): void;

    getMessage(): string;
    setMessage(value: string): void;

    hasSentAt(): boolean;
    clearSentAt(): void;
    getSentAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
    setSentAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): OutgoingAsyncInvitationInfo.AsObject;
    static toObject(includeInstance: boolean, msg: OutgoingAsyncInvitationInfo): OutgoingAsyncInvitationInfo.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: OutgoingAsyncInvitationInfo, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): OutgoingAsyncInvitationInfo;
    static deserializeBinaryFromReader(message: OutgoingAsyncInvitationInfo, reader: jspb.BinaryReader): OutgoingAsyncInvitationInfo;
  }

  export namespace OutgoingAsyncInvitationInfo {
    export type AsObject = {
      uniqueName: string,
      displayName: string,
      publicId: string,
      message: string,
      sentAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    }
  }
}

export class GetIncomingAsyncInvitationsRequest extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetIncomingAsyncInvitationsRequest.AsObject;
  static toObject(includeInstance: boolean, msg: GetIncomingAsyncInvitationsRequest): GetIncomingAsyncInvitationsRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetIncomingAsyncInvitationsRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetIncomingAsyncInvitationsRequest;
  static deserializeBinaryFromReader(message: GetIncomingAsyncInvitationsRequest, reader: jspb.BinaryReader): GetIncomingAsyncInvitationsRequest;
}

export namespace GetIncomingAsyncInvitationsRequest {
  export type AsObject = {
  }
}

export class GetIncomingAsyncInvitationsResponse extends jspb.Message {
  clearInvitationsList(): void;
  getInvitationsList(): Array<GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo>;
  setInvitationsList(value: Array<GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo>): void;
  addInvitations(value?: GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo, index?: number): GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GetIncomingAsyncInvitationsResponse.AsObject;
  static toObject(includeInstance: boolean, msg: GetIncomingAsyncInvitationsResponse): GetIncomingAsyncInvitationsResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GetIncomingAsyncInvitationsResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GetIncomingAsyncInvitationsResponse;
  static deserializeBinaryFromReader(message: GetIncomingAsyncInvitationsResponse, reader: jspb.BinaryReader): GetIncomingAsyncInvitationsResponse;
}

export namespace GetIncomingAsyncInvitationsResponse {
  export type AsObject = {
    invitationsList: Array<GetIncomingAsyncInvitationsResponse.IncomingAsyncInvitationInfo.AsObject>,
  }

  export class IncomingAsyncInvitationInfo extends jspb.Message {
    getPublicId(): string;
    setPublicId(value: string): void;

    getMessage(): string;
    setMessage(value: string): void;

    hasReceivedAt(): boolean;
    clearReceivedAt(): void;
    getReceivedAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
    setReceivedAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): IncomingAsyncInvitationInfo.AsObject;
    static toObject(includeInstance: boolean, msg: IncomingAsyncInvitationInfo): IncomingAsyncInvitationInfo.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: IncomingAsyncInvitationInfo, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): IncomingAsyncInvitationInfo;
    static deserializeBinaryFromReader(message: IncomingAsyncInvitationInfo, reader: jspb.BinaryReader): IncomingAsyncInvitationInfo;
  }

  export namespace IncomingAsyncInvitationInfo {
    export type AsObject = {
      publicId: string,
      message: string,
      receivedAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    }
  }
}

export class AcceptAsyncInvitationRequest extends jspb.Message {
  getPublicId(): string;
  setPublicId(value: string): void;

  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AcceptAsyncInvitationRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AcceptAsyncInvitationRequest): AcceptAsyncInvitationRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AcceptAsyncInvitationRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AcceptAsyncInvitationRequest;
  static deserializeBinaryFromReader(message: AcceptAsyncInvitationRequest, reader: jspb.BinaryReader): AcceptAsyncInvitationRequest;
}

export namespace AcceptAsyncInvitationRequest {
  export type AsObject = {
    publicId: string,
    uniqueName: string,
    displayName: string,
  }
}

export class AcceptAsyncInvitationResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AcceptAsyncInvitationResponse.AsObject;
  static toObject(includeInstance: boolean, msg: AcceptAsyncInvitationResponse): AcceptAsyncInvitationResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: AcceptAsyncInvitationResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AcceptAsyncInvitationResponse;
  static deserializeBinaryFromReader(message: AcceptAsyncInvitationResponse, reader: jspb.BinaryReader): AcceptAsyncInvitationResponse;
}

export namespace AcceptAsyncInvitationResponse {
  export type AsObject = {
  }
}

export class RejectAsyncInvitationRequest extends jspb.Message {
  getPublicId(): string;
  setPublicId(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RejectAsyncInvitationRequest.AsObject;
  static toObject(includeInstance: boolean, msg: RejectAsyncInvitationRequest): RejectAsyncInvitationRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RejectAsyncInvitationRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RejectAsyncInvitationRequest;
  static deserializeBinaryFromReader(message: RejectAsyncInvitationRequest, reader: jspb.BinaryReader): RejectAsyncInvitationRequest;
}

export namespace RejectAsyncInvitationRequest {
  export type AsObject = {
    publicId: string,
  }
}

export class RejectAsyncInvitationResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RejectAsyncInvitationResponse.AsObject;
  static toObject(includeInstance: boolean, msg: RejectAsyncInvitationResponse): RejectAsyncInvitationResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: RejectAsyncInvitationResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RejectAsyncInvitationResponse;
  static deserializeBinaryFromReader(message: RejectAsyncInvitationResponse, reader: jspb.BinaryReader): RejectAsyncInvitationResponse;
}

export namespace RejectAsyncInvitationResponse {
  export type AsObject = {
  }
}

export class CancelAsyncInvitationRequest extends jspb.Message {
  getPublicId(): string;
  setPublicId(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): CancelAsyncInvitationRequest.AsObject;
  static toObject(includeInstance: boolean, msg: CancelAsyncInvitationRequest): CancelAsyncInvitationRequest.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: CancelAsyncInvitationRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): CancelAsyncInvitationRequest;
  static deserializeBinaryFromReader(message: CancelAsyncInvitationRequest, reader: jspb.BinaryReader): CancelAsyncInvitationRequest;
}

export namespace CancelAsyncInvitationRequest {
  export type AsObject = {
    publicId: string,
  }
}

export class CancelAsyncInvitationResponse extends jspb.Message {
  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): CancelAsyncInvitationResponse.AsObject;
  static toObject(includeInstance: boolean, msg: CancelAsyncInvitationResponse): CancelAsyncInvitationResponse.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: CancelAsyncInvitationResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): CancelAsyncInvitationResponse;
  static deserializeBinaryFromReader(message: CancelAsyncInvitationResponse, reader: jspb.BinaryReader): CancelAsyncInvitationResponse;
}

export namespace CancelAsyncInvitationResponse {
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
  clearUniqueNameList(): void;
  getUniqueNameList(): Array<string>;
  setUniqueNameList(value: Array<string>): void;
  addUniqueName(value: string, index?: number): string;

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
    uniqueNameList: Array<string>,
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

export class IncomingMaybeFriend extends jspb.Message {
  getPublicId(): string;
  setPublicId(value: string): void;

  hasUniqueName(): boolean;
  clearUniqueName(): void;
  getUniqueName(): string;
  setUniqueName(value: string): void;

  hasDisplayName(): boolean;
  clearDisplayName(): void;
  getDisplayName(): string;
  setDisplayName(value: string): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): IncomingMaybeFriend.AsObject;
  static toObject(includeInstance: boolean, msg: IncomingMaybeFriend): IncomingMaybeFriend.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: IncomingMaybeFriend, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): IncomingMaybeFriend;
  static deserializeBinaryFromReader(message: IncomingMaybeFriend, reader: jspb.BinaryReader): IncomingMaybeFriend;
}

export namespace IncomingMaybeFriend {
  export type AsObject = {
    publicId: string,
    uniqueName: string,
    displayName: string,
  }
}

export class IncomingMessage extends jspb.Message {
  getUid(): number;
  setUid(value: number): void;

  getMessage(): string;
  setMessage(value: string): void;

  getFromUniqueName(): string;
  setFromUniqueName(value: string): void;

  getFromDisplayName(): string;
  setFromDisplayName(value: string): void;

  clearOtherRecipientsList(): void;
  getOtherRecipientsList(): Array<IncomingMaybeFriend>;
  setOtherRecipientsList(value: Array<IncomingMaybeFriend>): void;
  addOtherRecipients(value?: IncomingMaybeFriend, index?: number): IncomingMaybeFriend;

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
    uid: number,
    message: string,
    fromUniqueName: string,
    fromDisplayName: string,
    otherRecipientsList: Array<IncomingMaybeFriend.AsObject>,
    deliveredAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
    seen: boolean,
    delivered: boolean,
  }
}

export class OutgoingFriend extends jspb.Message {
  getUniqueName(): string;
  setUniqueName(value: string): void;

  getDisplayName(): string;
  setDisplayName(value: string): void;

  getDelivered(): boolean;
  setDelivered(value: boolean): void;

  hasDeliveredAt(): boolean;
  clearDeliveredAt(): void;
  getDeliveredAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
  setDeliveredAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): OutgoingFriend.AsObject;
  static toObject(includeInstance: boolean, msg: OutgoingFriend): OutgoingFriend.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: OutgoingFriend, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): OutgoingFriend;
  static deserializeBinaryFromReader(message: OutgoingFriend, reader: jspb.BinaryReader): OutgoingFriend;
}

export namespace OutgoingFriend {
  export type AsObject = {
    uniqueName: string,
    displayName: string,
    delivered: boolean,
    deliveredAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
  }
}

export class OutgoingMessage extends jspb.Message {
  getUid(): number;
  setUid(value: number): void;

  getMessage(): string;
  setMessage(value: string): void;

  clearToFriendsList(): void;
  getToFriendsList(): Array<OutgoingFriend>;
  setToFriendsList(value: Array<OutgoingFriend>): void;
  addToFriends(value?: OutgoingFriend, index?: number): OutgoingFriend;

  hasSentAt(): boolean;
  clearSentAt(): void;
  getSentAt(): google_protobuf_timestamp_pb.Timestamp | undefined;
  setSentAt(value?: google_protobuf_timestamp_pb.Timestamp): void;

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
    uid: number,
    message: string,
    toFriendsList: Array<OutgoingFriend.AsObject>,
    sentAt?: google_protobuf_timestamp_pb.Timestamp.AsObject,
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

export interface InvitationProgressMap {
  OUTGOINGASYNC: 0;
  OUTGOINGSYNC: 1;
  COMPLETE: 2;
}

export const InvitationProgress: InvitationProgressMap;
