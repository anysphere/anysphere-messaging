//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

const { contextBridge, clipboard } = require("electron");
const { promisify } = require("util");
var grpc = require("@grpc/grpc-js");
const daemonM = require("../daemon/schema/daemon_pb");
const daemonS = require("../daemon/schema/daemon_grpc_pb");
const path = require("path");

function get_socket_path() {
  if (process.env.XDG_RUNTIME_DIR) {
    return path.join(
      process.env.XDG_RUNTIME_DIR,
      "anysphere",
      "anysphere.sock"
    );
  } else if (process.env.XDG_CONFIG_HOME) {
    return path.join(
      process.env.XDG_CONFIG_HOME,
      "anysphere",
      "run",
      "anysphere.sock"
    );
  } else if (process.env.HOME) {
    return path.join(process.env.HOME, ".anysphere", "run", "anysphere.sock");
  } else {
    process.stderr(
      "$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set! Cannot find socket, aborting. :("
    );
    throw new Error("$HOME or $XDG_CONFIG_HOME or $XDG_RUNTIME_DIR not set!");
  }
}

function get_socket_address() {
  return "unix://" + get_socket_path();
}

const daemonClient = new daemonS.DaemonClient(
  get_socket_address(),
  grpc.credentials.createInsecure()
);

console.log(`SOCKET ADDRESS: ${get_socket_address()}`);

const FAKE_DATA = process.env.ASPHR_FAKE_DATA === "true";

contextBridge.exposeInMainWorld("send", async (message, to) => {
  if (FAKE_DATA) {
    return true;
  }
  const request = new daemonM.SendMessageRequest();
  request.setName(to);
  request.setMessage(message);
  const sendMessage = promisify(daemonClient.sendMessage).bind(daemonClient);
  try {
    const response = await sendMessage(request);
    return true;
  } catch (e) {
    console.log(`error in send: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld("copyToClipboard", async (s) => {
  clipboard.writeText(s, "selection");
});

function convertProtobufMessageToTypedMessage(m) {
  console.log("seconds", m.getReceivedTimestamp().getSeconds());
  var d = new Date(
    m.getReceivedTimestamp().getSeconds() * 1e3 +
      m.getReceivedTimestamp().getNanos() / 1e6
  );
  return {
    id: m.getM().getId(),
    from: m.getFrom(),
    to: "me",
    message: m.getM().getMessage(),
    timestamp: d,
  };
}

contextBridge.exposeInMainWorld("getNewMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "Alice",
        to: "me",
        message: "hello!\n\nthis is a test message\n\nbest,\nalice",
        timestamp: new Date(),
      },
      {
        id: "2",
        from: "Bob",
        to: "me",
        message: "hi",
        timestamp: new Date(),
      },
    ];
  }
  const request = new daemonM.GetNewMessagesRequest();
  const getNewMessages = promisify(daemonClient.getNewMessages).bind(
    daemonClient
  );
  try {
    const response = await getNewMessages(request);
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getNewMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld(
  "generateFriendKey",
  async (requestedFriend) => {
    if (FAKE_DATA) {
      return {
        friend: "sualeh",
        key: "6aFLPa03ldA9OyY-XlCRibbo3SG8Wsprw1iylnjvZIiFc",
      };
    }
    const request = new daemonM.GenerateFriendKeyRequest();
    request.setName(requestedFriend);
    const generateFriendKey = promisify(daemonClient.generateFriendKey).bind(
      daemonClient
    );
    try {
      const response = await generateFriendKey(request);
      return {
        friend: requestedFriend,
        key: response.getKey(),
      };
    } catch (e) {
      console.log(`error in generateFriendKey: ${e}`);
      return [];
    }
  }
);

contextBridge.exposeInMainWorld(
  "addFriend",
  async (requestedFriend, requestedFriendKey) => {
    const request = new daemonM.AddFriendRequest();
    request.setName(requestedFriend);
    request.setKey(requestedFriendKey);
    const addFriend = promisify(daemonClient.addFriend).bind(daemonClient);
    try {
      const response = await addFriend(request);
      return true;
    } catch (e) {
      console.log(`error in addFriend: ${e}`);
      return e;
    }
  }
);

contextBridge.exposeInMainWorld("getAllMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "Alice",
        to: "me",
        message: "hello",
        timestamp: new Date(),
      },
      {
        id: "2",
        from: "Bob",
        to: "me",
        message: "hi",
        timestamp: new Date(),
      },
      {
        id: "3",
        from: "Bob",
        to: "me",
        message: "hi this is my second message",
        timestamp: new Date(),
      },
      {
        id: "4",
        from: "Bob",
        to: "me",
        message: "hi this is my first message",
        timestamp: new Date(),
      },
    ];
  }
  const request = new daemonM.GetAllMessagesRequest();
  const getAllMessages = promisify(daemonClient.getAllMessages).bind(
    daemonClient
  );
  try {
    const response = await getAllMessages(request);
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getAllMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("getAllMessagesStreamed", async (f) => {
  const request = new daemonM.GetMessagesRequest();
  var call = daemonClient.getAllMessagesStreamed(request);
  call.on("data", function (r) {
    try {
      const lm = r.getMessagesList();
      const l = lm.map(convertProtobufMessageToTypedMessage);
      f(l);
    } catch (e) {
      console.log(`error in getAllMessagesStreamed: ${e}`);
    }
    console.log("got all messages streamed", r);
  });
  call.on("end", function () {
    // The server has finished sending
    console.log("getAllMessagesStreamed end");
  });
  call.on("error", function (e) {
    // An error has occurred and the stream has been closed.
    console.log("getAllMessagesStreamed error", e);
  });
  call.on("status", function (status) {
    // process status
    console.log("getAllMessagesStreamed status", status);
  });
  return call.cancel;
});

contextBridge.exposeInMainWorld("getOutboxMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "SuAlEh",
        to: "me",
        message: "hello!\n\nthis is a test message\n\nnot the best,\nSuAlEh",
        timestamp: new Date(),
      },
      {
        id: "2",
        from: "HI",
        to: "me",
        message: "HIHI",
        timestamp: new Date(),
      },
      {
        id: "3",
        from: "Bob",
        to: "me",
        message: "hi this is my second message",
        timestamp: new Date(),
      },
      {
        id: "4",
        from: "Bob",
        to: "me",
        message: "hi this is my first message",
        timestamp: new Date(),
      },
    ];
  }
  const request = new daemonM.GetOutboxMessagesRequest();
  const getOutboxMessages = promisify(daemonClient.getOutboxMessages).bind(
    daemonClient
  );
  try {
    const response = await getOutboxMessages(request);
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getOutboxMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("getSentMessages", async () => {
  if (FAKE_DATA) {
    return [
      {
        id: "1",
        from: "SuAlEh",
        to: "me",
        message:
          "Send send!\n\nthis is a test message\n\nnot the best,\nSuAlEh",
        timestamp: new Date(),
      },
      {
        id: "2",
        from: "sent happy happy",
        to: "me",
        message: "HIHI",
        timestamp: new Date(),
      },
    ];
  }
  const request = new daemonM.GetSentMessagesRequest();
  const getSentMessages = promisify(daemonClient.getSentMessages).bind(
    daemonClient
  );
  try {
    const response = await getSentMessages(request);
    const lm = response.getMessagesList();
    const l = lm.map(convertProtobufMessageToTypedMessage);
    return l;
  } catch (e) {
    console.log(`error in getSentMessages: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("getFriendList", async () => {
  if (FAKE_DATA) {
    return [
      {
        name: "Friend 1",
        status: "initiated",
      },
      {
        name: "Friend 2",
        status: "added",
      },
      {
        name: "Friend 3",
        status: "added",
      },
    ];
  }
  const request = new daemonM.GetFriendListRequest();
  const getFriendList = promisify(daemonClient.getFriendList).bind(
    daemonClient
  );
  try {
    const response = await getFriendList(request);
    const lm = response.getFriendInfosList();
    const l = lm.map((m) => {
      return {
        name: m.getName(),
        status: m.getEnabled() ? "added" : "initiated",
      };
    });
    return l;
  } catch (e) {
    console.log(`error in getFriendList: ${e}`);
    return [];
  }
});

contextBridge.exposeInMainWorld("messageSeen", async (message_id) => {
  const request = new daemonM.MessageSeenRequest();
  request.setId(message_id);
  const messageSeen = promisify(daemonClient.messageSeen).bind(daemonClient);
  try {
    const response = await messageSeen(request);
    return true;
  } catch (e) {
    console.log(`error in send: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld("hasRegistered", async () => {
  const request = new daemonM.GetStatusRequest();
  const getStatus = promisify(daemonClient.getStatus).bind(daemonClient);
  try {
    const response = await getStatus(request);
    return response.getRegistered();
  } catch (e) {
    console.log(`error in hasRegistered: ${e}`);
    return false;
  }
});

contextBridge.exposeInMainWorld("register", async (username, accessKey) => {
  const request = new daemonM.RegisterUserRequest();
  request.setName(username);
  request.setBetaKey(accessKey);
  const register = promisify(daemonClient.registerUser).bind(daemonClient);
  try {
    const response = await register(request);
    return true;
  } catch (e) {
    console.log(`error in register: ${e}`);
  }
  return false;
});
contextBridge.exposeInMainWorld("isPlatformMac", () => {
  return process.platform === "darwin";
});
