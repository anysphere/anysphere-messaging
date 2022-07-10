import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import { useState } from "react";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export default function AddFriendRemote({
  onClose,
  setStatus,
  publicId,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  publicId: string;
}): JSX.Element {
  const [theirId, setTheirId] = useState<string>("");

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
        <h2
          className="absolute left-1/2 top-1/2 -translate-y-1/2 -translate-x-1/2 pb-2 text-base"
          style={{ userSelect: "text" }}
        >
          anysphere.id/#{publicId}
        </h2>
        <h6 className="absolute bottom-24 left-0 right-0 text-center text-sm text-asbrown-light">
          To add a contact, click their ID link or paste it below.
        </h6>
        <input
          type="text"
          autoFocus={true}
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
