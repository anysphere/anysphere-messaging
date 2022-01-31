import * as React from "react";
import Modal from "./Modal";

function FriendsModal({ onClose }: { onClose: () => void }) {
  return (
    <Modal onClose={onClose}>
      <div>add friends</div>
    </Modal>
  );
}

export default FriendsModal;
