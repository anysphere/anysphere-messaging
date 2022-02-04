// GENERATED CODE -- DO NOT EDIT!

'use strict';
var grpc = require('@grpc/grpc-js');
var schema_daemon_pb = require('../schema/daemon_pb.js');
var google_protobuf_timestamp_pb = require('google-protobuf/google/protobuf/timestamp_pb.js');

function serialize_asphrdaemon_AddFriendRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.AddFriendRequest)) {
    throw new Error('Expected argument of type asphrdaemon.AddFriendRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddFriendRequest(buffer_arg) {
  return schema_daemon_pb.AddFriendRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_AddFriendResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.AddFriendResponse)) {
    throw new Error('Expected argument of type asphrdaemon.AddFriendResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_AddFriendResponse(buffer_arg) {
  return schema_daemon_pb.AddFriendResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GenerateFriendKeyRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GenerateFriendKeyRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GenerateFriendKeyRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GenerateFriendKeyRequest(buffer_arg) {
  return schema_daemon_pb.GenerateFriendKeyRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GenerateFriendKeyResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GenerateFriendKeyResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GenerateFriendKeyResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GenerateFriendKeyResponse(buffer_arg) {
  return schema_daemon_pb.GenerateFriendKeyResponse.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetAllMessagesRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetAllMessagesRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetAllMessagesRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetAllMessagesRequest(buffer_arg) {
  return schema_daemon_pb.GetAllMessagesRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetAllMessagesResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetAllMessagesResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetAllMessagesResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetAllMessagesResponse(buffer_arg) {
  return schema_daemon_pb.GetAllMessagesResponse.deserializeBinary(new Uint8Array(buffer_arg));
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

function serialize_asphrdaemon_GetNewMessagesRequest(arg) {
  if (!(arg instanceof schema_daemon_pb.GetNewMessagesRequest)) {
    throw new Error('Expected argument of type asphrdaemon.GetNewMessagesRequest');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetNewMessagesRequest(buffer_arg) {
  return schema_daemon_pb.GetNewMessagesRequest.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_asphrdaemon_GetNewMessagesResponse(arg) {
  if (!(arg instanceof schema_daemon_pb.GetNewMessagesResponse)) {
    throw new Error('Expected argument of type asphrdaemon.GetNewMessagesResponse');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_asphrdaemon_GetNewMessagesResponse(buffer_arg) {
  return schema_daemon_pb.GetNewMessagesResponse.deserializeBinary(new Uint8Array(buffer_arg));
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
  generateFriendKey: {
    path: '/asphrdaemon.Daemon/GenerateFriendKey',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GenerateFriendKeyRequest,
    responseType: schema_daemon_pb.GenerateFriendKeyResponse,
    requestSerialize: serialize_asphrdaemon_GenerateFriendKeyRequest,
    requestDeserialize: deserialize_asphrdaemon_GenerateFriendKeyRequest,
    responseSerialize: serialize_asphrdaemon_GenerateFriendKeyResponse,
    responseDeserialize: deserialize_asphrdaemon_GenerateFriendKeyResponse,
  },
  addFriend: {
    path: '/asphrdaemon.Daemon/AddFriend',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.AddFriendRequest,
    responseType: schema_daemon_pb.AddFriendResponse,
    requestSerialize: serialize_asphrdaemon_AddFriendRequest,
    requestDeserialize: deserialize_asphrdaemon_AddFriendRequest,
    responseSerialize: serialize_asphrdaemon_AddFriendResponse,
    responseDeserialize: deserialize_asphrdaemon_AddFriendResponse,
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
  getAllMessages: {
    path: '/asphrdaemon.Daemon/GetAllMessages',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetAllMessagesRequest,
    responseType: schema_daemon_pb.GetAllMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetAllMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetAllMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetAllMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetAllMessagesResponse,
  },
  getNewMessages: {
    path: '/asphrdaemon.Daemon/GetNewMessages',
    requestStream: false,
    responseStream: false,
    requestType: schema_daemon_pb.GetNewMessagesRequest,
    responseType: schema_daemon_pb.GetNewMessagesResponse,
    requestSerialize: serialize_asphrdaemon_GetNewMessagesRequest,
    requestDeserialize: deserialize_asphrdaemon_GetNewMessagesRequest,
    responseSerialize: serialize_asphrdaemon_GetNewMessagesResponse,
    responseDeserialize: deserialize_asphrdaemon_GetNewMessagesResponse,
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
};

exports.DaemonClient = grpc.makeGenericClientConstructor(DaemonService);
