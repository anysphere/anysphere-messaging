const { contextBridge } = require("electron");
const { promisify } = require("util");
var grpc = require("@grpc/grpc-js");

const daemonM = require("./daemon/schema/daemon_pb");
const daemonS = require("./daemon/schema/daemon_grpc_pb");
const daemonClient = new daemonS.DaemonClient(
  "unix:///var/run/asphr.sock",
  grpc.credentials.createInsecure()
);

contextBridge.exposeInMainWorld("send", async (to: string, message: string) => {
  const request = new daemonM.SendMessageRequest();
  request.setName(to);
  request.setMessage(message);
  const sendMessage = promisify(daemonClient.sendMessage).bind(daemonClient);
  try {
    const response = await sendMessage(request);
    if (!response.getSuccess()) {
      console.log(`unsuccessful send`);
      return;
    }
  } catch (e) {
    console.log(`error in send: ${e}`);
    return e;
  }
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
