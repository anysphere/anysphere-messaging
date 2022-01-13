console.log("preload!");

const daemonMessages = require("./daemon/schema/daemon_pb");
const daemonService = require("./daemon/schema/daemon_grpc_pb");

const registerUserRequest = new daemonMessages.RegisterUserRequest();
registerUserRequest.setName("test");
console.log(registerUserRequest);

const { contextBridge } = require("electron");

contextBridge.exposeInMainWorld("send", (to: string, message: string) => {
  console.log(`sending message to ${to}: ${message}`);
});

contextBridge.exposeInMainWorld("getNewMessages", () => {
  return [
    {
      id: "qowijefoiqwejf",
      from: "sualeh",
      to: "arvid",
      message: "hello",
      timestamp: "today",
    },
  ];
});

contextBridge.exposeInMainWorld("getAllMessages", () => {
  return [
    {
      id: "qowijefoiqwejf",
      from: "sualeh",
      to: "arvid",
      message: "hello",
      timestamp: "today",
    },
    {
      id: "fweofijweiofjoiqwejf",
      from: "sualeh",
      to: "arvid",
      message: "my first hello",
      timestamp: "yesterday",
    },
  ];
});
