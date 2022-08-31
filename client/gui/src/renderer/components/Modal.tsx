//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { classNames } from "../utils";

export enum ModalType {
  Standard = "standard",
  Medium = "medium",
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
}): JSX.Element {
  React.useEffect(() => {
    const handler = (event: KeyboardEvent): void => {
      if (event.key === "Escape") {
        onClose();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [onClose]);

  return (
    <div className="fixed top-0 left-0 right-0 z-10">
      <div
        className="h-screen w-screen drop-shadow-sm backdrop-blur-sm"
        onClick={onClose}
      >
        <div className="grid h-screen w-screen">
          <div
            className={classNames(
              (() => {
                switch (type) {
                  case ModalType.Standard:
                    return "h-[20rem] w-[36rem]";
                  case ModalType.Medium:
                    return "h-[min(40rem,90%)] w-[min(60rem,90%)]";
                  case ModalType.Large:
                    return "h-[90%] w-[90%]";
                }
              })(),
              "place-self-center overflow-scroll rounded-md border-4 border-asbrown-100 bg-white"
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
