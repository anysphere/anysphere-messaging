//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

import MessageList from "./components/MessageList";
import Read from "./components/Read";
import Write from "./components/Write";
import FriendsModal from "./components/FriendsModal";
import { InitFriendModal } from "./components/FriendsModal";
import { RegisterModal } from "./components/RegisterModal";
import { Message } from "./types";
import { Tab, TabType, TabContainer } from "./components/Tabs";
import { truncate } from "./utils";
import { stringify } from "querystring";
import { CmdK } from "./components/cmd-k/CmdK";
import { CmdKPortal } from "./components/cmd-k/CmdKPortal";
import { CmdKSearch } from "./components/cmd-k/CmdKSearch";
import { CmdKResultRenderer } from "./components/cmd-k/CmdKResultRenderer";
import { KBarOptions } from "./components/cmd-k/types";
import { StatusHandler, StatusContext } from "./components/Status";

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", data: null, unclosable: true },
  { type: TabType.All, name: "All", data: null, unclosable: true },
];

function MainWrapper() {
  return (
    <StatusHandler>
      <Main />
    </StatusHandler>
  );
}

function Main() {
  const [tabs, setTabs] = React.useState<Tab[]>(defaultTabs);
  const [previousSelectedTab, setPreviousSelectedTab] =
    React.useState<number>(0);
  const [selectedTab, setSelectedTab] = React.useState<number>(0);
  const [modal, setModal] = React.useState<JSX.Element | null>(null);

  const statusState = React.useContext(StatusContext);
  console.log("statusState");
  console.log(statusState);
  console.log("endstatusState");

  const readMessage = React.useCallback(
    (message: Message, mode: string) => {
      for (let i = 0; i < tabs.length; i++) {
        if (tabs[i].type === TabType.Read && tabs[i].data.id === message.id) {
          setPreviousSelectedTab(selectedTab);
          setSelectedTab(i);
          return;
        }
      }
      (window as any).messageSeen(message.id);
      const readTab = {
        type: TabType.Read,
        name: `${truncate(message.message, 10)} - ${message.from}`,
        id: tabs.length,
        data: message,
        unclosable: false,
      };
      setTabs([...tabs, readTab]);
      console.log("selectedTabHI", selectedTab);
      setPreviousSelectedTab(selectedTab);
      setSelectedTab(readTab.id);
    },
    [tabs, selectedTab, previousSelectedTab]
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

  const { setStatus } = React.useContext(StatusContext);

  const send = React.useCallback(
    (content: string, to: string) => {
      (window as any).send(content, to).then((s: boolean) => {
        if (s) {
          console.log("SEND SUCCESS");
          statusState.setStatus({
            message: "Message sent!",
            action: () => {},
            actionName: null,
          });
          statusState.setVisible();
          // setStatus("Message sent!");
        } else {
          console.log("SEND FAILURE");
          statusState.setStatus({
            message: "Message failed to send!",
            action: () => {},
            actionName: null,
          });
          statusState.setVisible();
          // setStatus("Message failed to send!");
        }
      });
      let newTabs = [];
      for (let i = 0; i < tabs.length; i++) {
        if (i === selectedTab) {
          continue;
        } else {
          newTabs.push(tabs[i]);
        }
      }
      console.log("previousSelectedTab", previousSelectedTab);
      if (previousSelectedTab < tabs.length) {
        setSelectedTab(previousSelectedTab);
      } else {
        setSelectedTab(0);
      }
      setPreviousSelectedTab(selectedTab);
      setTabs(newTabs);
    },
    [tabs, selectedTab, previousSelectedTab, setStatus]
  );

  const writeMessage = React.useCallback(() => {
    const writeTab: Tab = {
      type: TabType.Write,
      name: "Compose",
      data: {
        content: "",
        to: "",
        multiSelectState: { text: "" },
        focus: "to",
      },
      unclosable: false,
    };
    setTabs([...tabs, writeTab]);
    setPreviousSelectedTab(selectedTab);
    setSelectedTab(tabs.length);
  }, [tabs, selectedTab, previousSelectedTab]);

  const closeModal = React.useCallback(() => {
    setModal(null);
  }, [setModal]);

  const openFriendModal = React.useCallback(() => {
    setModal(
      <FriendsModal onClose={closeModal} onAddFriend={openAddFriendModal} />
    );
  }, [setModal, closeModal]);

  const openAddFriendModal = React.useCallback(
    (friend: string) => {
      (window as any)
        .generateFriendKey(friend)
        .then(({ friend, key }: { friend: string; key: string }) => {
          setModal(
            <InitFriendModal
              onClose={closeModal}
              friend={friend}
              friendKey={key}
              onPasteKey={(key: string) => {
                (window as any).addFriend(friend, key);
                closeModal();
              }}
            />
          );
        });
    },
    [setModal, closeModal]
  );

  let selectedComponent;
  switch (tabs[selectedTab].type) {
    case TabType.New:
      selectedComponent = (
        <MessageList
          readCallback={(m: Message) => readMessage(m, "new")}
          messages="new"
        />
      );
      break;
    case TabType.All:
      selectedComponent = (
        <MessageList
          readCallback={(m: Message) => readMessage(m, "all")}
          messages="all"
        />
      );
      break;
    case TabType.Read:
      selectedComponent = (
        <Read
          message={tabs[selectedTab].data}
          onClose={() => {
            closeTab(selectedTab);
          }}
        />
      );
      break;
    case TabType.Write:
      selectedComponent = (
        <Write
          send={send}
          edit={editWrite}
          data={tabs[selectedTab].data}
          onClose={() => {
            closeTab(selectedTab);
          }}
        />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  const closeTab = React.useCallback(
    (index: number) => {
      let newTabs = [];
      for (let i = 0; i < tabs.length; i++) {
        if (i === index) {
          continue;
        } else {
          newTabs.push(tabs[i]);
        }
      }
      if (index === selectedTab) {
        console.log("previousSelectedTab", previousSelectedTab);
        if (previousSelectedTab < tabs.length) {
          setSelectedTab(previousSelectedTab);
        } else {
          setSelectedTab(0);
        }
        setPreviousSelectedTab(selectedTab);
      }
      if (index < selectedTab) {
        setPreviousSelectedTab(selectedTab);
        setSelectedTab(selectedTab - 1);
      }
      setTabs(newTabs);
    },
    [tabs, selectedTab, setTabs, setSelectedTab, previousSelectedTab]
  );

  React.useEffect(() => {
    (window as any).hasRegistered().then((registered: boolean) => {
      if (!registered) {
        setModal(
          <RegisterModal
            onClose={closeModal}
            onRegister={(username: string, key: string) => {
              (window as any)
                .register(username, key)
                .then((registered: boolean) => {
                  if (registered) {
                    closeModal();
                  }
                });
            }}
          />
        );
      }
    });
  }, []);
  const CmdKActions = [
    {
      id: "friend",
      name: "Add and Manage Friends",
      shortcut: ["f"],
      keywords: "friends",
      perform: () => {
        openFriendModal();
      },
    },
    {
      id: "write",
      name: "Compose",
      shortcut: ["c"],
      keywords: "write compose messages",
      perform: writeMessage,
    },
    {
      id: "search",
      name: "Search",
      shortcut: ["/"],
      keywords: "search",
    },
    {
      id: "next_tab",
      name: "Next Tab",
      shortcut: ["g", "t"],
      keywords: "next tab",
      perform: () => {
        setPreviousSelectedTab(selectedTab);
        setSelectedTab((selectedTab + 1) % tabs.length);
      },
    },
    // {
    //   id: "settings",
    //   name: "Settings",
    //   shortcut: ["s"],
    //   keywords: "settings",
    // },
    {
      id: "help",
      name: "Help",
      shortcut: ["h"],
      keywords: "help",
    },
    // {
    //   id: "quit",
    //   name: "Quit",
    //   shortcut: ["q"],
    //   keywords: "quit",
    // },
  ];

  const CmdKOptions: KBarOptions = {
    callbacks: {
      onClose: () => {
        closeModal();
      },
    },
  };

  return (
    <div className="w-full">
      <div className="h-4 draggable" />
      <div className="flex flex-row gap-2 fixed w-full pt-2 px-2">
        <TabContainer
          tabs={tabs}
          selectTab={(t) => {
            setPreviousSelectedTab(selectedTab);
            setSelectedTab(t);
          }}
          closeTab={closeTab}
          selectedTab={selectedTab}
          hidden={
            false &&
            (tabs[selectedTab].type === TabType.Write ||
              tabs[selectedTab].type === TabType.Read)
          }
        />
        <button
          className="unselectable px-2 rounded-md bg-asbrown-100 text-asbrown-light "
          onClick={openFriendModal}
        >
          <div className="codicon codicon-person-add"></div>
        </button>
        <button
          className="unselectable px-2 rounded-md bg-asbrown-100 text-asbrown-light "
          onClick={writeMessage}
        >
          <div className="codicon codicon-edit"></div>
        </button>
      </div>
      <div>
        <div className="overflow-scroll mt-9 pb-12 h-[calc(100vh_-_3.25rem)]">
          {selectedComponent}
        </div>
      </div>

      <CmdK actions={CmdKActions} options={CmdKOptions}>
        {modal}
        <CmdKPortal onClose={closeModal}>
          <CmdKSearch />
          <hr className="border-asbeige" />
          <div className="h-auto overflow-y-auto">
            <CmdKResultRenderer />
          </div>
        </CmdKPortal>
      </CmdK>
    </div>
  );
}

export default MainWrapper;
