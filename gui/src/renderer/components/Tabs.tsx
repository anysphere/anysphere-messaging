//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
// import { usePointerMovedSinceMount } from "../utils";

export interface Tab {
  type: TabType;
  name: string;
  data: unknown;
  unclosable: boolean;
  id: string;
}

export enum TabType {
  New = "new",
  All = "all",
  Read = "read",
  Write = "write",
  Outbox = "outbox",
  Sent = "sent",
  Invitations = "invitations",
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
}): JSX.Element {
  const [hovering, setHovering] = React.useState(false);
  const closeButton = (
    <div
      className={`grid place-content-center ${
        hovering || selected ? "" : "invisible"
      }`}
    >
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
      className={`unselectable flex flex-row content-center px-2 py-1 text-sm hover:cursor-pointer ${
        selected ? "text-black" : "text-asbrown-200"
      }`}
      onClick={onClick}
      onMouseOver={() => {
        setHovering(true);
      }}
      onMouseOut={() => setHovering(false)}
    >
      <div>{tab.name}</div>
      {!tab.unclosable && closeButton}
    </div>
  );
}

export function TabContainer(props: {
  tabs: Tab[];
  selectTab: (id: string) => void;
  closeTab: (id: string) => void;
  nextTab: () => void;
  previousTab: () => void;
  selectedTab: Tab;
  hidden: boolean;
}): JSX.Element {
  const [hovering, setHovering] = React.useState(false);

  React.useEffect(() => {
    const handler = (event: KeyboardEvent): void => {
      if (event.ctrlKey && event.shiftKey && event.key === "Tab") {
        event.preventDefault();
        props.previousTab();
      } else if (event.ctrlKey && event.key === "Tab") {
        event.preventDefault();
        props.nextTab();
      } else if (event.metaKey && event.key === "w") {
        event.preventDefault();
        props.closeTab(props.selectedTab.id);
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [props.previousTab, props.nextTab, props.closeTab, props.selectedTab]);

  // const pointerMoved = usePointerMovedSinceMount(); // commented out because used in code that auto-hides the tab bar, which we ended up deciding against for the moment

  return (
    <div
      className="w-full"
      onMouseOver={() => {
        setHovering(true);
      }}
      onMouseOut={() => setHovering(false)}
    >
      <div
        className={`mb-1 flex-1 ${
          props.hidden && !hovering ? "invisible" : ""
        }`}
      >
        <div className={`flex flex-row `}>
          {props.tabs.map((tab, index) => (
            <TabElem
              key={index}
              selected={tab.id === props.selectedTab.id}
              onClick={() => props.selectTab(tab.id)}
              onClose={() => props.closeTab(tab.id)}
              tab={tab}
            />
          ))}
        </div>
        <hr className="border-asbrown-100" />
      </div>
    </div>
  );
}
// returns: pushTab(tab), closeTab(id), nextTab(), previousTab(), switchTab(id), selectedTab
export function useTabs(
  initial: Tab[]
): [
  Tab,
  Tab[],
  (tab: Tab) => void,
  (id: string) => void,
  () => void,
  () => void,
  (id: string) => void,
  (tab: Tab) => void
] {
  const defaultTab = initial[0];
  const [tabs, setTabs] = React.useState<Tab[]>(initial);

  const [previousSelectedTab, setPreviousSelectedTab] = React.useState<string>(
    defaultTab.id
  );
  const [selectedTab, setSelectedTab] = React.useState<string>(defaultTab.id);

  const pushTab = React.useCallback((tab: Tab) => {
    setTabs((tabs) => [...tabs, tab]);
    setSelectedTab((selectedTab) => {
      setPreviousSelectedTab(selectedTab);
      return tab.id;
    });
  }, []);

  const closeTab = React.useCallback(
    (id: string) => {
      setTabs((tabs) => {
        const newTabs = tabs.filter((tab) => tab.id !== id);

        setSelectedTab((selectedTab) => {
          if (selectedTab === id) {
            if (
              newTabs.filter((tab) => tab.id === previousSelectedTab).length > 0
            ) {
              return previousSelectedTab;
            } else {
              return defaultTab.id;
            }
          } else {
            return selectedTab;
          }
        });

        return newTabs;
      });
    },
    [previousSelectedTab, defaultTab.id]
  );

  const nextTab = React.useCallback(() => {
    setSelectedTab((selectedTab) => {
      setPreviousSelectedTab(selectedTab);
      const index = tabs.findIndex((tab) => tab.id === selectedTab);
      return tabs[(index + 1) % tabs.length].id;
    });
  }, [tabs]);

  const previousTab = React.useCallback(() => {
    setSelectedTab((selectedTab) => {
      setPreviousSelectedTab(selectedTab);
      const index = tabs.findIndex((tab) => tab.id === selectedTab);
      return tabs[(index - 1 + tabs.length) % tabs.length].id;
    });
  }, [tabs]);

  const switchTab = React.useCallback((id: string) => {
    setSelectedTab((selectedTab) => {
      setPreviousSelectedTab(selectedTab);
      return id;
    });
  }, []);

  const selectedActualTab = React.useMemo(() => {
    const tab = tabs.find((tab) => tab.id === selectedTab);
    return tab ? tab : defaultTab;
  }, [tabs, selectedTab, defaultTab]);

  const updateTab = React.useCallback((tab: Tab) => {
    setTabs((tabs) => tabs.map((tabb) => (tabb.id === tab.id ? tab : tabb)));
  }, []);

  return [
    selectedActualTab,
    tabs,
    pushTab,
    closeTab,
    nextTab,
    previousTab,
    switchTab,
    updateTab,
  ];
}
