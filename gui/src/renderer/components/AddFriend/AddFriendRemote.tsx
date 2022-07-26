//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import { useEffect, useState } from "react";
import { IDAnimation } from "./IDAnimation";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export default function AddFriendRemote({
  onClose,
  setStatus,
  publicId,
  onPastePublicId,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  publicId: string;
  onPastePublicId: (publicId: string) => void;
}): JSX.Element {
  const [theirId, setTheirId] = useState<string>("");

  useEffect(() => {
    // strip whitespace
    let id = theirId.replace(/\s/g, "");
    // remove http(s)://
    id = id.replace(/^https?:\/\//, "");
    if (id.startsWith("anysphere.id/#")) {
      id = id.substring("anysphere.id/#".length);
    }
    // for fake data, we want to check that id is not empty
    if (id.length == 0) {
      return;
    }
    // check if id is valid
    window
      .isValidPublicID({ publicId: id })
      .then(({ valid }) => {
        if (valid) {
          onPastePublicId(id);
        }
      })
      .catch((err) => {
        setStatus({
          message: `Failed to check if public ID is valid: ${err}`,
          actionName: null,
        });
      });
  }, [theirId, onPastePublicId, setStatus]);

  return (
    <div className="grid h-full w-full items-center">
      <div
        className={classNames(
          "unselectable relative h-full",
          DEBUG_COLORS ? "bg-yellow-100" : ""
        )}
      >
        <h3
          className={classNames(
            "absolute left-0 right-0 top-14  z-10 text-center font-['Lora'] text-xl",
            DEBUG_COLORS ? "bg-yellow-700" : ""
          )}
        >
          Share your Anysphere ID.
        </h3>
        <h6 className="absolute top-24 left-0 right-0 text-center text-xs text-asbrown-light">
          Your ID is not secret. Put it in your Twitter profile, on your
          website, or anywhere else. Others can use it to add you.
        </h6>
        <div
          className={classNames(
            "absolute bottom-20 top-36 left-0 right-0",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div className="grid h-full w-full grid-cols-1 justify-center justify-items-center">
            <div></div>
            <div className="relative h-48 w-48">
              <IDAnimation seed={publicId} bordersize={4} />
            </div>
            <div></div>
            <h2 className="text-xs" style={{ userSelect: "text" }}>
              anysphere.id/#{publicId}
            </h2>
            <div></div>
            <div></div>
            <div></div>
          </div>
        </div>
        <h6 className="absolute bottom-24 left-0 right-0 text-center text-sm text-asbrown-light">
          To add a contact, click their ID link or paste it below.
        </h6>
        <input
          type="text"
          className="
          absolute
          bottom-10
          m-0
          block
          h-fit
          w-full
          border-0
          border-b-2
          border-asbrown-100
          p-2 text-center text-sm
          placeholder-asbrown-300 focus:border-asbrown-300
          focus:ring-0
          "
          onChange={(e) => {
            setTheirId(e.target.value);
          }}
          value={theirId}
          placeholder="anysphere.id/#..."
        />
      </div>
    </div>
  );
}
