import * as React from "react";
import Modal from "./Modal";

function FriendsModal({ onClose }: { onClose: () => void }) {
  const [selected, setSelected] = React.useState<number>(0);

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
              className="bg-red-100/[0] focus:border-none focus:border-red-500
              focus:outline-none
              focus:ring-0"
            />
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

export default FriendsModal;
