import * as React from "react";

export interface Tab {
  type: TabType;
  name: string;
  data: any;
}

export enum TabType {
  New = "new",
  All = "all",
  Read = "read",
  Write = "write",
}

export function TabElem({
  selected,
  onClick,
  tab,
}: {
  selected: boolean;
  onClick: () => void;
  tab: Tab;
}) {
  return (
    <div
      className={`px-1 hover:cursor-pointer ${
        selected ? "text-black" : "text-gray-400"
      }`}
      onClick={onClick}
    >
      {tab.name}
    </div>
  );
}

export function TabContainer(props: {
  tabs: Tab[];
  selectTab: (index: number) => void;
  selectedTab: number;
}) {
  return (
    <div className="flex flex-row flex-1">
      {props.tabs.map((tab, index) => (
        <TabElem
          key={index}
          selected={index === props.selectedTab}
          onClick={() => props.selectTab(index)}
          tab={tab}
        />
      ))}
    </div>
  );
}
