import * as React from "react";
import Modal from "./Modal";

function FriendsModal({
  onClose,
  onAddFriend,
}: {
  onClose: () => void;
  onAddFriend: (_: string) => void;
}) {
  const [selected, setSelected] = React.useState<number>(0);
  const [friendname, setFriendname] = React.useState<string>("");

  return (
    <Modal onClose={onClose}>
      <div className="grid">
        <div className="text-asbrown-light text-xs pt-2">New</div>
        <hr className="border-asbrown-100" />
        <div className={`mt-1 ${selected === 0 ? "bg-asbeige" : ""}`}>
          <div className="p-2 flex flex-row gap-2">
            <div>Add friend:</div>
            <input
              autoFocus={selected === 0}
              type="text"
              value={friendname}
              onChange={(e) => {
                setFriendname(e.target.value);
              }}
              className="bg-red-100/[0] focus:border-none focus:border-red-500 flex-grow
              focus:outline-none
              focus:ring-0"
            />
            <button
              className="unselectable px-2 rounded-md bg-asbrown-100 text-asbrown-light "
              onClick={() => onAddFriend(friendname)}
            >
              <div className="codicon codicon-arrow-right"></div>
            </button>
          </div>
        </div>
        <div className="text-asbrown-300 text-xs text-center pt-1">
          (First enter their name. Next, you'll get a secret key.)
        </div>
        <div className="text-asbrown-light text-xs pt-2">
          Pending invitations
        </div>
        <hr className="border-asbrown-100" />
        <div className={`mt-1 p-2 ${selected === 1 ? "bg-gray-100" : ""}`}>
          Friend 1
        </div>
        <div className="text-asbrown-light text-xs pt-2">Friends</div>
        <hr className="border-asbrown-100" />
        <div className={`mt-1 p-2 ${selected === 1 ? "bg-gray-100" : ""}`}>
          Friend 2
        </div>
      </div>
    </Modal>
  );
}

export function InitFriendModal({
  onClose,
  friend,
  friendKey,
}: {
  onClose: () => void;
  friend: string;
  friendKey: string;
}) {
  return (
    <Modal onClose={onClose}>
      <div className="grid">
        <div className="text-center font-bold">{friend}</div>
        <div className="text-sm text-center py-1">
          You're almost done adding {friend} as a friend!
        </div>
        <div className="grid">
          <div className="text-sm">1. Send the following key to {friend}.</div>
          <div className="flex flex-row my-2 gap-1 justify-center">
            <code className="justify-self-center bg-asbeige py-1 px-2 rounded-md">
              {friendKey}
            </code>
            <button
              className="unselectable px-2 py-0 rounded-md bg-asbrown-100 text-asbrown-light"
              onClick={() => (window as any).copyToClipboard(friendKey)}
            >
              <div className="codicon codicon-copy"></div>
            </button>
          </div>
        </div>
        <div className="grid">
          <div className="text-sm">2. Paste their key below.</div>
          <div className="flex flex-row my-2 gap-1 justify-center mx-9">
            <input
              autoFocus
              type="text"
              className="bg-red-100/[0] px-2 font-mono focus:outline-none
              focus:ring-0 border-b-2 flex-grow border-asbrown-100"
            />
            <button className="unselectable px-2 py-0 rounded-md bg-asbrown-100 text-asbrown-light">
              <div className="codicon codicon-check"></div>
            </button>
          </div>
        </div>
        <div className="text-xs text-asbrown-dark pt-1">
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

export default FriendsModal;
