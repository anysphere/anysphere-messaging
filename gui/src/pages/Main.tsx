import * as React from "react";

import MessageList from "../components/MessageList";
import Read from "../components/Read";
import Write from "../components/Write";
import { Message } from "../types";

interface Tab {
  type: TabType;
  name: string;
  id: number;
  data: any;
}

enum TabType {
  New = "new",
  All = "all",
  Read = "read",
  Write = "write",
}

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", id: 0, data: null },
  { type: TabType.All, name: "All", id: 1, data: null },
];

function Main() {
  const [tabs, setTabs] = React.useState<Tab[]>(defaultTabs);
  const [selectedTab, setSelectedTab] = React.useState<number>(0);

  const readMessage = React.useCallback(
    (message: Message) => {
      for (let tab of tabs) {
        if (tab.type === TabType.Read && tab.data.id === message.id) {
          setSelectedTab(tab.id);
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
    (content: string) => {
      if (tabs[selectedTab].type !== TabType.Write) {
        return;
      }
      tabs[selectedTab].data = content;
    },
    [tabs, selectedTab]
  );

  const writeMessage = React.useCallback(() => {
    const writeTab: Tab = {
      type: TabType.Write,
      name: "New message",
      id: tabs.length,
      data: "draft...",
    };
    setTabs([...tabs, writeTab]);
    setSelectedTab(writeTab.id);
  }, [tabs]);

  let selectedComponent;
  switch (tabs[selectedTab].type) {
    case TabType.New:
      selectedComponent = (
        <MessageList
          readCallback={readMessage}
          messages={(window as any).getNewMessages()}
        />
      );
      break;
    case TabType.All:
      selectedComponent = (
        <MessageList
          readCallback={readMessage}
          messages={(window as any).getAllMessages()}
        />
      );
      break;
    case TabType.Read:
      selectedComponent = <Read message={tabs[selectedTab].data} />;
      break;
    case TabType.Write:
      selectedComponent = (
        <Write editCallback={editWrite} draft={tabs[selectedTab].data} />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  return (
    <div className="p-2">
      <div className="flex flex-row">
        <div className="flex flex-row flex-1">
          {tabs.map((tab) => (
            <div onClick={() => setSelectedTab(tab.id)} key={tab.id}>
              {tab.name}
            </div>
          ))}
        </div>
        <button onClick={writeMessage}>+</button>
      </div>
      {selectedComponent}
    </div>
  );
}

export default Main;
