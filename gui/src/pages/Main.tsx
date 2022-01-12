import * as React from "react";

import MessageList from "../components/MessageList";
import ReadMessage from "../components/ReadMessage";
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

  let selectedComponent;
  switch (tabs[selectedTab].type) {
    case "new":
      selectedComponent = (
        <MessageList
          readCallback={readMessage}
          messages={(window as any).getNewMessages()}
        />
      );
      break;
    case "all":
      selectedComponent = (
        <MessageList
          readCallback={readMessage}
          messages={(window as any).getAllMessages()}
        />
      );
      break;
    case "read":
      selectedComponent = <ReadMessage message={tabs[selectedTab].data} />;
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  return (
    <div>
      <div className="flex flex-row">
        {tabs.map((tab) => (
          <div onClick={() => setSelectedTab(tab.id)} key={tab.id}>
            {tab.name}
          </div>
        ))}
      </div>
      {selectedComponent}
    </div>
  );
}

export default Main;
