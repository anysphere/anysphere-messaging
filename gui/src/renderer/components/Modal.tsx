import * as React from "react";

function Modal({
  children,
  onClose,
}: {
  children: React.ReactNode;
  onClose: () => void;
}) {
  return (
    <div className="fixed top-0 left-0 right-0">
      <div className="backdrop-blur-sm w-screen h-screen" onClick={onClose}>
        <div className="grid w-screen h-screen">
          <div
            className="place-self-center w-96 bg-white rounded-md p-2 border-2 border-asbrown-100"
            onClick={(event) => {
              event.stopPropagation();
            }}
          >
            {children}
          </div>
        </div>
      </div>
    </div>
  );
}

export default Modal;
