//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

function Modal({
  children,
  onClose,
}: {
  children: React.ReactNode;
  onClose: () => void;
}) {
  return (
    <div className="fixed z-10 top-0 left-0 right-0">
      <div
        className="backdrop-blur-sm drop-shadow-sm w-screen h-screen"
        onClick={onClose}
      >
        <div className="grid w-screen h-screen">
          <div
            className="place-self-center w-[36rem] h-[20rem] bg-white rounded-md p-2 border-2 border-asbrown-100 overflow-scroll"
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
