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
  getOutboxMessages: grpc.MethodDefinition<schema_daemon_pb.GetOutboxMessagesRequest, schema_daemon_pb.GetOutboxMessagesResponse>;
  getSentMessages: grpc.MethodDefinition<schema_daemon_pb.GetSentMessagesRequest, schema_daemon_pb.GetSentMessagesResponse>;
  messageSeen: grpc.MethodDefinition<schema_daemon_pb.MessageSeenRequest, schema_daemon_pb.MessageSeenResponse>;
  getStatus: grpc.MethodDefinition<schema_daemon_pb.GetStatusRequest, schema_daemon_pb.GetStatusResponse>;
  getLatency: grpc.MethodDefinition<schema_daemon_pb.GetLatencyRequest, schema_daemon_pb.GetLatencyResponse>;
  changeLatency: grpc.MethodDefinition<schema_daemon_pb.ChangeLatencyRequest, schema_daemon_pb.ChangeLatencyResponse>;
  kill: grpc.MethodDefinition<schema_daemon_pb.KillRequest, schema_daemon_pb.KillResponse>;
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
  getOutboxMessages: grpc.handleUnaryCall<schema_daemon_pb.GetOutboxMessagesRequest, schema_daemon_pb.GetOutboxMessagesResponse>;
  getSentMessages: grpc.handleUnaryCall<schema_daemon_pb.GetSentMessagesRequest, schema_daemon_pb.GetSentMessagesResponse>;
  messageSeen: grpc.handleUnaryCall<schema_daemon_pb.MessageSeenRequest, schema_daemon_pb.MessageSeenResponse>;
  getStatus: grpc.handleUnaryCall<schema_daemon_pb.GetStatusRequest, schema_daemon_pb.GetStatusResponse>;
  getLatency: grpc.handleUnaryCall<schema_daemon_pb.GetLatencyRequest, schema_daemon_pb.GetLatencyResponse>;
  changeLatency: grpc.handleUnaryCall<schema_daemon_pb.ChangeLatencyRequest, schema_daemon_pb.ChangeLatencyResponse>;
  kill: grpc.handleUnaryCall<schema_daemon_pb.KillRequest, schema_daemon_pb.KillResponse>;
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
  getOutboxMessages(argument: schema_daemon_pb.GetOutboxMessagesRequest, callback: grpc.requestCallback<schema_daemon_pb.GetOutboxMessagesResponse>): grpc.ClientUnaryCall;
  getOutboxMessages(argument: schema_daemon_pb.GetOutboxMessagesRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutboxMessagesResponse>): grpc.ClientUnaryCall;
  getOutboxMessages(argument: schema_daemon_pb.GetOutboxMessagesRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutboxMessagesResponse>): grpc.ClientUnaryCall;
  getSentMessages(argument: schema_daemon_pb.GetSentMessagesRequest, callback: grpc.requestCallback<schema_daemon_pb.GetSentMessagesResponse>): grpc.ClientUnaryCall;
  getSentMessages(argument: schema_daemon_pb.GetSentMessagesRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetSentMessagesResponse>): grpc.ClientUnaryCall;
  getSentMessages(argument: schema_daemon_pb.GetSentMessagesRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetSentMessagesResponse>): grpc.ClientUnaryCall;
  messageSeen(argument: schema_daemon_pb.MessageSeenRequest, callback: grpc.requestCallback<schema_daemon_pb.MessageSeenResponse>): grpc.ClientUnaryCall;
  messageSeen(argument: schema_daemon_pb.MessageSeenRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.MessageSeenResponse>): grpc.ClientUnaryCall;
  messageSeen(argument: schema_daemon_pb.MessageSeenRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.MessageSeenResponse>): grpc.ClientUnaryCall;
  getStatus(argument: schema_daemon_pb.GetStatusRequest, callback: grpc.requestCallback<schema_daemon_pb.GetStatusResponse>): grpc.ClientUnaryCall;
  getStatus(argument: schema_daemon_pb.GetStatusRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetStatusResponse>): grpc.ClientUnaryCall;
  getStatus(argument: schema_daemon_pb.GetStatusRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetStatusResponse>): grpc.ClientUnaryCall;
  getLatency(argument: schema_daemon_pb.GetLatencyRequest, callback: grpc.requestCallback<schema_daemon_pb.GetLatencyResponse>): grpc.ClientUnaryCall;
  getLatency(argument: schema_daemon_pb.GetLatencyRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetLatencyResponse>): grpc.ClientUnaryCall;
  getLatency(argument: schema_daemon_pb.GetLatencyRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetLatencyResponse>): grpc.ClientUnaryCall;
  changeLatency(argument: schema_daemon_pb.ChangeLatencyRequest, callback: grpc.requestCallback<schema_daemon_pb.ChangeLatencyResponse>): grpc.ClientUnaryCall;
  changeLatency(argument: schema_daemon_pb.ChangeLatencyRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.ChangeLatencyResponse>): grpc.ClientUnaryCall;
  changeLatency(argument: schema_daemon_pb.ChangeLatencyRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.ChangeLatencyResponse>): grpc.ClientUnaryCall;
  kill(argument: schema_daemon_pb.KillRequest, callback: grpc.requestCallback<schema_daemon_pb.KillResponse>): grpc.ClientUnaryCall;
  kill(argument: schema_daemon_pb.KillRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.KillResponse>): grpc.ClientUnaryCall;
  kill(argument: schema_daemon_pb.KillRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.KillResponse>): grpc.ClientUnaryCall;
}
