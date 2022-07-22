import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import { useState, useCallback } from "react";
import { generateUniqueNameFromDisplayName } from "./utils";

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

  const finish = useCallback(() => {
    if (displayName.length == 0) {
      setStatus({
        message: "Please enter a name.",
        action: () => {},
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
          Send invitation.
        </h3>
        <h6 className="absolute top-24 left-0 right-0 text-center text-xs leading-5 text-asbrown-light">
          Your invitation will be delivered within 24 hours to anysphere.id/#
          {theirId}.
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
              <div className="relative h-32 w-32">
                {/* <StoryAnimationSmall
              seed={computeSeedFromStories(story, theirstorylist)}
            />{" "} */}
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
              <div></div>
            </div>
          </div>
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
        </>
      </div>
    </div>
  );
}
