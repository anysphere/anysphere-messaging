import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import { classNames } from "../../utils";

const yourStory = [
  "Haphazard ambassador hides gradient",
  "Cool mark resolves ambush",
  "Eerie kiss revises distinction",
  "Fashionable film fosters age",
  "New customer views proprietor",
  "Boiled plant kicks truck",
  "Spectacular commission complies triumph.",
];

export function StoryForm() {
  const numberOfLines = 7;
  let storyFromInput: String[] = [];
  for (let i = 0; i < numberOfLines; i++) {
    storyFromInput.push("");
  }

  function SimpleStoryLine(props: { lineNumber: number }): JSX.Element {
    return (
      <div
        className={classNames(
          props.lineNumber === 1 ? "" : "rounded-t-none",
          props.lineNumber === numberOfLines ? "" : "rounded-b-none",
          "relative rounded-md px-3 py-2 focus-within:z-10 focus-within:rounded-md focus-within:border focus-within:border-asgreen-dark focus-within:ring-1 focus-within:ring-asgreen-dark"
        )}
      >
        <label
          htmlFor={`Story ${props.lineNumber}`}
          className="block text-xs font-medium text-gray-900"
        >
          Sentence {props.lineNumber}
        </label>
        {/* we will read from the input into the story */}
        <input
          type="text"
          name={`Story ${props.lineNumber}`}
          id={`Story ${props.lineNumber}`}
          className="block w-full border-0 border-b-2 bg-transparent p-0 text-gray-900 placeholder-gray-500 focus:ring-0 sm:text-sm"
          placeholder="Jane Smith"
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
    <div className="mr-16 mt-4 mb-8">
      <div className="isolate -space-y-px rounded-md ">
        {[...Array(numberOfLines)].map((_, i) => (
          <SimpleStoryLine key={i} lineNumber={i + 1} />
        ))}
      </div>
    </div>
  );
}

function FocusedBlock(): JSX.Element {
  return (
    <div className="mt-10 max-w-lg lg:col-start-1 lg:col-end-10 lg:row-start-1 lg:row-end-4 lg:-mx-4 lg:mt-0 lg:max-w-none">
      <div className="relative z-10 rounded-lg shadow-xl">
        <div
          className="pointer-events-none absolute inset-0 rounded-lg border-2 border-asgreen-dark"
          aria-hidden="true"
        />
        <div className="absolute inset-x-0 top-0 translate-y-px transform">
          <div className="flex -translate-y-1/2 transform justify-center">
            <span className="inline-flex rounded-full bg-asgreen-dark px-4 py-1 text-sm font-semibold uppercase tracking-wider text-white">
              Recommended for you
            </span>
          </div>
        </div>
        <div className="rounded-t-lg bg-asbrown-100 px-6 pt-12 pb-10">
          <div>
            <h3 className="text-center text-3xl font-semibold text-gray-900 sm:-mx-6">
              In Person
            </h3>
          </div>
        </div>

        <div className=" bg-asbrown-100 px-6 pt-6 pb-8 sm:px-10 sm:py-10">
          <div className="grid-col-3 mx-auto grid w-full grid-flow-col">
            <div className="mx-auto w-fit rounded-md border border-asbrown-300">
              <div className="text-md p-2 text-gray-700">
                Enter the story of your friend
              </div>

              <StoryForm />
            </div>
            {/* dividier vertical between the two columns */}
            <div className="mx-auto w-fit rounded-md border border-asbrown-300" />

            <div className="mx-auto w-fit rounded-md border border-asbrown-300">
              <div className="text-md p-2 text-gray-700">Your Story</div>
              <ul role="list" className="space-y-3 p-2">
                {yourStory.map((feature) => (
                  <li key={feature} className="flex items-start">
                    <p className="text-sm text-gray-500">{feature}</p>
                  </li>
                ))}
              </ul>
            </div>
          </div>

          <div className="mt-16">
            <div className="mx-auto block w-1/2 rounded-lg shadow-md">
              <a
                href="#"
                className="mx-auto block w-full rounded-lg border border-transparent bg-asgreen-dark/90 px-6 py-4 text-center text-xl font-medium leading-6 text-white hover:bg-asgreen-dark"
                aria-describedby="tier-growth"
              >
                Add In Person
              </a>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

function HiddenBlock(): JSX.Element {
  return (
    <div className="mx-auto mt-10 max-w-md lg:col-start-9 lg:col-end-12 lg:row-start-2 lg:row-end-3 lg:m-0 lg:max-w-none">
      <div className="flex h-full flex-col overflow-hidden rounded-lg shadow-lg lg:rounded-none lg:rounded-r-lg">
        <div className="flex flex-1 flex-col">
          <div className="bg-gray-50 px-6 py-10">
            <div>
              <h3
                className="text-center text-2xl font-medium text-gray-900"
                id="tier-scale"
              >
                Remote
              </h3>
            </div>
          </div>
          <div className="flex flex-1 flex-col justify-between border-t-2 border-gray-100 bg-gray-50 p-6 sm:p-10 lg:p-6 xl:p-10">
            <div className="mt-8">
              <div className="rounded-lg shadow-md">
                <a
                  href="#"
                  className="block w-full rounded-lg border border-transparent bg-white px-6 py-3 text-center text-base font-medium text-asgreen-dark hover:bg-gray-50"
                  aria-describedby="tier-scale"
                >
                  Add Remotely
                </a>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default function AddFriend({
  onClose,
  setStatus,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
}): JSX.Element {
  return (
    <Modal onClose={onClose} type={ModalType.Large}>
      <div className="rounded-sm bg-asbrown-dark lg:h-full">
        <div className="px-4 pt-12 sm:px-6 lg:px-20 lg:pt-20">
          <div className="text-left">
            <p className="mt-2 text-xl font-extrabold text-asbrown-100 sm:text-2xl lg:text-2xl">
              Add Your Friend Securely
            </p>
          </div>
        </div>
        <div className="mt-16 bg-white pb-12 lg:mt-20 lg:h-full lg:pb-20">
          <div className="relative z-0 lg:h-full">
            <div className="absolute inset-0 h-5/6 bg-asbrown-dark lg:h-full" />
            <div className="mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
              <div className="relative  lg:grid lg:grid-cols-12">
                <FocusedBlock />
                <HiddenBlock />
              </div>
            </div>
          </div>
        </div>
      </div>
    </Modal>
  );
}
