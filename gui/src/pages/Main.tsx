import * as React from "react";

import MessageList from "../components/MessageList";
import Read from "../components/Read";
import Write from "../components/Write";
import { Message } from "../types";
import { Tab, TabType, TabContainer } from "../components/Tabs";

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
        <Write send={send} edit={editWrite} data={tabs[selectedTab].data} />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

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
          <button onClick={writeMessage}>+</button>
        </div>
        {selectedComponent}
      </div>
    </div>
  );
}

export default Main;
