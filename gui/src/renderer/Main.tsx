//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

import {
  IncomingMessageList,
  OutgoingMessageList,
} from "./components/MessageList";
import Read from "./components/Read";
import Write from "./components/Compose/Write";
import { WriteData } from "./components/Compose/Write";
import { RegisterModal } from "./components/RegisterModal";
import { IncomingMessage, OutgoingMessage } from "../types";
import { Tab, TabType, TabContainer, useTabs } from "./components/Tabs";
import { randomString, truncate } from "./utils";
import { CmdK } from "./components/cmd-k/CmdK";
import { CmdKPortal } from "./components/cmd-k/CmdKPortal";
import { CmdKSearch } from "./components/cmd-k/CmdKSearch";
import { CmdKResultRenderer } from "./components/cmd-k/CmdKResultRenderer";
import { KBarOptions } from "./components/cmd-k/types";
import { StatusHandler, statusContext } from "./components/Status";
import { SideBar } from "./components/SideBar/SideBar";
import { SideBarButton } from "./components/SideBar/SideBarProps";
import { Invitations } from "./components/Invitations";
import AddFriend, { AddFriendScreen } from "./components/AddFriend/AddFriend";
import Modal from "./components/Modal";

const defaultTabs: Tab[] = [
  { type: TabType.New, name: "New", data: null, unclosable: true, id: "new" },
  { type: TabType.All, name: "All", data: null, unclosable: true, id: "all" },
];

function MainWrapper({
  initialModal,
  initialModalData,
}: {
  initialModal?: InitialModal;
  initialModalData: unknown;
}): JSX.Element {
  return (
    <StatusHandler>
      <Main initialModal={initialModal} initialModalData={initialModalData} />
    </StatusHandler>
  );
}

export enum InitialModal {
  AddFriendByPublicId,
}

function Main({
  initialModal,
  initialModalData,
}: {
  initialModal?: InitialModal;
  initialModalData: unknown;
}): JSX.Element {
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
  const [hasShownInitialModal, setHasShownInitialModal] = React.useState(false);

  const statusState = React.useContext(statusContext);

  const readMessage = React.useCallback(
    (message: IncomingMessage | OutgoingMessage, _mode: string) => {
      for (const tab of tabs) {
        if (tab.type === TabType.Read && tab.data.id === message.uid) {
          switchTab(tab.id);
          return;
        }
      }
      window.messageSeen({ id: message.uid }).catch(console.error);
      let title = "";
      if ("fromDisplayName" in message) {
        title = `${truncate(message.message, 10)} - ${message.fromDisplayName}`;
      } else {
        title = `${truncate(message.message, 10)} - me`;
      }
      const readTab = {
        type: TabType.Read,
        name: title,
        id: `read-${message.uid}`,
        data: message,
        unclosable: false,
      };
      pushTab(readTab);
    },
    [switchTab, tabs, pushTab]
  );

  const editWrite = React.useCallback(
    (data: unknown) => {
      if (selectedTab.type !== TabType.Write) {
        return;
      }
      const newTab = selectedTab;
      newTab.data = data;
      updateTab(newTab);
    },
    [selectedTab, updateTab]
  );

  const writeMessage = React.useCallback(() => {
    const writeData: WriteData = {
      multiSelectState: {
        text: "",
        friends: [],
      },
      content: "",
      focus: "to",
    };
    const writeTab: Tab = {
      type: TabType.Write,
      name: "Compose",
      data: writeData,
      unclosable: false,
      id: "write-" + randomString(10),
    };
    pushTab(writeTab);
  }, [pushTab]);

  const closeModal = React.useCallback(() => {
    setModal(null);
  }, [setModal]);

  React.useEffect(() => {
    if (hasShownInitialModal) {
      return;
    }
    if (initialModal === InitialModal.AddFriendByPublicId) {
      setModal(
        <AddFriend
          onClose={closeModal}
          setStatus={(x) => {
            statusState.setStatus(x);
            statusState.setVisible();
          }}
          initialScreen={AddFriendScreen.RemotePartTwo}
          theirPublicId={initialModalData as string}
        />
      );
    }
    setHasShownInitialModal(true);
  }, [
    closeModal,
    statusState,
    initialModal,
    initialModalData,
    hasShownInitialModal,
  ]);

  const openFriendModal = React.useCallback(() => {
    setModal(
      <AddFriend
        onClose={closeModal}
        setStatus={(x) => {
          statusState.setStatus(x);
          statusState.setVisible();
        }}
      />
    );
  }, [setModal, closeModal, statusState]);

  let selectedComponent;
  console.log(selectedTab);
  switch (selectedTab.type) {
    case TabType.New:
      selectedComponent = (
        <IncomingMessageList
          readCallback={(m: IncomingMessage) => readMessage(m, "new")}
          type="new"
        />
      );
      break;
    case TabType.All:
      selectedComponent = (
        <IncomingMessageList
          readCallback={(m: IncomingMessage) => readMessage(m, "all")}
          type="all"
        />
      );
      break;
    case TabType.Outbox:
      selectedComponent = (
        <OutgoingMessageList
          readCallback={(m: OutgoingMessage) => readMessage(m, "outbox")}
          type="outbox"
        />
      );
      break;
    case TabType.Sent:
      selectedComponent = (
        <OutgoingMessageList
          readCallback={(m: OutgoingMessage) => readMessage(m, "sent")}
          type="sent"
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
          onDone={() => closeTab(selectedTab.id)}
          edit={editWrite}
          data={selectedTab.data}
          onClose={() => {
            closeTab(selectedTab.id);
          }}
          setStatus={(x) => {
            statusState.setStatus(x);
            statusState.setVisible();
          }}
        />
      );
      break;
    case TabType.Invitations:
      selectedComponent = (
        <Invitations
          setStatus={(x) => {
            statusState.setStatus(x);
            statusState.setVisible();
          }}
          handleIncomingInvitation={(inv) => {
            setModal(
              <AddFriend
                onClose={closeModal}
                setStatus={(x) => {
                  statusState.setStatus(x);
                  statusState.setVisible();
                }}
                initialScreen={AddFriendScreen.Incoming}
                incomingInvitation={inv}
              />
            );
          }}
        />
      );
      break;
    default:
      selectedComponent = <div>Unknown tab</div>;
  }

  React.useEffect(() => {
    window
      .hasRegistered()
      .then((registered: boolean) => {
        if (!registered) {
          setModal(
            <RegisterModal
              onClose={() => {
                console.log(
                  "// should not be able to close modal by clicking outside"
                );
              }}
              onRegister={(username: string, key: string) => {
                window
                  .registerUser({ name: username, betaKey: key })
                  .then(() => {
                    closeModal();
                    statusState.setStatus({
                      message: `Registered!`,
                      actionName: null,
                    });
                    statusState.setVisible();
                  })
                  .catch((e) => {
                    console.error(e);
                    statusState.setStatus({
                      message: `Unable to register. Perhaps incorrect access key?`,
                      actionName: null,
                    });
                    statusState.setVisible();
                  });
              }}
            />
          );
        }
      })
      .catch(console.error);
  }, [closeModal, statusState]);

  const openOutbox = React.useCallback(() => {
    for (const tab of tabs) {
      if (tab.type === TabType.Outbox) {
        switchTab(tab.id);
        return;
      }
    }

    const outboxTab = {
      type: TabType.Outbox,
      name: "Outbox",
      id: "outbox",
      data: null,
      unclosable: false,
    };
    pushTab(outboxTab);
  }, [switchTab, tabs, pushTab]);

  const openSent = React.useCallback(() => {
    for (const tab of tabs) {
      if (tab.type === TabType.Sent) {
        switchTab(tab.id);
        return;
      }
    }
    const sentTab = {
      type: TabType.Sent,
      name: "Sent",
      id: "sent",
      data: null,
      unclosable: false,
    };
    pushTab(sentTab);
  }, [switchTab, tabs, pushTab]);

  // Sidebar options
  const [sidebarOpen, setSidebarOpen] = React.useState(false);
  const sideBarCallback = (b: SideBarButton): void => {
    setSidebarOpen(false);
    const invTab = {
      type: TabType.Invitations,
      name: "Invitations",
      id: `invitations`,
      data: null,
      unclosable: false,
    };
    switch (b) {
      case SideBarButton.INBOX:
        return switchTab("all");
      case SideBarButton.OUTBOX:
        return openOutbox();
      case SideBarButton.SENT:
        return openSent();
      case SideBarButton.ADD_FRIEND:
        return openFriendModal();
      case SideBarButton.INVITATIONS:
        for (const tab of tabs) {
          if (tab.type === TabType.Invitations && tab.id === "invitations") {
            switchTab(tab.id);
            return;
          }
        }
        return pushTab(invTab);
      default:
        return;
        break;
    }
  };

  // CmdK options and shortcuts
  const cmdKActions = [
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
    {
      id: "install-cli",
      name: "Install 'anysphere' command",
      keywords: "install, cli, command",
      perform: () => {
        setModal(
          <Modal
            onClose={() => {
              closeModal();
            }}
          >
            <div className="relative h-full w-full">
              <h1 className="absolute top-4 left-0 right-0 text-center text-sm font-bold">
                Install {"anysphere"} command
              </h1>
              <div className="absolute top-8 bottom-0 left-0 right-0 grid place-content-center">
                <div className="grid h-full w-full gap-2 p-2 text-sm">
                  <p>
                    Run the following sequence of commands in your terminal:
                  </p>
                  <div className="overflow-scroll bg-asbeige p-1">
                    <code className="whitespace-pre text-[11px]">
                      {window.isPlatformMac()
                        ? `sudo mkdir -p /usr/local/bin
sudo ln -sf /Applications/Anysphere.app/Contents/Resources/bin/anysphere /usr/local/bin/anysphere
cat << EOF >> ~/.zprofile
export PATH="\\$PATH:/usr/local/bin"
EOF`
                        : `sudo mkdir -p /usr/local/bin
sudo ln -sf /Applications/Anysphere.app/Contents/Resources/bin/anysphere /usr/local/bin/anysphere
cat << EOF >> ~/.zprofile
export PATH="\\$PATH:/usr/local/bin"
EOF`}
                    </code>
                  </div>
                  <div className="unselectable pt-1 text-xs text-asbrown-300">
                    Replace `.zprofile` with `.bash_profile` or something else
                    if you use a different shell.
                  </div>
                  <div className="unselectable pt-1 text-xs text-asbrown-300">
                    Why do we ask you to run this yourself? Administrator
                    privileges are needed to install the command, and we{" "}
                    {"don't"}
                    want to ask you for your password without you seeing exactly
                    what is being run.
                  </div>
                </div>
              </div>
            </div>
          </Modal>
        );
      },
    },
    // {
    //   id: "quit",
    //   name: "Quit",
    //   shortcut: ["q"],
    //   keywords: "quit",
    // },
  ];

  const cmdKOptions: KBarOptions = {
    callbacks: {
      onClose: () => {
        closeModal();
      },
    },
  };

  return (
    <div className="w-full">
      <div className="draggable h-4" />
      <div className="fixed flex w-full flex-row gap-2 px-2 pt-2">
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
          className="unselectable rounded-md bg-asbrown-100 px-2 text-asbrown-light "
          onClick={openFriendModal}
        >
          <div className="codicon codicon-person-add"></div>
        </button>
        <button
          className="unselectable rounded-md bg-asbrown-100 px-2 text-asbrown-light "
          onClick={writeMessage}
        >
          <div className="codicon codicon-edit"></div>
        </button>
      </div>
      <div>
        <div className="mt-9 h-[calc(100vh_-_3.25rem)] overflow-scroll pb-12">
          {selectedComponent}
        </div>
      </div>

      <CmdK actions={cmdKActions} options={cmdKOptions}>
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
        title="Communicate privately."
        open={sidebarOpen}
        setOpen={setSidebarOpen}
        sideBarCallback={sideBarCallback}
      ></SideBar>
    </div>
  );
}

export default MainWrapper;
