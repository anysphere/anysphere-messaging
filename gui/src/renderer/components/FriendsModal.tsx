//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import Modal from "./Modal";
import { Friend } from "../../types";
import { SelectableList, ListItem } from "./SelectableList";
import * as daemon_pb from "../../daemon/schema/daemon_pb";

function LegacyFriendsModal({
  onClose,
  onAddFriend,
}: {
  onClose: () => void;
  onAddFriend: (_: string) => void;
}) {
  const [friendname, setFriendname] = React.useState<string>("");
  const [friends, setFriends] = React.useState<Friend[]>([]);

  React.useEffect(() => {
    // check whether window.getFriendList is defined
    if (typeof window.getFriendList === "function") {
      window.getFriendList().then((friends: Friend[]) => {
        setFriends(friends);
      });
    } else {
      let friends: Friend[] = [];
      for (let i = 0; i < 5; i++) {
        friends.push({
          uniqueName: `friend${i}`,
          displayName: `Friend ${i}`,
          publicId: `friend${i}`,
          invitationProgress: daemon_pb.InvitationProgress.COMPLETE,
        });
      }
      for (let i = 5; i < 10; i++) {
        friends.push({
          uniqueName: `friend${i}`,
          displayName: `Friend ${i}`,
          publicId: `friend${i}`,
          invitationProgress: daemon_pb.InvitationProgress.COMPLETE,
        });
      }

      setFriends(friends);
    }
  }, []);

  const addFriend = React.useCallback(() => {
    if (friendname === "") {
      console.log("ERR");
    } else {
      if (
        friends.find(
          (f) =>
            f.uniqueName === friendname &&
            f.invitationProgress === daemon_pb.InvitationProgress.COMPLETE
        )
      ) {
        console.log("Already added friend...");
      } else {
        onAddFriend(friendname);
      }
    }
  }, [onAddFriend, friendname]);

  React.useEffect(() => {
    const handler = (event: any) => {
      if (event.key === "Enter") {
        event.preventDefault();
        addFriend();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [addFriend]);

  let friendsList: ListItem<{
    type: "add" | "friend" | "none";
    name: string;
  }>[] = friends
    .filter(
      (friend) =>
        friend.invitationProgress === daemon_pb.InvitationProgress.COMPLETE
    )
    .map((friend, index) => ({
      id: `friend-${index}`,
      action: () => {},
      data: {
        type: "friend",
        name: friend.displayName,
      },
    }));
  if (friendsList.length === 0) {
    friendsList = [
      {
        id: "no-friends",
        action: () => {},
        data: {
          type: "none",
          name: "(No friends.)",
        },
      },
    ];
  }

  let invitations: ListItem<{
    type: "add" | "friend" | "none";
    name: string;
  }>[] = friends
    .filter(
      (friend) =>
        friend.invitationProgress !== daemon_pb.InvitationProgress.COMPLETE
    )
    .map((friend, index) => ({
      id: `invitation-${index}`,
      action: () => {
        onAddFriend(friend.uniqueName);
      },
      data: {
        type: "friend",
        name: friend.displayName,
      },
    }));
  if (invitations.length === 0) {
    invitations = [
      {
        id: "no-invitations",
        action: () => {},
        data: {
          type: "none",
          name: "(No pending invitations.)",
        },
      },
    ];
  }

  let selectableOptions: (
    | ListItem<{ type: "add" | "friend" | "none"; name: string }>
    | string
  )[] = [
    "New",
    {
      id: "add-friend",
      action: () => {},
      data: {
        type: "add",
        name: "",
      },
    },
    "Pending invitations",
    ...invitations,
    "Friends",
    ...friendsList,
  ];

  return (
    <Modal onClose={onClose}>
      <SelectableList
        items={selectableOptions}
        searchable={true}
        globalAction={() => {}}
        onRender={({ item, active }) => {
          if (typeof item === "string") {
            return (
              <div>
                <div className="unselectable pt-2 text-xs text-asbrown-light">
                  {item}
                </div>
                <hr className="border-asbrown-100" />
              </div>
            );
          }
          if (item.data.type === "add") {
            return (
              <div
                className={`flex flex-row gap-2 p-2 text-asbrown-dark ${
                  active ? "bg-asbeige" : ""
                }`}
              >
                <div className="unselectable text-sm ">Add contact:</div>
                <input
                  autoFocus={active}
                  type="text"
                  value={friendname}
                  onChange={(e) => {
                    setFriendname(e.target.value);
                  }}
                  placeholder="What you would like to call them?"
                  className="flex-grow bg-red-100/[0] text-sm placeholder:text-asbrown-200
              focus:border-none focus:border-red-500
              focus:outline-none
              focus:ring-0"
                />
                <button
                  className="unselectable rounded-md bg-asbrown-100 px-2 text-asbrown-light "
                  onClick={addFriend}
                  disabled={friendname.length === 0}
                >
                  <div className="codicon codicon-arrow-right"></div>
                </button>
              </div>
            );
          }
          if (item.data.type === "friend") {
            return (
              <div
                className={`py-1 px-2 text-asbrown-dark ${
                  active ? "bg-asbeige" : ""
                }`}
              >
                <div className="text-sm">{item.data.name}</div>
              </div>
            );
          }
          if (item.data.type === "none") {
            return (
              <div className="unselectable py-2 text-center text-xs text-asbrown-300">
                {item.data.name}
              </div>
            );
          }
          return <div></div>;
        }}
      />
      {/* <div className="grid">
        <div className="text-asbrown-light unselectable text-xs pt-2">New</div>
        <hr className="border-asbrown-100" />
        <div className={`mt-1 ${selected === 0 ? "bg-asbeige" : ""}`}>
          <div className="p-2 flex flex-row gap-2">
            <div className="unselectable text-sm">Add contact:</div>
            <input
              autoFocus={selected === 0}
              type="text"
              value={friendname}
              onChange={(e) => {
                setFriendname(e.target.value);
              }}
              placeholder="What you would like to call them?"
              className="bg-red-100/[0] focus:border-none focus:border-red-500 flex-grow
              focus:outline-none text-sm
              focus:ring-0
              placeholder:text-asbrown-200"
            />
            <button
              className="unselectable px-2 rounded-md bg-asbrown-100 text-asbrown-light "
              onClick={addFriend}
              disabled={friendname.length === 0}
            >
              <div className="codicon codicon-arrow-right"></div>
            </button>
          </div>
        </div>
        <div className="text-asbrown-light unselectable text-xs pt-2">
          Pending invitations
        </div>
        <hr className="border-asbrown-100" />
        {friends
          .filter((friend) => friend.status === "initiated")
          .map((friend, index) => (
            <div
              className={`mt-1 text-sm py-1 px-2 ${
                selected === index + 2 ? "bg-gray-100" : ""
              }`}
              key={index}
            >
              {friend.name}
            </div>
          ))}
        {friends.filter((friend) => friend.status === "initiated").length ===
          0 && (
          <div className="text-asbrown-300 unselectable text-xs text-center pt-1">
            (No pending invitations.)
          </div>
        )}
        <div className="text-asbrown-light text-xs unselectable pt-2">
          Friends
        </div>
        <hr className="border-asbrown-100" />
        {friends
          .filter((friend) => friend.status === "added")
          .map((friend, index) => (
            <div
              className={`mt-1 text-sm py-1 px-2 ${
                selected === index + 2 ? "bg-gray-100" : ""
              }`}
              key={index}
            >
              {friend.name}
            </div>
          ))}
      </div> */}
    </Modal>
  );
}

export function InitFriendModal({
  onClose,
  friend,
  friendKey,
  onPasteKey,
}: {
  onClose: () => void;
  friend: string;
  friendKey: string;
  onPasteKey: (_: string) => void;
}) {
  const [theirkey, setTheirkey] = React.useState<string>("");

  const submitPaste = React.useCallback(() => {
    onPasteKey(theirkey);
  }, [onPasteKey, theirkey]);

  React.useEffect(() => {
    const handler = (event: any) => {
      if (event.key === "Enter") {
        submitPaste();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [submitPaste]);

  return (
    <Modal onClose={onClose}>
      <div className="grid">
        <div className="text-center font-bold">{friend}</div>
        <div className="unselectable py-1 text-center text-sm">
          You're almost done adding {friend} as a friend!
        </div>
        <div className="grid">
          <div className="unselectable text-sm">
            1. Send the following key to {friend}.
          </div>
          <div className="my-2 flex flex-row justify-center gap-1">
            <code className="justify-self-center rounded-md bg-asbeige py-1 px-2">
              {friendKey}
            </code>
            <button
              className="unselectable rounded-md bg-asbrown-100 px-2 py-0 text-asbrown-light"
              onClick={() => window.copyToClipboard(friendKey)}
            >
              <div className="codicon codicon-copy"></div>
            </button>
          </div>
        </div>
        <div className="grid">
          <div className="unselectable text-sm">
            2. Ask {friend} to add you as a friend and send you their key.
          </div>
          <div className="my-2 mx-9 flex flex-row justify-center gap-1">
            <input
              autoFocus
              type="text"
              value={theirkey}
              onChange={(e) => {
                setTheirkey(e.target.value);
              }}
              placeholder="Paste their key here"
              className="flex-grow border-b-2 border-asbrown-100 bg-red-100/[0] px-2
              font-mono placeholder:text-asbrown-200 focus:outline-none focus:ring-0"
            />
            <button
              className="unselectable rounded-md bg-asbrown-100 px-2 py-0 text-asbrown-light"
              onClick={submitPaste}
            >
              <div className="codicon codicon-check"></div>
            </button>
          </div>
        </div>
        <div className="pt-1 text-xs text-asbrown-dark">
          Privacy details, because details matter: This key can be thought of as
          your public key — hence, it is okay for you to send it over an
          unencrypted channel, or show it to others! However, if you, say, make
          a public tweet containing this key and your friend's name, the whole
          world would reasonably be able to conclude that you are communicating
          with them. Ideally, you would share this key in person, but if you
          already have a communication channel open with {friend}, you might as
          well use that (because you already have the channel open, so sending
          one additional message doesn't change anything). We are working on a
          way to improve this flow, so stay tuned.
        </div>
      </div>
    </Modal>
  );
}

export default LegacyFriendsModal;
