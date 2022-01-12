import * as React from "react";

import MessageList from "../components/MessageList";

interface Tab {
  type: string;
  name: string;
  id: number;
}

const defaultTabs: Tab[] = [
  { type: "new", name: "New", id: 0 },
  { type: "all", name: "All", id: 1 },
];

function Main() {
  const [tabs, setTabs] = React.useState<Tab[]>(defaultTabs);
  const [selectedTab, setSelectedTab] = React.useState<number>(0);

  let selectedComponent;
  switch (tabs[selectedTab].type) {
    case "new":
      selectedComponent = (
        <MessageList messages={(window as any).getNewMessages()} />
      );
      break;
    case "all":
      selectedComponent = (
        <MessageList messages={(window as any).getAllMessages()} />
      );
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
