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
      className={`px-1 unselectable hover:cursor-pointer ${
        selected ? "text-black" : "text-asbrown-200"
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
    <div className="flex-1 mb-1">
      <div className="flex flex-row">
        {props.tabs.map((tab, index) => (
          <TabElem
            key={index}
            selected={index === props.selectedTab}
            onClick={() => props.selectTab(index)}
            tab={tab}
          />
        ))}
      </div>
      <hr />
    </div>
  );
}
