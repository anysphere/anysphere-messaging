console.log("preload!");

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
