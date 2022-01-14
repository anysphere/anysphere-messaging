// GENERATED CODE -- DO NOT EDIT!

// package: asphrdaemon
// file: schema/daemon.proto

import * as schema_daemon_pb from "../schema/daemon_pb";
import * as grpc from "@grpc/grpc-js";

interface IDaemonService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
  registerUser: grpc.MethodDefinition<schema_daemon_pb.RegisterUserRequest, schema_daemon_pb.RegisterUserResponse>;
  getFriendList: grpc.MethodDefinition<schema_daemon_pb.GetFriendListRequest, schema_daemon_pb.GetFriendListResponse>;
  generateFriendKey: grpc.MethodDefinition<schema_daemon_pb.GenerateFriendKeyRequest, schema_daemon_pb.GenerateFriendKeyResponse>;
  addFriend: grpc.MethodDefinition<schema_daemon_pb.AddFriendRequest, schema_daemon_pb.AddFriendResponse>;
  removeFriend: grpc.MethodDefinition<schema_daemon_pb.RemoveFriendRequest, schema_daemon_pb.RemoveFriendResponse>;
  sendMessage: grpc.MethodDefinition<schema_daemon_pb.SendMessageRequest, schema_daemon_pb.SendMessageResponse>;
  getAllMessages: grpc.MethodDefinition<schema_daemon_pb.GetAllMessagesRequest, schema_daemon_pb.GetAllMessagesResponse>;
  getNewMessages: grpc.MethodDefinition<schema_daemon_pb.GetNewMessagesRequest, schema_daemon_pb.GetNewMessagesResponse>;
}

export const DaemonService: IDaemonService;

export interface IDaemonServer extends grpc.UntypedServiceImplementation {
  registerUser: grpc.handleUnaryCall<schema_daemon_pb.RegisterUserRequest, schema_daemon_pb.RegisterUserResponse>;
  getFriendList: grpc.handleUnaryCall<schema_daemon_pb.GetFriendListRequest, schema_daemon_pb.GetFriendListResponse>;
  generateFriendKey: grpc.handleUnaryCall<schema_daemon_pb.GenerateFriendKeyRequest, schema_daemon_pb.GenerateFriendKeyResponse>;
  addFriend: grpc.handleUnaryCall<schema_daemon_pb.AddFriendRequest, schema_daemon_pb.AddFriendResponse>;
  removeFriend: grpc.handleUnaryCall<schema_daemon_pb.RemoveFriendRequest, schema_daemon_pb.RemoveFriendResponse>;
  sendMessage: grpc.handleUnaryCall<schema_daemon_pb.SendMessageRequest, schema_daemon_pb.SendMessageResponse>;
  getAllMessages: grpc.handleUnaryCall<schema_daemon_pb.GetAllMessagesRequest, schema_daemon_pb.GetAllMessagesResponse>;
  getNewMessages: grpc.handleUnaryCall<schema_daemon_pb.GetNewMessagesRequest, schema_daemon_pb.GetNewMessagesResponse>;
}

export class DaemonClient extends grpc.Client {
  constructor(address: string, credentials: grpc.ChannelCredentials, options?: object);
  registerUser(argument: schema_daemon_pb.RegisterUserRequest, callback: grpc.requestCallback<schema_daemon_pb.RegisterUserResponse>): grpc.ClientUnaryCall;
  registerUser(argument: schema_daemon_pb.RegisterUserRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RegisterUserResponse>): grpc.ClientUnaryCall;
  registerUser(argument: schema_daemon_pb.RegisterUserRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RegisterUserResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  generateFriendKey(argument: schema_daemon_pb.GenerateFriendKeyRequest, callback: grpc.requestCallback<schema_daemon_pb.GenerateFriendKeyResponse>): grpc.ClientUnaryCall;
  generateFriendKey(argument: schema_daemon_pb.GenerateFriendKeyRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GenerateFriendKeyResponse>): grpc.ClientUnaryCall;
  generateFriendKey(argument: schema_daemon_pb.GenerateFriendKeyRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GenerateFriendKeyResponse>): grpc.ClientUnaryCall;
  addFriend(argument: schema_daemon_pb.AddFriendRequest, callback: grpc.requestCallback<schema_daemon_pb.AddFriendResponse>): grpc.ClientUnaryCall;
  addFriend(argument: schema_daemon_pb.AddFriendRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddFriendResponse>): grpc.ClientUnaryCall;
  addFriend(argument: schema_daemon_pb.AddFriendRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddFriendResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  getAllMessages(argument: schema_daemon_pb.GetAllMessagesRequest, callback: grpc.requestCallback<schema_daemon_pb.GetAllMessagesResponse>): grpc.ClientUnaryCall;
  getAllMessages(argument: schema_daemon_pb.GetAllMessagesRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetAllMessagesResponse>): grpc.ClientUnaryCall;
  getAllMessages(argument: schema_daemon_pb.GetAllMessagesRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetAllMessagesResponse>): grpc.ClientUnaryCall;
  getNewMessages(argument: schema_daemon_pb.GetNewMessagesRequest, callback: grpc.requestCallback<schema_daemon_pb.GetNewMessagesResponse>): grpc.ClientUnaryCall;
  getNewMessages(argument: schema_daemon_pb.GetNewMessagesRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetNewMessagesResponse>): grpc.ClientUnaryCall;
  getNewMessages(argument: schema_daemon_pb.GetNewMessagesRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetNewMessagesResponse>): grpc.ClientUnaryCall;
}
