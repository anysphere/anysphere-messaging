import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import { useState, useCallback, useRef } from "react";
import { generateUniqueNameFromDisplayName } from "./utils";
import { IDAnimation } from "./IDAnimation";
import { SmallEditor } from "./SmallEditor";
import { $getRoot, EditorState } from "lexical";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export default function AddFriendRemotePartTwo({
  onClose,
  setStatus,
  publicId,
  theirId,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  publicId: string;
  theirId: string;
}): JSX.Element {
  const [displayName, setDisplayName] = useState<string>("");
  const [uniqueName, setUniqueName] = useState<string>("");
  const [hasModifiedUniqueName, setHasModifiedUniqueName] =
    useState<boolean>(false);
  const [message, setMessage] = useState<string>("");
  const [progress, setProgress] = useState<number>(0);
  const editorStateRef = useRef<EditorState>(null);

  const finish = useCallback(() => {
    if (displayName.length == 0) {
      setStatus({
        message: "Please enter a name.",
        actionName: null,
      });
      return;
    }
    window
      .addAsyncFriend({
        uniqueName,
        displayName,
        publicId: theirId,
        message: message,
      })
      .then(() => {
        setStatus({
          message: `Sent an invitation to ${displayName}.`,
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
  }, [message, uniqueName, theirId, displayName, setStatus, onClose]);

  const component1 = (
    <>
      <h3
        className={classNames(
          "absolute left-0 right-0 top-14  z-10 text-center font-['Lora'] text-xl",
          DEBUG_COLORS ? "bg-yellow-700" : ""
        )}
      >
        New contact. What do you want to call them?
      </h3>
      <h6 className="absolute top-24 left-0 right-0 text-center text-xs leading-5 text-asbrown-light">
        You're adding anysphere.id/#{theirId}. Please make sure this is the
        right person.
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
            <div className="relative h-48 w-48">
              <IDAnimation seed={theirId} bordersize={4} />
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
            {/* <input
              type="text"
              autoFocus={true}
              className="
          m-0
          block
          h-fit
          w-full
          max-w-[24rem]
          rounded-lg
          border-2
          border-asbrown-100
          p-2 
          text-sm placeholder-asbrown-300
          focus:border-asbrown-300
          focus:ring-0
          "
              onChange={(e) => {
                setMessage(e.target.value);
              }}
              value={message}
              placeholder="Message"
            /> */}
            <div></div>
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
            Next
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
        Write an invitation message.
      </h3>
      <>
        <div
          className={classNames(
            "absolute bottom-20 top-36 left-0 right-0",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div className="grid h-full w-full grid-cols-1 justify-center justify-items-center">
            <div className="align-items-center grid grid-cols-1 place-content-center items-center gap-8">
              <div className="flex w-fit flex-row items-center gap-2 justify-self-center">
                <div className="relative h-12 w-12">
                  <IDAnimation seed={theirId} bordersize={2} />
                </div>
                <div className="text-lg">{displayName}</div>
              </div>
              <div className="w-96 rounded-lg border border-asbrown-100 p-2 shadow-sm">
                <SmallEditor
                  focused={true}
                  onFocus={() => {}}
                  placeholder="Message"
                  editorStateRef={editorStateRef}
                />
              </div>
            </div>
            <div></div>
          </div>
        </div>
        <h6 className="absolute bottom-24 left-0 right-0 text-center text-xs leading-5 text-asbrown-light">
          Your invitation will be delivered within 24 hours.
        </h6>
        <div className="absolute left-0 right-0 bottom-8 grid items-center justify-center text-center">
          <button
            className={classNames(
              "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
            )}
            onClick={() => {
              finish();
            }}
          >
            Send invitation
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
