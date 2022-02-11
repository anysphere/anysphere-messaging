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

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", data: null, unclosable: true },
  { type: TabType.All, name: "All", data: null, unclosable: true },
];

function Main() {
  const [tabs, setTabs] = React.useState<Tab[]>(defaultTabs);
  const [selectedTab, setSelectedTab] = React.useState<number>(0);
  const [modal, setModal] = React.useState<JSX.Element | null>(null);

  const readMessage = React.useCallback(
    (message: Message, mode: string) => {
      for (let i = 0; i < tabs.length; i++) {
        if (tabs[i].type === TabType.Read && tabs[i].data.id === message.id) {
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
      unclosable: false,
    };
    setTabs([...tabs, writeTab]);
    setSelectedTab(tabs.length);
  }, [tabs]);

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
        setSelectedTab(0);
      }
      if (index < selectedTab) {
        setSelectedTab(selectedTab - 1);
      }
      setTabs(newTabs);
    },
    [tabs, selectedTab, setTabs, setSelectedTab]
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

  return (
    <div className="w-full">
      <div className="h-4 draggable" />
      <div className="flex flex-row gap-2 fixed w-full pt-2 px-2">
        <TabContainer
          tabs={tabs}
          selectTab={setSelectedTab}
          closeTab={closeTab}
          selectedTab={selectedTab}
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
      {modal}
    </div>
  );
}

export default Main;
