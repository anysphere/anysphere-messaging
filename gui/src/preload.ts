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
  } catch (e) {
    console.log(`error in send: ${e}`);
    return e;
  }
});

contextBridge.exposeInMainWorld("getNewMessages", async () => {
  const request = new daemonM.GetNewMessagesRequest();
  const getNewMessages = promisify(daemonClient.getNewMessages).bind(
    daemonClient
  );
  try {
    const response = await getNewMessages(request);
    return response.getMessagesList();
  } catch (e) {
    console.log(`error in getNewMessages: ${e}`);
    return e;
  }
});

contextBridge.exposeInMainWorld("getAllMessages", async () => {
  const request = new daemonM.GetAllMessagesRequest();
  const getAllMessages = promisify(daemonClient.getAllMessages).bind(
    daemonClient
  );
  try {
    const response = await getAllMessages(request);
    return response.getMessagesList();
  } catch (e) {
    console.log(`error in getAllMessages: ${e}`);
    return e;
  }
});
