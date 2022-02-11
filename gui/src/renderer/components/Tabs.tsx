//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

export interface Tab {
  type: TabType;
  name: string;
  data: any;
  unclosable: boolean;
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
  onClose,
  tab,
}: {
  selected: boolean;
  onClick: () => void;
  onClose: () => void;
  tab: Tab;
}) {
  const closeButton = (
    <div className="place-content-center grid">
      <div
        className="codicon codicon-close"
        onClick={(event) => {
          onClose();
          event.stopPropagation();
        }}
      ></div>
    </div>
  );

  return (
    <div
      className={`px-2 flex flex-row content-center py-1 unselectable text-sm hover:cursor-pointer ${
        selected ? "text-black" : "text-asbrown-200"
      }`}
      onClick={onClick}
    >
      <div>{tab.name}</div>
      {!tab.unclosable && closeButton}
    </div>
  );
}

export function TabContainer(props: {
  tabs: Tab[];
  selectTab: (index: number) => void;
  closeTab: (index: number) => void;
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
            onClose={() => props.closeTab(index)}
            tab={tab}
          />
        ))}
      </div>
      <hr className="border-asbrown-100" />
    </div>
  );
}
