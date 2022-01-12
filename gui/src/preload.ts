console.log("preload!");

const { contextBridge } = require("electron");

contextBridge.exposeInMainWorld("send", (to: string, message: string) => {
  console.log(`sending message to ${to}: ${message}`);
});

contextBridge.exposeInMainWorld("getNewMessages", () => {
  return [
    {
      from: "sualeh",
      to: "arvid",
      message: "hello",
    },
  ];
});

contextBridge.exposeInMainWorld("getAllMessages", () => {
  return [
    {
      from: "sualeh",
      to: "arvid",
      message: "hello",
    },
    {
      from: "sualeh",
      to: "arvid",
      message: "my first hello",
    },
  ];
});
