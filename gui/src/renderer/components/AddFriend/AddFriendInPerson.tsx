import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import { classNames } from "../../utils";
import * as React from "react";
import seedrandom from "seedrandom";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

function ProgressBar({ progress }: { progress: number }): JSX.Element {
  return (
    <div className="mx-auto text-xs text-asbrown-100">
      <div className="grid grid-cols-3 gap-16 text-center">
        <div className={classNames(progress == 0 ? "text-asbrown-light" : "")}>
          1. Share stories.
        </div>
        <div className={classNames(progress == 1 ? "text-asbrown-light" : "")}>
          2. Verify.
        </div>
        <div className={classNames(progress == 2 ? "text-asbrown-light" : "")}>
          3. Finish.
        </div>
      </div>
    </div>
  );
}

export function StoryForm({
  theirstory,
  setTheirStory,
}: {
  theirstory: string[];
  setTheirStory(story: string[]): void;
}): JSX.Element {
  return (
    <div className="grid grid-cols-1 gap-4">
      {theirstory.map((line, i) => (
        <div className="flex flex-row gap-4">
          <p className="text-sm text-asbrown-300">{i + 1}.</p>
          <input
            type="text"
            name={`Story ${i}`}
            id={`Story ${i}`}
            className="m-0
          block
          w-full
          border-0
          border-b-2
          border-asbrown-100 p-0 text-sm
          focus:border-asbrown-300 focus:ring-0"
            onChange={(e) => {
              setTheirStory(
                theirstory.map((line, j) => {
                  if (j == i) {
                    return e.target.value;
                  }
                  return line;
                })
              );
            }}
            value={line}
          />
        </div>
      ))}
    </div>
  );
}

async function sha256(s: string) {
  const encoder = new TextEncoder();
  const data = encoder.encode(s);
  const hash = await crypto.subtle.digest("SHA-256", data);
  return hash;
}
async function sha256string(s: string) {
  const hashBuffer = await sha256(s);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  const hashHex = hashArray
    .map((b) => b.toString(16).padStart(2, "0"))
    .join("");
  return hashHex;
}

function StoryAnimation({ seed }: { seed: string }): JSX.Element {
  const [shaseed, setShaseed] = React.useState("");

  React.useEffect(() => {
    sha256string(seed).then((hash) => {
      setShaseed(hash);
    });
  });

  const rng = seedrandom(shaseed);

  return (
    <div className="grid grid-cols-1 gap-4">
      {rng()}
      {rng()}
      {rng()}
      <div
        className={`animation-delay-2000 absolute top-[20px] left-[135px] h-[115px] w-[115px] animate-blob rounded-full bg-gradient-to-t 
                      from-asbrown-dark to-asbrown-dark/70 opacity-80 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter
                      md:top-[10px] md:left-[-10px] md:h-[230px] md:w-[230px] `}
      ></div>
      <div
        className="absolute -top-[50px] left-[200px] h-[130px] w-[130px] animate-blob rounded-full bg-gradient-to-t from-asyellow-dark 
                      to-asyellow-light/70 opacity-80 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter 
                      md:-top-[130px] md:left-[260px] md:h-[300px] md:w-[300px]"
      ></div>
      <div
        className="animation-delay-4000 absolute -bottom-4 left-10 h-[190px] w-[190px] animate-blob rounded-full bg-gradient-to-t from-asgreen-dark
                      to-asgreen-dark/70 opacity-90 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter 
                      md:-bottom-3 md:left-0 md:h-[440px] md:w-[440px]"
      ></div>
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
  const [theirstorylist, setTheirstory] = React.useState<string[]>([]);

  React.useEffect(() => {
    setTimeout(() => {
      setShowtedious(true);
    }, 60000);
  }, [progress]);

  const storylist = React.useMemo(() => {
    const l = story.split(". ").map((line) => {
      if (line[line.length - 1] == ".") {
        return line;
      } else {
        return line + ".";
      }
    });
    setTheirstory(new Array<string>(l.length).fill(""));
    return l;
  }, [story]);

  React.useEffect(() => {
    if (theirstorylist.length == 0) {
      return;
    }
    console.log("theirstorylist", theirstorylist);
    let allComplete = true;
    theirstorylist.forEach((line) => {
      console.log("line", line);
      if (line.length == 0) {
        allComplete = false;
      }
    });
    if (allComplete) {
      setShowverify(true);
    }
  }, [theirstorylist]);

  const shareStoriesComponent = (
    <>
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
              theirstory={theirstorylist}
              setTheirStory={(story) => setTheirstory(story)}
            />
          </div>
        </div>
      </div>
      <div className="grid items-center justify-center text-center">
        <button
          className={classNames(
            "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light",
            showverify ? "" : "disabled cursor-not-allowed opacity-20"
          )}
          onClick={() => {
            console.log("next step");
            if (showverify) {
              setProgress(1);
            }
          }}
        >
          Verify
        </button>
      </div>
      <p
        className={classNames(
          "self-center px-20 text-center text-xs transition",
          showtedious ? "text-asbrown-200" : "text-transparent"
        )}
      >
        Tedious? We know, complete privacy is work. But trust us, it is worth
        it.
      </p>
    </>
  );

  const verifyStoriesComponent = (
    <>
      <h3
        className={classNames(
          "text-center font-['Lora'] text-xl",
          DEBUG_COLORS ? "bg-yellow-700" : ""
        )}
      >
        Are the animations identical?
      </h3>
      <div>
        <StoryAnimation seed={story + theirstorylist.join("")} />
      </div>
      <div className="grid items-center justify-center text-center">
        <button
          className={classNames(
            "unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
          )}
        >
          Yes, they are identical.
        </button>
        <button
          className="unselectable text-xs text-asbrown-200"
          onClick={() => setProgress(0)}
        >
          ‹ No, take me back.
        </button>
      </div>
    </>
  );
  const finishComponent = (
    <>
      <div>unimplemented</div>
    </>
  );

  return (
    <div className="grid h-full w-full items-center">
      <div
        className={classNames(
          "unselectable grid h-full pt-8 pb-4",
          DEBUG_COLORS ? "bg-yellow-100" : ""
        )}
      >
        <ProgressBar progress={progress} />
        {(() => {
          switch (progress) {
            case 0:
              return shareStoriesComponent;
            case 1:
              return verifyStoriesComponent;
            case 2:
              return finishComponent;
            default:
              console.log("ERRORRRRRR");
          }
        })()}
      </div>
    </div>
  );
}
