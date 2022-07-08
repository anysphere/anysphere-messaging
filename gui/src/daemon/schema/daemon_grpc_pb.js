// @generated

// GENERATED CODE -- DO NOT EDIT!

'use strict';
var grpc = require('@grpc/grpc-js');
var schema_daemon_pb = require('../schema/daemon_pb.js');
var google_protobuf_timestamp_pb = require('google-protobuf/google/protobuf/timestamp_pb.js');

function serialize_asphrdaemon_AcceptAsyncInvitationRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.AcceptAsyncInvitationRequest)) {
    throw new Error('Expected argument of type asphrdaemon.AcceptAsyncInvitationRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AcceptAsyncInvitationRequest(buffer_arg) {
  return schema_daemon_pb.AcceptAsyncInvitationRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AcceptAsyncInvitationResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.AcceptAsyncInvitationResponse)) {
    throw new Error('Expected argument of type asphrdaemon.AcceptAsyncInvitationResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AcceptAsyncInvitationResponse(buffer_arg) {
  return schema_daemon_pb.AcceptAsyncInvitationResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AddAsyncFriendRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.AddAsyncFriendRequest)) {
    throw new Error('Expected argument of type asphrdaemon.AddAsyncFriendRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddAsyncFriendRequest(buffer_arg) {
  return schema_daemon_pb.AddAsyncFriendRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AddAsyncFriendResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.AddAsyncFriendResponse)) {
    throw new Error('Expected argument of type asphrdaemon.AddAsyncFriendResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddAsyncFriendResponse(buffer_arg) {
  return schema_daemon_pb.AddAsyncFriendResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AddSyncFriendRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.AddSyncFriendRequest)) {
    throw new Error('Expected argument of type asphrdaemon.AddSyncFriendRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddSyncFriendRequest(buffer_arg) {
  return schema_daemon_pb.AddSyncFriendRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AddSyncFriendResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.AddSyncFriendResponse)) {
    throw new Error('Expected argument of type asphrdaemon.AddSyncFriendResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddSyncFriendResponse(buffer_arg) {
  return schema_daemon_pb.AddSyncFriendResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_ChangeLatencyRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.ChangeLatencyRequest)) {
    throw new Error('Expected argument of type asphrdaemon.ChangeLatencyRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_ChangeLatencyRequest(buffer_arg) {
  return schema_daemon_pb.ChangeLatencyRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_ChangeLatencyResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.ChangeLatencyResponse)) {
    throw new Error('Expected argument of type asphrdaemon.ChangeLatencyResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_ChangeLatencyResponse(buffer_arg) {
  return schema_daemon_pb.ChangeLatencyResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetFriendListRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetFriendListRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetFriendListRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetFriendListRequest(buffer_arg) {
  return schema_daemon_pb.GetFriendListRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetFriendListResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetFriendListResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetFriendListResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetFriendListResponse(buffer_arg) {
  return schema_daemon_pb.GetFriendListResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetIncomingAsyncInvitationsRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetIncomingAsyncInvitationsRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetIncomingAsyncInvitationsRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetIncomingAsyncInvitationsRequest(buffer_arg) {
  return schema_daemon_pb.GetIncomingAsyncInvitationsRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetIncomingAsyncInvitationsResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetIncomingAsyncInvitationsResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetIncomingAsyncInvitationsResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetIncomingAsyncInvitationsResponse(buffer_arg) {
  return schema_daemon_pb.GetIncomingAsyncInvitationsResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetLatencyRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetLatencyRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetLatencyRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetLatencyRequest(buffer_arg) {
  return schema_daemon_pb.GetLatencyRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetLatencyResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetLatencyResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetLatencyResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetLatencyResponse(buffer_arg) {
  return schema_daemon_pb.GetLatencyResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetMessagesRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetMessagesRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetMessagesRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetMessagesRequest(buffer_arg) {
  return schema_daemon_pb.GetMessagesRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetMessagesResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetMessagesResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetMessagesResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetMessagesResponse(buffer_arg) {
  return schema_daemon_pb.GetMessagesResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetMyPublicIDRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetMyPublicIDRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetMyPublicIDRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetMyPublicIDRequest(buffer_arg) {
  return schema_daemon_pb.GetMyPublicIDRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetMyPublicIDResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetMyPublicIDResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetMyPublicIDResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetMyPublicIDResponse(buffer_arg) {
  return schema_daemon_pb.GetMyPublicIDResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutboxMessagesRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutboxMessagesRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutboxMessagesRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutboxMessagesRequest(buffer_arg) {
  return schema_daemon_pb.GetOutboxMessagesRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutboxMessagesResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutboxMessagesResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutboxMessagesResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutboxMessagesResponse(buffer_arg) {
  return schema_daemon_pb.GetOutboxMessagesResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutgoingAsyncInvitationsRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutgoingAsyncInvitationsRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutgoingAsyncInvitationsRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutgoingAsyncInvitationsRequest(buffer_arg) {
  return schema_daemon_pb.GetOutgoingAsyncInvitationsRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutgoingAsyncInvitationsResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutgoingAsyncInvitationsResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutgoingAsyncInvitationsResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutgoingAsyncInvitationsResponse(buffer_arg) {
  return schema_daemon_pb.GetOutgoingAsyncInvitationsResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutgoingSyncInvitationsRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutgoingSyncInvitationsRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutgoingSyncInvitationsRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutgoingSyncInvitationsRequest(buffer_arg) {
  return schema_daemon_pb.GetOutgoingSyncInvitationsRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetOutgoingSyncInvitationsResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetOutgoingSyncInvitationsResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetOutgoingSyncInvitationsResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetOutgoingSyncInvitationsResponse(buffer_arg) {
  return schema_daemon_pb.GetOutgoingSyncInvitationsResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetSentMessagesRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetSentMessagesRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetSentMessagesRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetSentMessagesRequest(buffer_arg) {
  return schema_daemon_pb.GetSentMessagesRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetSentMessagesResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetSentMessagesResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetSentMessagesResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetSentMessagesResponse(buffer_arg) {
  return schema_daemon_pb.GetSentMessagesResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetStatusRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetStatusRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetStatusRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetStatusRequest(buffer_arg) {
  return schema_daemon_pb.GetStatusRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetStatusResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetStatusResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetStatusResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetStatusResponse(buffer_arg) {
  return schema_daemon_pb.GetStatusResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_KillRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.KillRequest)) {
    throw new Error('Expected argument of type asphrdaemon.KillRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_KillRequest(buffer_arg) {
  return schema_daemon_pb.KillRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_KillResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.KillResponse)) {
    throw new Error('Expected argument of type asphrdaemon.KillResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_KillResponse(buffer_arg) {
  return schema_daemon_pb.KillResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_MessageSeenRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.MessageSeenRequest)) {
    throw new Error('Expected argument of type asphrdaemon.MessageSeenRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_MessageSeenRequest(buffer_arg) {
  return schema_daemon_pb.MessageSeenRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_MessageSeenResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.MessageSeenResponse)) {
    throw new Error('Expected argument of type asphrdaemon.MessageSeenResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_MessageSeenResponse(buffer_arg) {
  return schema_daemon_pb.MessageSeenResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RegisterUserRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.RegisterUserRequest)) {
    throw new Error('Expected argument of type asphrdaemon.RegisterUserRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RegisterUserRequest(buffer_arg) {
  return schema_daemon_pb.RegisterUserRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RegisterUserResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.RegisterUserResponse)) {
    throw new Error('Expected argument of type asphrdaemon.RegisterUserResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RegisterUserResponse(buffer_arg) {
  return schema_daemon_pb.RegisterUserResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RejectAsyncInvitationRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.RejectAsyncInvitationRequest)) {
    throw new Error('Expected argument of type asphrdaemon.RejectAsyncInvitationRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RejectAsyncInvitationRequest(buffer_arg) {
  return schema_daemon_pb.RejectAsyncInvitationRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RejectAsyncInvitationResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.RejectAsyncInvitationResponse)) {
    throw new Error('Expected argument of type asphrdaemon.RejectAsyncInvitationResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RejectAsyncInvitationResponse(buffer_arg) {
  return schema_daemon_pb.RejectAsyncInvitationResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RemoveFriendRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.RemoveFriendRequest)) {
    throw new Error('Expected argument of type asphrdaemon.RemoveFriendRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RemoveFriendRequest(buffer_arg) {
  return schema_daemon_pb.RemoveFriendRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_RemoveFriendResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.RemoveFriendResponse)) {
    throw new Error('Expected argument of type asphrdaemon.RemoveFriendResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_RemoveFriendResponse(buffer_arg) {
  return schema_daemon_pb.RemoveFriendResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_SendMessageRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.SendMessageRequest)) {
    throw new Error('Expected argument of type asphrdaemon.SendMessageRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_SendMessageRequest(buffer_arg) {
  return schema_daemon_pb.SendMessageRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_SendMessageResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.SendMessageResponse)) {
    throw new Error('Expected argument of type asphrdaemon.SendMessageResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_SendMessageResponse(buffer_arg) {
  return schema_daemon_pb.SendMessageResponse.deserializeBinary(new Uint8Array(buffer_arg));
}


var DaemonService = exports.DaemonService = {
  registerUser: {
    path: '/asphrdaemon.Daemon/RegisterUser',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.RegisterUserRequest,
    responseType: schema_daemon_pb.RegisterUserResponse,
    requestSerialize: serialize_asphrdaemon_RegisterUserRequest,
    requestDeserialize: deserialize_asphrdaemon_RegisterUserRequest,
    responseSerialize: serialize_asphrdaemon_RegisterUserResponse,
    responseDeserialize: deserialize_asphrdaemon_RegisterUserResponse,
  },
  getMyPublicID: {
    path: '/asphrdaemon.Daemon/GetMyPublicID',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetMyPublicIDRequest,
    responseType: schema_daemon_pb.GetMyPublicIDResponse,
    requestSerialize: serialize_asphrdaemon_GetMyPublicIDRequest,
    requestDeserialize: deserialize_asphrdaemon_GetMyPublicIDRequest,
    responseSerialize: serialize_asphrdaemon_GetMyPublicIDResponse,
    responseDeserialize: deserialize_asphrdaemon_GetMyPublicIDResponse,
  },
  getFriendList: {
    path: '/asphrdaemon.Daemon/GetFriendList',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetFriendListRequest,
    responseType: schema_daemon_pb.GetFriendListResponse,
    requestSerialize: serialize_asphrdaemon_GetFriendListRequest,
    requestDeserialize: deserialize_asphrdaemon_GetFriendListRequest,
    responseSerialize: serialize_asphrdaemon_GetFriendListResponse,
    responseDeserialize: deserialize_asphrdaemon_GetFriendListResponse,
  },
  removeFriend: {
    path: '/asphrdaemon.Daemon/RemoveFriend',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.RemoveFriendRequest,
    responseType: schema_daemon_pb.RemoveFriendResponse,
    requestSerialize: serialize_asphrdaemon_RemoveFriendRequest,
    requestDeserialize: deserialize_asphrdaemon_RemoveFriendRequest,
    responseSerialize: serialize_asphrdaemon_RemoveFriendResponse,
    responseDeserialize: deserialize_asphrdaemon_RemoveFriendResponse,
  },
  addSyncFriend: {
    path: '/asphrdaemon.Daemon/AddSyncFriend',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.AddSyncFriendRequest,
    responseType: schema_daemon_pb.AddSyncFriendResponse,
    requestSerialize: serialize_asphrdaemon_AddSyncFriendRequest,
    requestDeserialize: deserialize_asphrdaemon_AddSyncFriendRequest,
    responseSerialize: serialize_asphrdaemon_AddSyncFriendResponse,
    responseDeserialize: deserialize_asphrdaemon_AddSyncFriendResponse,
  },
  addAsyncFriend: {
    path: '/asphrdaemon.Daemon/AddAsyncFriend',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.AddAsyncFriendRequest,
    responseType: schema_daemon_pb.AddAsyncFriendResponse,
    requestSerialize: serialize_asphrdaemon_AddAsyncFriendRequest,
    requestDeserialize: deserialize_asphrdaemon_AddAsyncFriendRequest,
    responseSerialize: serialize_asphrdaemon_AddAsyncFriendResponse,
    responseDeserialize: deserialize_asphrdaemon_AddAsyncFriendResponse,
  },
  getOutgoingSyncInvitations: {
    path: '/asphrdaemon.Daemon/GetOutgoingSyncInvitations',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetOutgoingSyncInvitationsRequest,
    responseType: schema_daemon_pb.GetOutgoingSyncInvitationsResponse,
    requestSerialize: serialize_asphrdaemon_GetOutgoingSyncInvitationsRequest,
    requestDeserialize: deserialize_asphrdaemon_GetOutgoingSyncInvitationsRequest,
    responseSerialize: serialize_asphrdaemon_GetOutgoingSyncInvitationsResponse,
    responseDeserialize: deserialize_asphrdaemon_GetOutgoingSyncInvitationsResponse,
  },
  getOutgoingAsyncInvitations: {
    path: '/asphrdaemon.Daemon/GetOutgoingAsyncInvitations',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetOutgoingAsyncInvitationsRequest,
    responseType: schema_daemon_pb.GetOutgoingAsyncInvitationsResponse,
    requestSerialize: serialize_asphrdaemon_GetOutgoingAsyncInvitationsRequest,
    requestDeserialize: deserialize_asphrdaemon_GetOutgoingAsyncInvitationsRequest,
    responseSerialize: serialize_asphrdaemon_GetOutgoingAsyncInvitationsResponse,
    responseDeserialize: deserialize_asphrdaemon_GetOutgoingAsyncInvitationsResponse,
  },
  getIncomingAsyncInvitations: {
    path: '/asphrdaemon.Daemon/GetIncomingAsyncInvitations',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetIncomingAsyncInvitationsRequest,
    responseType: schema_daemon_pb.GetIncomingAsyncInvitationsResponse,
    requestSerialize: serialize_asphrdaemon_GetIncomingAsyncInvitationsRequest,
    requestDeserialize: deserialize_asphrdaemon_GetIncomingAsyncInvitationsRequest,
    responseSerialize: serialize_asphrdaemon_GetIncomingAsyncInvitationsResponse,
    responseDeserialize: deserialize_asphrdaemon_GetIncomingAsyncInvitationsResponse,
  },
  acceptAsyncInvitation: {
    path: '/asphrdaemon.Daemon/AcceptAsyncInvitation',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.AcceptAsyncInvitationRequest,
    responseType: schema_daemon_pb.AcceptAsyncInvitationResponse,
    requestSerialize: serialize_asphrdaemon_AcceptAsyncInvitationRequest,
    requestDeserialize: deserialize_asphrdaemon_AcceptAsyncInvitationRequest,
    responseSerialize: serialize_asphrdaemon_AcceptAsyncInvitationResponse,
    responseDeserialize: deserialize_asphrdaemon_AcceptAsyncInvitationResponse,
  },
  rejectAsyncInvitation: {
    path: '/asphrdaemon.Daemon/RejectAsyncInvitation',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.RejectAsyncInvitationRequest,
    responseType: schema_daemon_pb.RejectAsyncInvitationResponse,
    requestSerialize: serialize_asphrdaemon_RejectAsyncInvitationRequest,
    requestDeserialize: deserialize_asphrdaemon_RejectAsyncInvitationRequest,
    responseSerialize: serialize_asphrdaemon_RejectAsyncInvitationResponse,
    responseDeserialize: deserialize_asphrdaemon_RejectAsyncInvitationResponse,
  },
  sendMessage: {
    path: '/asphrdaemon.Daemon/SendMessage',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.SendMessageRequest,
    responseType: schema_daemon_pb.SendMessageResponse,
    requestSerialize: serialize_asphrdaemon_SendMessageRequest,
    requestDeserialize: deserialize_asphrdaemon_SendMessageRequest,
    responseSerialize: serialize_asphrdaemon_SendMessageResponse,
    responseDeserialize: deserialize_asphrdaemon_SendMessageResponse,
  },
  getMessages: {
    path: '/asphrdaemon.Daemon/GetMessages',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetMessagesRequest,
    responseType: schema_daemon_pb.GetMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetMessagesResponse,
  },
  getMessagesStreamed: {
    path: '/asphrdaemon.Daemon/GetMessagesStreamed',
    requestStream: false,
    responseStream: true,
    requestType: schema_daemon_pb.GetMessagesRequest,
    responseType: schema_daemon_pb.GetMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetMessagesResponse,
  },
  getOutboxMessages: {
    path: '/asphrdaemon.Daemon/GetOutboxMessages',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetOutboxMessagesRequest,
    responseType: schema_daemon_pb.GetOutboxMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetOutboxMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetOutboxMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetOutboxMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetOutboxMessagesResponse,
  },
  getSentMessages: {
    path: '/asphrdaemon.Daemon/GetSentMessages',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetSentMessagesRequest,
    responseType: schema_daemon_pb.GetSentMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetSentMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetSentMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetSentMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetSentMessagesResponse,
  },
  messageSeen: {
    path: '/asphrdaemon.Daemon/MessageSeen',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.MessageSeenRequest,
    responseType: schema_daemon_pb.MessageSeenResponse,
    requestSerialize: serialize_asphrdaemon_MessageSeenRequest,
    requestDeserialize: deserialize_asphrdaemon_MessageSeenRequest,
    responseSerialize: serialize_asphrdaemon_MessageSeenResponse,
    responseDeserialize: deserialize_asphrdaemon_MessageSeenResponse,
  },
  getStatus: {
    path: '/asphrdaemon.Daemon/GetStatus',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetStatusRequest,
    responseType: schema_daemon_pb.GetStatusResponse,
    requestSerialize: serialize_asphrdaemon_GetStatusRequest,
    requestDeserialize: deserialize_asphrdaemon_GetStatusRequest,
    responseSerialize: serialize_asphrdaemon_GetStatusResponse,
    responseDeserialize: deserialize_asphrdaemon_GetStatusResponse,
  },
  getLatency: {
    path: '/asphrdaemon.Daemon/GetLatency',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetLatencyRequest,
    responseType: schema_daemon_pb.GetLatencyResponse,
    requestSerialize: serialize_asphrdaemon_GetLatencyRequest,
    requestDeserialize: deserialize_asphrdaemon_GetLatencyRequest,
    responseSerialize: serialize_asphrdaemon_GetLatencyResponse,
    responseDeserialize: deserialize_asphrdaemon_GetLatencyResponse,
  },
  changeLatency: {
    path: '/asphrdaemon.Daemon/ChangeLatency',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.ChangeLatencyRequest,
    responseType: schema_daemon_pb.ChangeLatencyResponse,
    requestSerialize: serialize_asphrdaemon_ChangeLatencyRequest,
    requestDeserialize: deserialize_asphrdaemon_ChangeLatencyRequest,
    responseSerialize: serialize_asphrdaemon_ChangeLatencyResponse,
    responseDeserialize: deserialize_asphrdaemon_ChangeLatencyResponse,
  },
  kill: {
    path: '/asphrdaemon.Daemon/Kill',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.KillRequest,
    responseType: schema_daemon_pb.KillResponse,
    requestSerialize: serialize_asphrdaemon_KillRequest,
    requestDeserialize: deserialize_asphrdaemon_KillRequest,
    responseSerialize: serialize_asphrdaemon_KillResponse,
    responseDeserialize: deserialize_asphrdaemon_KillResponse,
  },
};

exports.DaemonClient = grpc.makeGenericClientConstructor(DaemonService);
