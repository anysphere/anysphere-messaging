console.log("preload!");

const { contextBridge } = require("electron");

contextBridge.exposeInMainWorld("test", () => {
  console.log("test hiii");
});
