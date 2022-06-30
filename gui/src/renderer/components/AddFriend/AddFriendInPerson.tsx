import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import * as React from "react";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

function ProgressBar({ progress }: { progress: number }): JSX.Element {
  return (
    <div className="mx-auto text-xs text-asbrown-100">
      <div className="grid grid-cols-3 gap-16 text-center">
        <div className={classNames(progress == 0 ? "text-asbrown-light" : "")}>
          1. Share stories.
        </div>
        <div>2. Verify.</div>
        <div>3. Finish.</div>
      </div>
    </div>
  );
}

export function StoryForm({
  numberOfLines,
  completeAll,
}: {
  numberOfLines: number;
  completeAll: () => void;
}): JSX.Element {
  let storyFromInput: String[] = [];
  for (let i = 0; i < numberOfLines; i++) {
    storyFromInput.push("");
  }

  function SimpleStoryLine(props: { lineNumber: number }): JSX.Element {
    return (
      <div className="flex flex-row gap-4">
        <p className="text-sm text-asbrown-300">{props.lineNumber}.</p>
        <input
          type="text"
          name={`Story ${props.lineNumber}`}
          id={`Story ${props.lineNumber}`}
          className="m-0
          block
          w-full
          border-0
          border-b-2
          border-asbrown-100 p-0 text-sm
          focus:border-asbrown-300 focus:ring-0"
        />
      </div>
    );
  }

  // attach event listeners to the inputs
  function handleChange(event: React.ChangeEvent<HTMLInputElement>) {
    const target = event.target;
    const value = target.value;
    const name = target.name;
    const lineNumber = Number(name.split(" ")[1]);
    storyFromInput[lineNumber - 1] = value;
  }

  return (
    <div className="grid grid-cols-1 gap-4">
      {[...Array(numberOfLines)].map((_, i) => (
        <SimpleStoryLine key={i} lineNumber={i + 1} />
      ))}
    </div>
  );
}

export default function AddFriendInPerson({
  onClose,
  setStatus,
  story,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  story: string;
}): JSX.Element {
  const [progress, setProgress] = React.useState<number>(0);
  const [showtedious, setShowtedious] = React.useState<boolean>(false);
  const [showverify, setShowverify] = React.useState<boolean>(false);

  React.useEffect(() => {
    setTimeout(() => {
      setShowtedious(true);
    }, 60000);
  }, [progress]);

  const storylist = React.useMemo(() => {
    return story.split(". ").map((line) => {
      if (line[line.length - 1] == ".") {
        return line;
      } else {
        return line + ".";
      }
    });
  }, [story]);

  return (
    <div className="grid h-full w-full items-center">
      <div
        className={classNames(
          "unselectable grid h-full pt-8 pb-4",
          DEBUG_COLORS ? "bg-yellow-100" : ""
        )}
      >
        <ProgressBar progress={progress} />
        <h3
          className={classNames(
            "text-center font-['Lora'] text-xl",
            DEBUG_COLORS ? "bg-yellow-700" : ""
          )}
        >
          Share stories with each other.
        </h3>
        <h6 className="text-center text-xs text-asbrown-light">
          These correspond to your public key, and allow you and your friend to
          communicate privately.
        </h6>
        <div
          className={classNames(
            "grid grid-cols-2 divide-x divide-asbrown-100 self-center",
            DEBUG_COLORS ? "bg-purple-100" : ""
          )}
        >
          <div
            className={classNames(
              "grid items-center px-8 ",
              DEBUG_COLORS ? "bg-green-100" : ""
            )}
          >
            <div className="pb-4">
              <h3 className="pb-8 text-center text-xs text-asbrown-light">
                Your story.
              </h3>
              <div className="grid gap-4">
                {storylist.map((feature, index) => (
                  <div className="flex flex-row gap-4" key={index}>
                    <p className="text-sm text-asbrown-300">{index + 1}.</p>
                    <p className="mb-[2px] text-sm">{feature}</p>
                  </div>
                ))}
              </div>
            </div>
          </div>
          <div
            className={classNames(
              "grid items-center px-8 ",
              DEBUG_COLORS ? "bg-red-100" : ""
            )}
          >
            <div className="pb-4">
              <h3 className="pb-8 text-center text-xs text-asbrown-light">
                Their story.
              </h3>
              <StoryForm
                numberOfLines={storylist.length}
                completeAll={() => setShowverify(true)}
              />
            </div>
          </div>
        </div>
        <p
          className={classNames(
            "self-center px-20 pt-5 text-center text-xs transition",
            showtedious ? "text-asbrown-200" : "text-transparent",
            showverify ? "hidden" : ""
          )}
        >
          Tedious? We know, complete privacy is work. But trust us, it is worth
          it.
        </p>
        <div className="grid items-center justify-center text-center">
          <button
            className={classNames(
              "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light",
              showverify ? "" : "hidden"
            )}
            onClick={() => {
              console.log("next step");
              setShowverify(false);
            }}
          >
            Verify
          </button>
        </div>
      </div>
    </div>
  );
}
