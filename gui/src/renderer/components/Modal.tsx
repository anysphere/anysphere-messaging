//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { classNames } from "../utils";

export enum ModalType {
  Standard = "standard",
  Large = "large",
}

export function Modal({
  children,
  onClose,
  type = ModalType.Standard,
}: {
  children: React.ReactNode;
  onClose: () => void;
  type?: ModalType;
}) {
  return (
    <div className="fixed top-0 left-0 right-0 z-10">
      <div
        className="h-screen w-screen drop-shadow-sm backdrop-blur-sm"
        onClick={onClose}
      >
        <div className="grid h-screen w-screen">
          <div
            className={classNames(
              type == ModalType.Large
                ? "h-[95%] w-[95%]"
                : "h-[20rem] w-[36rem]",
              "place-self-center overflow-scroll rounded-md border-2 border-asbrown-100 bg-white p-2"
            )}
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
