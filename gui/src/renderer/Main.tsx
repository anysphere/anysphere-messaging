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
import { Tab, TabType, TabContainer, useTabs } from "./components/Tabs";
import { randomString, truncate } from "./utils";
import { CmdK } from "./components/cmd-k/CmdK";
import { CmdKPortal } from "./components/cmd-k/CmdKPortal";
import { CmdKSearch } from "./components/cmd-k/CmdKSearch";
import { CmdKResultRenderer } from "./components/cmd-k/CmdKResultRenderer";
import { KBarOptions } from "./components/cmd-k/types";
import { StatusHandler, StatusContext } from "./components/Status";
import { SideBar } from "./components/SideBar/SideBar";
import { SideBarButton } from "./components/SideBar/SideBarProps";

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", data: null, unclosable: true, id: "new" },
  { type: TabType.All, name: "All", data: null, unclosable: true, id: "all" },
];

function MainWrapper() {
  return (
    <StatusHandler>
      <Main />
    </StatusHandler>
  );
}

function Main() {
  const [
    selectedTab,
    tabs,
    pushTab,
    closeTab,
    nextTab,
    previousTab,
    switchTab,
    updateTab,
  ] = useTabs(defaultTabs);
  const [modal, setModal] = React.useState<JSX.Element | null>(null);

  const statusState = React.useContext(StatusContext);

  const readMessage = React.useCallback(
    (message: Message, mode: string) => {
      for (let i = 0; i < tabs.length; i++) {
        if (tabs[i].type === TabType.Read && tabs[i].data.id === message.id) {
          switchTab(tabs[i].id);
          return;
        }
      }
      (window as any).messageSeen(message.id);
      const readTab = {
        type: TabType.Read,
        name: `${truncate(message.message, 10)} - ${message.from}`,
        id: `read-${message.id}`,
        data: message,
        unclosable: false,
      };
      pushTab(readTab);
    },
    [switchTab, tabs, pushTab]
  );

  const editWrite = React.useCallback(
    (data: any) => {
      if (selectedTab.type !== TabType.Write) {
        return;
      }
      const newTab = selectedTab;
      newTab.data = data;
      updateTab(newTab);
    },
    [selectedTab, updateTab]
  );

  const send = React.useCallback(
    (content: string, to: string) => {
      (window as any).send(content, to).then((s: boolean) => {
        if (s) {
          statusState.setStatus({
            message: `Message sent to ${to}!`,
            action: () => {},
            actionName: null,
          });
          statusState.setVisible();
        } else {
          statusState.setStatus({
            message: `Message to ${to} failed to send.`,
            action: () => {},
            actionName: null,
          });
          statusState.setVisible();
        }
      });
      closeTab(selectedTab.id);
    },
    [selectedTab, closeTab, statusState]
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
      id: "write-" + randomString(10),
    };
    pushTab(writeTab);
  }, [pushTab]);

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
                (window as any)
                  .addFriend(friend, key)
                  .then((successOrError: any) => {
                    if (successOrError === true) {
                      statusState.setStatus({
                        message: `Added ${friend}!`,
                        action: () => {},
                        actionName: null,
                      });
                      statusState.setVisible();
                    } else {
                      statusState.setStatus({
                        message: `Friend ${friend} not added: ${successOrError}`,
                        action: () => {},
                        actionName: null,
                      });
                      statusState.setVisible();
                    }
                  });
                closeModal();
              }}
            />
          );
        });
    },
    [setModal, closeModal, statusState]
  );

  let selectedComponent;
  console.log(selectedTab);
  switch (selectedTab.type) {
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
          message={selectedTab.data}
          onClose={() => {
            closeTab(selectedTab.id);
          }}
        />
      );
      break;
    case TabType.Write:
      selectedComponent = (
        <Write
          send={send}
          edit={editWrite}
          data={selectedTab.data}
          onClose={() => {
            closeTab(selectedTab.id);
          }}
        />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  React.useEffect(() => {
    (window as any).hasRegistered().then((registered: boolean) => {
      if (!registered) {
        setModal(
          <RegisterModal
            onClose={() => {}} // should not be able to close modal by clicking outside
            onRegister={(username: string, key: string) => {
              (window as any)
                .register(username, key)
                .then((registered: boolean) => {
                  if (registered) {
                    closeModal();
                    statusState.setStatus({
                      message: `Registered as ${username}.`,
                      action: () => {},
                      actionName: null,
                    });
                    statusState.setVisible();
                  } else {
                    statusState.setStatus({
                      message: `Unable to register. Perhaps incorrect access key?`,
                      action: () => {},
                      actionName: null,
                    });
                    statusState.setVisible();
                  }
                });
            }}
          />
        );
      }
    });
  }, []);

  // Sidebar options
  const [sidebarOpen, setSidebarOpen] = React.useState(false);
  const sideBarCallback = (b: SideBarButton) => {
    switch (b) {
      case SideBarButton.INBOX:
        return React.useCallback(() => {
          switchTab(tabs[0].id);
          // TODO(sualeh): the sidebar should handle this itself. 
          setSidebarOpen(false);
        }, [switchTab, tabs, setSidebarOpen]);
      case SideBarButton.OUTBOX:
        return React.useCallback(() => {
          switchTab(tabs[1].id);
          setSidebarOpen(false);
        }, [switchTab, tabs, setSidebarOpen]);
      case SideBarButton.SENT:
        return React.useCallback(() => {
          switchTab(tabs[2].id);
          setSidebarOpen(false);
        }, [switchTab, tabs, setSidebarOpen]);
      default:
        return React.useCallback(() => {}, []);
        break;
    }
  };

  // CmdK options and shortcuts
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
      perform: nextTab,
    },
    // {
    //   id: "settings",
    //   name: "Settings",
    //   shortcut: ["s"],
    //   keywords: "settings",
    // },
    {
      id: "sidebar",
      name: "Toggle Sidebar",
      // shortcuts are "m" and the left button
      shortcut: ["m"],
      keywords: "sidebar, side, menu, sent, outbox",
      perform: () => {
        setSidebarOpen(!sidebarOpen);
      },
    },
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
          selectTab={switchTab}
          closeTab={closeTab}
          nextTab={nextTab}
          previousTab={previousTab}
          selectedTab={selectedTab}
          hidden={
            false &&
            (selectedTab.type === TabType.Write ||
              selectedTab.type === TabType.Read)
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

      <SideBar
        title="Welcome to Private Messaging!"
        open={sidebarOpen}
        setOpen={setSidebarOpen}
        sideBarCallback={sideBarCallback}
      ></SideBar>
    </div>
  );
}

export default MainWrapper;
