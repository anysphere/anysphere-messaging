import * as React from "react";

import MessageList from "../components/MessageList";
import Read from "../components/Read";
import Write from "../components/Write";
import { Message } from "../types";
import { Tab, TabType, TabContainer } from "../components/Tabs";
import { useHotkeys } from "react-hotkeys-hook";
import { write } from "original-fs";

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", data: null },
  { type: TabType.All, name: "All", data: null },
];

function Main() {
  const [tabs, setTabs] = React.useState<Tab[]>(defaultTabs);
  const [selectedTab, setSelectedTab] = React.useState<number>(0);

  const readMessage = React.useCallback(
    (message: Message) => {
      for (let i = 0; i < tabs.length; i++) {
        if (tabs[i].type === TabType.Read && tabs[i].data.id === message.id) {
          setSelectedTab(i);
          return;
        }
      }
      const readTab = {
        type: TabType.Read,
        name: `${message.from} — ${message.timestamp}`,
        id: tabs.length,
        data: message,
      };
      setTabs([...tabs, readTab]);
      setSelectedTab(readTab.id);
    },
    [tabs]
  );

  const editWrite = React.useCallback(
    (data: any) => {
      if (tabs[selectedTab].type !== TabType.Write) {
        return;
      }
      const newTab = tabs[selectedTab];
      newTab.data = data;
      let newTabs = [];
      for (let i = 0; i < tabs.length; i++) {
        if (i === selectedTab) {
          newTabs.push(newTab);
        } else {
          newTabs.push(tabs[i]);
        }
      }
      setTabs(newTabs);
    },
    [tabs, selectedTab]
  );

  const send = React.useCallback(
    (content: string, to: string) => {
      (window as any).send(content, to);
      let newTabs = [];
      for (let i = 0; i < tabs.length; i++) {
        if (i === selectedTab) {
          continue;
        } else {
          newTabs.push(tabs[i]);
        }
      }
      setTabs(newTabs);
      setSelectedTab(0);
    },
    [tabs, selectedTab]
  );

  const writeMessage = React.useCallback(() => {
    const writeTab: Tab = {
      type: TabType.Write,
      name: "Write",
      data: {
        content: "",
        to: "",
      },
    };
    setTabs([...tabs, writeTab]);
    setSelectedTab(tabs.length);
  }, [tabs]);

  let selectedComponent;
  switch (tabs[selectedTab].type) {
    case TabType.New:
      selectedComponent = (
        <MessageList readCallback={readMessage} messages="new" />
      );
      break;
    case TabType.All:
      selectedComponent = (
        <MessageList readCallback={readMessage} messages="all" />
      );
      break;
    case TabType.Read:
      selectedComponent = <Read message={tabs[selectedTab].data} />;
      break;
    case TabType.Write:
      selectedComponent = (
        <Write send={send} edit={editWrite} data={tabs[selectedTab].data} />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  const closeTab = React.useCallback(() => {
    let newTabs = [];
    for (let i = 0; i < tabs.length; i++) {
      if (i === selectedTab) {
        continue;
      } else {
        newTabs.push(tabs[i]);
      }
    }
    setTabs(newTabs);
    setSelectedTab(0);
  }, [tabs, selectedTab]);

  useHotkeys("CmdOrCtrl+n", () => writeMessage(), null, [writeMessage]);
  useHotkeys("CmdOrCtrl+w", () => closeTab(), null, [closeTab]);

  return (
    <div>
      <div className="h-4 draggable" />
      <div className="p-2">
        <div className="flex flex-row">
          <TabContainer
            tabs={tabs}
            selectTab={setSelectedTab}
            selectedTab={selectedTab}
          />
          <button
            className="unselectable px-2 rounded-md bg-[#e3e0d8] text-[#7a776d]"
            onClick={writeMessage}
          >
            +
          </button>
        </div>
        {selectedComponent}
      </div>
    </div>
  );
}

export default Main;
