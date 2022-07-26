//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import { useState, useCallback, useRef } from "react";
import { generateUniqueNameFromDisplayName } from "./utils";
import { IDAnimation } from "./IDAnimation";
import { SmallEditor } from "./SmallEditor";
import { $getRoot, EditorState } from "lexical";
import { IncomingAsyncInvitation } from "types";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export default function AddFriendIncoming({
  onClose,
  setStatus,
  publicId,
  inv,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  publicId: string;
  inv: IncomingAsyncInvitation;
}): JSX.Element {
  const [displayName, setDisplayName] = useState<string>("");
  const [uniqueName, setUniqueName] = useState<string>("");
  const [hasModifiedUniqueName, setHasModifiedUniqueName] =
    useState<boolean>(false);
  const [progress, setProgress] = useState<number>(0);

  const accept = useCallback(() => {
    if (displayName.length == 0) {
      setStatus({
        message: "Please enter a name.",
        actionName: null,
      });
      return;
    }
    window
      .acceptAsyncInvitation({
        uniqueName,
        displayName,
        publicId: inv.publicId,
      })
      .then(() => {
        setStatus({
          message: `Added ${displayName}.`,
          actionName: null,
        });
        onClose();
      })
      .catch((err) => {
        setStatus({
          message: `Could not add ${displayName}: ${err}`,
          actionName: null,
        });
      });
  }, [displayName, uniqueName, inv.publicId, setStatus, onClose]);
  const reject = useCallback(() => {
    window
      .rejectAsyncInvitation({ publicId: inv.publicId })
      .then(() => {
        setStatus({
          message: "Rejected invitation from " + inv.publicId + ".",
          actionName: null,
        });
        onClose();
      })
      .catch((err) => {
        setStatus({
          message: `Failed to reject invitation: ${err}`,
          actionName: null,
        });
      });
  }, [inv.publicId, onClose, setStatus]);

  const component1 = (
    <>
      <h3
        className={classNames(
          "absolute left-0 right-0 top-14  z-10 text-center font-['Lora'] text-xl",
          DEBUG_COLORS ? "bg-yellow-700" : ""
        )}
      >
        Accept invitation?
      </h3>
      <h6 className="absolute top-24 left-0 right-0 text-center text-xs leading-5 text-asbrown-light">
        Only accept invitations from people you trust. Their message may claim
        that they are someone they are not, so please verify the public ID
        before accepting.
      </h6>
      <>
        <div
          className={classNames(
            "absolute bottom-20 top-36 left-0 right-0",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div className="grid h-full w-full grid-cols-1 justify-center justify-items-center">
            <div></div>
            <div className="grid h-full w-full grid-cols-1 justify-center justify-items-center gap-2">
              <div className="relative h-48 w-48">
                <IDAnimation seed={inv.publicId} bordersize={4} />
              </div>
              <div className="text-xs text-asbrown-light">
                anysphere.id/#{inv.publicId}
              </div>
            </div>
            <div></div>
            <div className="text-sm">{`"${inv.message}"`}</div>
          </div>
        </div>
        <div className="absolute left-0 right-0 bottom-8 grid items-center justify-center text-center">
          <button
            className={classNames(
              "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
            )}
            onClick={() => {
              setProgress(1);
            }}
          >
            Accept invitation
          </button>
        </div>
        <div className="absolute left-0 right-0 bottom-2 grid items-center justify-center text-center">
          <button
            className="unselectable ring-none text-xs text-asbrown-200 outline-none"
            onClick={() => {
              reject();
            }}
          >
            Remove invitation
          </button>
        </div>
      </>
    </>
  );
  const component2 = (
    <>
      <h3
        className={classNames(
          "absolute left-0 right-0 top-14  z-10 text-center font-['Lora'] text-xl",
          DEBUG_COLORS ? "bg-yellow-700" : ""
        )}
      >
        Great! What do you want to call them?
      </h3>
      <>
        <div
          className={classNames(
            "absolute bottom-20 top-36 left-0 right-0",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div className="grid h-full w-full grid-cols-1 justify-center justify-items-center">
            <div></div>
            <div className="relative h-48 w-48">
              <IDAnimation seed={inv.publicId} bordersize={4} />
            </div>
            <div></div>
            <div className="grid h-fit grid-cols-1 gap-4">
              <input
                type="text"
                autoFocus={true}
                className="
          m-0
          block
          h-fit
          w-full
          border-0
          border-b-2
          border-asbrown-100
          p-2 text-center text-xl
          placeholder-asbrown-300 focus:border-asbrown-300
          focus:ring-0
          "
                onChange={(e) => {
                  setDisplayName(e.target.value);
                  if (!hasModifiedUniqueName) {
                    setUniqueName(
                      generateUniqueNameFromDisplayName(e.target.value)
                    );
                  }
                }}
                value={displayName}
                placeholder="First Last"
              />
              <input
                type="text"
                className="m-0
          block
          h-fit
          w-full
          border-0
          border-asbrown-100
          p-0
          text-center text-sm text-asbrown-light
          placeholder-asbrown-300 focus:border-asbrown-300
          focus:ring-0
          "
                onChange={(e) => {
                  setUniqueName(e.target.value);
                  setHasModifiedUniqueName(true);
                }}
                value={uniqueName}
              />
            </div>
            <div></div>
          </div>
        </div>
        <div className="absolute left-0 right-0 bottom-8 grid items-center justify-center text-center">
          <button
            className={classNames(
              "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
            )}
            onClick={() => {
              accept();
            }}
          >
            Finish
          </button>
        </div>
        <div className="absolute left-0 right-0 bottom-2 grid items-center justify-center text-center">
          <button
            className="unselectable ring-none text-xs text-asbrown-200 outline-none"
            onClick={() => setProgress(0)}
          >
            ‹ Go back
          </button>
        </div>
      </>
    </>
  );

  return (
    <div className="grid h-full w-full items-center">
      <div
        className={classNames(
          "unselectable relative h-full",
          DEBUG_COLORS ? "bg-yellow-100" : ""
        )}
      >
        {(() => {
          switch (progress) {
            case 0:
              return component1;
            case 1:
              return component2;
            default:
              return component1;
          }
        })()}
      </div>
    </div>
  );
}
