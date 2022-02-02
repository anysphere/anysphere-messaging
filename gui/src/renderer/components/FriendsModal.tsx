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
}: {
  onClose: () => void;
  friend: string;
}) {
  return (
    <Modal onClose={onClose}>
      <div className="grid">
        <div>Init friend {friend} hello</div>
      </div>
    </Modal>
  );
}

export default FriendsModal;
