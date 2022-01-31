import * as React from "react";
import Modal from "./Modal";

function FriendsModal({ onClose }: { onClose: () => void }) {
  const [selected, setSelected] = React.useState<number>(0);

  return (
    <Modal onClose={onClose}>
      <div className="grid">
        <div className={`${selected === 0 ? "bg-gray-100" : ""}`}>
          <div className="flex flex-row">
            <div>Add friend:</div>
            <input type="text" />
          </div>
        </div>
        <div className={`${selected === 1 ? "bg-gray-100" : ""}`}>Friend 1</div>
      </div>
    </Modal>
  );
}

export default FriendsModal;
