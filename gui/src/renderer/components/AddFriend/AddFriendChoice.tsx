//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import { classNames } from "../../utils";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export default function AddFriendChoice({
  onClose,
  setStatus,
  chooseInperson,
  chooseRemote,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  chooseInperson: () => void;
  chooseRemote: () => void;
}): JSX.Element {
  return (
    <div className="grid h-full w-full items-center p-4">
      <div
        className={classNames(
          "unselectable grid h-full",
          DEBUG_COLORS ? "bg-yellow-100" : ""
        )}
      >
        <h1
          className={classNames(
            "self-center text-center text-sm text-asbrown-light",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          Add a new contact, securely.
        </h1>
        <div
          className={classNames(
            "grid grid-cols-2 divide-x divide-asbrown-100 self-center",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div
            className={classNames(
              "grid items-center px-8 py-16 hover:bg-asbeige",
              DEBUG_COLORS ? "bg-green-100" : ""
            )}
            onClick={chooseInperson}
          >
            <div className="pb-4">
              <h3 className="pb-12 text-center font-['Lora'] text-xl">
                In person.
              </h3>
              <div className="grid grid-cols-1 gap-4 text-sm">
                <p>More secure: you know for sure who you are adding.</p>
                <p>{"< 5 minutes."}</p>
                <p>Zero metadata leakage.</p>
              </div>
            </div>
          </div>
          <div
            className={classNames(
              "grid items-center px-8 py-16 hover:bg-asbeige",
              DEBUG_COLORS ? "bg-red-100" : ""
            )}
            onClick={chooseRemote}
          >
            <div className="pb-4">
              <h3 className="pb-12 text-center font-['Lora'] text-xl">
                Asynchronously.
              </h3>
              <div className="grid grid-cols-1 gap-4 text-sm">
                <p>More convenient: no need to be in the same room.</p>
                <p>{"~ 24 hours."}</p>
                <p>
                  <i>Basically</i> zero metadata leakage.*
                </p>
              </div>
            </div>
          </div>
        </div>
        <p className="self-center px-20 text-center text-xs text-asbrown-200">
          *Want to learn more? Read our blog post on how we ensure privacy when
          creating new contacts. Spoiler: it is hard, and no other communication
          platform bothers to do it.
        </p>
      </div>
    </div>
  );
}
