// @generated

// GENERATED CODE -- DO NOT EDIT!

// package: asphrdaemon
// file: schema/daemon.proto

import * as schema_daemon_pb from "../schema/daemon_pb";
import * as grpc from "@grpc/grpc-js";

interface IDaemonService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
  registerUser: grpc.MethodDefinition<schema_daemon_pb.RegisterUserRequest, schema_daemon_pb.RegisterUserResponse>;
  getMyPublicID: grpc.MethodDefinition<schema_daemon_pb.GetMyPublicIDRequest, schema_daemon_pb.GetMyPublicIDResponse>;
  isValidPublicID: grpc.MethodDefinition<schema_daemon_pb.IsValidPublicIDRequest, schema_daemon_pb.IsValidPublicIDResponse>;
  getFriendList: grpc.MethodDefinition<schema_daemon_pb.GetFriendListRequest, schema_daemon_pb.GetFriendListResponse>;
  removeFriend: grpc.MethodDefinition<schema_daemon_pb.RemoveFriendRequest, schema_daemon_pb.RemoveFriendResponse>;
  addSyncFriend: grpc.MethodDefinition<schema_daemon_pb.AddSyncFriendRequest, schema_daemon_pb.AddSyncFriendResponse>;
  addAsyncFriend: grpc.MethodDefinition<schema_daemon_pb.AddAsyncFriendRequest, schema_daemon_pb.AddAsyncFriendResponse>;
  getOutgoingSyncInvitations: grpc.MethodDefinition<schema_daemon_pb.GetOutgoingSyncInvitationsRequest, schema_daemon_pb.GetOutgoingSyncInvitationsResponse>;
  getOutgoingAsyncInvitations: grpc.MethodDefinition<schema_daemon_pb.GetOutgoingAsyncInvitationsRequest, schema_daemon_pb.GetOutgoingAsyncInvitationsResponse>;
  getIncomingAsyncInvitations: grpc.MethodDefinition<schema_daemon_pb.GetIncomingAsyncInvitationsRequest, schema_daemon_pb.GetIncomingAsyncInvitationsResponse>;
  acceptAsyncInvitation: grpc.MethodDefinition<schema_daemon_pb.AcceptAsyncInvitationRequest, schema_daemon_pb.AcceptAsyncInvitationResponse>;
  rejectAsyncInvitation: grpc.MethodDefinition<schema_daemon_pb.RejectAsyncInvitationRequest, schema_daemon_pb.RejectAsyncInvitationResponse>;
  cancelAsyncInvitation: grpc.MethodDefinition<schema_daemon_pb.CancelAsyncInvitationRequest, schema_daemon_pb.CancelAsyncInvitationResponse>;
  sendMessage: grpc.MethodDefinition<schema_daemon_pb.SendMessageRequest, schema_daemon_pb.SendMessageResponse>;
  getMessages: grpc.MethodDefinition<schema_daemon_pb.GetMessagesRequest, schema_daemon_pb.GetMessagesResponse>;
  getMessagesStreamed: grpc.MethodDefinition<schema_daemon_pb.GetMessagesRequest, schema_daemon_pb.GetMessagesResponse>;
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
  getMyPublicID: grpc.handleUnaryCall<schema_daemon_pb.GetMyPublicIDRequest, schema_daemon_pb.GetMyPublicIDResponse>;
  isValidPublicID: grpc.handleUnaryCall<schema_daemon_pb.IsValidPublicIDRequest, schema_daemon_pb.IsValidPublicIDResponse>;
  getFriendList: grpc.handleUnaryCall<schema_daemon_pb.GetFriendListRequest, schema_daemon_pb.GetFriendListResponse>;
  removeFriend: grpc.handleUnaryCall<schema_daemon_pb.RemoveFriendRequest, schema_daemon_pb.RemoveFriendResponse>;
  addSyncFriend: grpc.handleUnaryCall<schema_daemon_pb.AddSyncFriendRequest, schema_daemon_pb.AddSyncFriendResponse>;
  addAsyncFriend: grpc.handleUnaryCall<schema_daemon_pb.AddAsyncFriendRequest, schema_daemon_pb.AddAsyncFriendResponse>;
  getOutgoingSyncInvitations: grpc.handleUnaryCall<schema_daemon_pb.GetOutgoingSyncInvitationsRequest, schema_daemon_pb.GetOutgoingSyncInvitationsResponse>;
  getOutgoingAsyncInvitations: grpc.handleUnaryCall<schema_daemon_pb.GetOutgoingAsyncInvitationsRequest, schema_daemon_pb.GetOutgoingAsyncInvitationsResponse>;
  getIncomingAsyncInvitations: grpc.handleUnaryCall<schema_daemon_pb.GetIncomingAsyncInvitationsRequest, schema_daemon_pb.GetIncomingAsyncInvitationsResponse>;
  acceptAsyncInvitation: grpc.handleUnaryCall<schema_daemon_pb.AcceptAsyncInvitationRequest, schema_daemon_pb.AcceptAsyncInvitationResponse>;
  rejectAsyncInvitation: grpc.handleUnaryCall<schema_daemon_pb.RejectAsyncInvitationRequest, schema_daemon_pb.RejectAsyncInvitationResponse>;
  cancelAsyncInvitation: grpc.handleUnaryCall<schema_daemon_pb.CancelAsyncInvitationRequest, schema_daemon_pb.CancelAsyncInvitationResponse>;
  sendMessage: grpc.handleUnaryCall<schema_daemon_pb.SendMessageRequest, schema_daemon_pb.SendMessageResponse>;
  getMessages: grpc.handleUnaryCall<schema_daemon_pb.GetMessagesRequest, schema_daemon_pb.GetMessagesResponse>;
  getMessagesStreamed: grpc.handleServerStreamingCall<schema_daemon_pb.GetMessagesRequest, schema_daemon_pb.GetMessagesResponse>;
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
  getMyPublicID(argument: schema_daemon_pb.GetMyPublicIDRequest, callback: grpc.requestCallback<schema_daemon_pb.GetMyPublicIDResponse>): grpc.ClientUnaryCall;
  getMyPublicID(argument: schema_daemon_pb.GetMyPublicIDRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetMyPublicIDResponse>): grpc.ClientUnaryCall;
  getMyPublicID(argument: schema_daemon_pb.GetMyPublicIDRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetMyPublicIDResponse>): grpc.ClientUnaryCall;
  isValidPublicID(argument: schema_daemon_pb.IsValidPublicIDRequest, callback: grpc.requestCallback<schema_daemon_pb.IsValidPublicIDResponse>): grpc.ClientUnaryCall;
  isValidPublicID(argument: schema_daemon_pb.IsValidPublicIDRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.IsValidPublicIDResponse>): grpc.ClientUnaryCall;
  isValidPublicID(argument: schema_daemon_pb.IsValidPublicIDRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.IsValidPublicIDResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  getFriendList(argument: schema_daemon_pb.GetFriendListRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetFriendListResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  removeFriend(argument: schema_daemon_pb.RemoveFriendRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RemoveFriendResponse>): grpc.ClientUnaryCall;
  addSyncFriend(argument: schema_daemon_pb.AddSyncFriendRequest, callback: grpc.requestCallback<schema_daemon_pb.AddSyncFriendResponse>): grpc.ClientUnaryCall;
  addSyncFriend(argument: schema_daemon_pb.AddSyncFriendRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddSyncFriendResponse>): grpc.ClientUnaryCall;
  addSyncFriend(argument: schema_daemon_pb.AddSyncFriendRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddSyncFriendResponse>): grpc.ClientUnaryCall;
  addAsyncFriend(argument: schema_daemon_pb.AddAsyncFriendRequest, callback: grpc.requestCallback<schema_daemon_pb.AddAsyncFriendResponse>): grpc.ClientUnaryCall;
  addAsyncFriend(argument: schema_daemon_pb.AddAsyncFriendRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddAsyncFriendResponse>): grpc.ClientUnaryCall;
  addAsyncFriend(argument: schema_daemon_pb.AddAsyncFriendRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AddAsyncFriendResponse>): grpc.ClientUnaryCall;
  getOutgoingSyncInvitations(argument: schema_daemon_pb.GetOutgoingSyncInvitationsRequest, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingSyncInvitationsResponse>): grpc.ClientUnaryCall;
  getOutgoingSyncInvitations(argument: schema_daemon_pb.GetOutgoingSyncInvitationsRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingSyncInvitationsResponse>): grpc.ClientUnaryCall;
  getOutgoingSyncInvitations(argument: schema_daemon_pb.GetOutgoingSyncInvitationsRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingSyncInvitationsResponse>): grpc.ClientUnaryCall;
  getOutgoingAsyncInvitations(argument: schema_daemon_pb.GetOutgoingAsyncInvitationsRequest, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  getOutgoingAsyncInvitations(argument: schema_daemon_pb.GetOutgoingAsyncInvitationsRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  getOutgoingAsyncInvitations(argument: schema_daemon_pb.GetOutgoingAsyncInvitationsRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetOutgoingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  getIncomingAsyncInvitations(argument: schema_daemon_pb.GetIncomingAsyncInvitationsRequest, callback: grpc.requestCallback<schema_daemon_pb.GetIncomingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  getIncomingAsyncInvitations(argument: schema_daemon_pb.GetIncomingAsyncInvitationsRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetIncomingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  getIncomingAsyncInvitations(argument: schema_daemon_pb.GetIncomingAsyncInvitationsRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetIncomingAsyncInvitationsResponse>): grpc.ClientUnaryCall;
  acceptAsyncInvitation(argument: schema_daemon_pb.AcceptAsyncInvitationRequest, callback: grpc.requestCallback<schema_daemon_pb.AcceptAsyncInvitationResponse>): grpc.ClientUnaryCall;
  acceptAsyncInvitation(argument: schema_daemon_pb.AcceptAsyncInvitationRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AcceptAsyncInvitationResponse>): grpc.ClientUnaryCall;
  acceptAsyncInvitation(argument: schema_daemon_pb.AcceptAsyncInvitationRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.AcceptAsyncInvitationResponse>): grpc.ClientUnaryCall;
  rejectAsyncInvitation(argument: schema_daemon_pb.RejectAsyncInvitationRequest, callback: grpc.requestCallback<schema_daemon_pb.RejectAsyncInvitationResponse>): grpc.ClientUnaryCall;
  rejectAsyncInvitation(argument: schema_daemon_pb.RejectAsyncInvitationRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RejectAsyncInvitationResponse>): grpc.ClientUnaryCall;
  rejectAsyncInvitation(argument: schema_daemon_pb.RejectAsyncInvitationRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.RejectAsyncInvitationResponse>): grpc.ClientUnaryCall;
  cancelAsyncInvitation(argument: schema_daemon_pb.CancelAsyncInvitationRequest, callback: grpc.requestCallback<schema_daemon_pb.CancelAsyncInvitationResponse>): grpc.ClientUnaryCall;
  cancelAsyncInvitation(argument: schema_daemon_pb.CancelAsyncInvitationRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.CancelAsyncInvitationResponse>): grpc.ClientUnaryCall;
  cancelAsyncInvitation(argument: schema_daemon_pb.CancelAsyncInvitationRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.CancelAsyncInvitationResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  sendMessage(argument: schema_daemon_pb.SendMessageRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.SendMessageResponse>): grpc.ClientUnaryCall;
  getMessages(argument: schema_daemon_pb.GetMessagesRequest, callback: grpc.requestCallback<schema_daemon_pb.GetMessagesResponse>): grpc.ClientUnaryCall;
  getMessages(argument: schema_daemon_pb.GetMessagesRequest, metadataOrOptions: grpc.Metadata | grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetMessagesResponse>): grpc.ClientUnaryCall;
  getMessages(argument: schema_daemon_pb.GetMessagesRequest, metadata: grpc.Metadata | null, options: grpc.CallOptions | null, callback: grpc.requestCallback<schema_daemon_pb.GetMessagesResponse>): grpc.ClientUnaryCall;
  getMessagesStreamed(argument: schema_daemon_pb.GetMessagesRequest, metadataOrOptions?: grpc.Metadata | grpc.CallOptions | null): grpc.ClientReadableStream<schema_daemon_pb.GetMessagesResponse>;
  getMessagesStreamed(argument: schema_daemon_pb.GetMessagesRequest, metadata?: grpc.Metadata | null, options?: grpc.CallOptions | null): grpc.ClientReadableStream<schema_daemon_pb.GetMessagesResponse>;
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
