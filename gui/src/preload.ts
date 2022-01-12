console.log("preload!");

const { contextBridge } = require("electron");

contextBridge.exposeInMainWorld("send", (to: string, message: string) => {
  console.log(`sending message to ${to}: ${message}`);
});
