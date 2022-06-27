const growthFeatures = [
  "Quia rem est sed impedit magnam",
  "Dolorem vero ratione voluptates",
  "Qui sed ab doloribus voluptatem dolore",
  "Laborum commodi molestiae id et fugiat",
  "Nam ut ipsa nesciunt culpa modi dolor",
  "Quia rem est sed impedit magnam",
  "Dolorem vero ratione voluptates",
  "Quia rem est sed impedit magnam",
  "Dolorem vero ratione voluptates",
  "Qui sed ab doloribus voluptatem dolore",
];

function FocusedBlock(): JSX.Element {
  return (
    <div className="mx-auto mt-10 max-w-lg lg:col-start-2 lg:col-end-10 lg:row-start-1 lg:row-end-4 lg:mx-0 lg:mt-0 lg:max-w-none">
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
            <h3
              className="text-center text-3xl font-semibold text-gray-900 sm:-mx-6"
              id="tier-growth"
            >
              In Person
            </h3>
            {/* <div className="mt-4 flex items-center justify-center">
              <span className="flex items-start px-3 text-6xl tracking-tight text-gray-900 sm:text-6xl">
                <span className="mt-2 mr-2 text-4xl font-medium">$</span>
                <span className="font-extrabold">149</span>
              </span>
              <span className="text-2xl font-medium text-asbrown-200">
                /month
              </span>
            </div> */}
          </div>
        </div>
        <div className="rounded-b-lg bg-asbrown-100 px-6 pt-10 pb-8 sm:px-10 sm:py-10">
          <ul role="list" className="space-y-4">
            {growthFeatures.map((feature) => (
              <li key={feature} className="flex items-start">
                <div className="flex-shrink-0">
                  {/* <CheckIcon
                    className="h-6 w-6 flex-shrink-0 text-green-500"
                    aria-hidden="true"
                  /> */}
                </div>
                <p className="ml-3 text-base font-medium text-gray-500">
                  {feature}
                </p>
              </li>
            ))}
          </ul>
          <div className="mt-10">
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
              {/* <div className="mt-4 flex items-center justify-center">
                <span className="flex items-start px-3 text-6xl tracking-tight text-gray-900">
                  <span className="mt-2 mr-2 text-4xl font-medium">$</span>
                  <span className="font-extrabold">349</span>
                </span>
                <span className="text-xl font-medium text-gray-500">
                  /month
                </span>
              </div> */}
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

export default function AddFriend() {
  return (
    <div className="bg-asbrown-dark">
      <div className="px-4 pt-12 sm:px-6 lg:px-8 lg:pt-20">
        <div className="text-center">
          {/* <h2 className="text-lg font-semibold uppercase leading-6 tracking-wider text-gray-300">
            Friends
          </h2> */}
          <p className="mt-2 text-3xl font-extrabold text-asbrown-100 sm:text-4xl lg:text-5xl">
            Add Your Friend Securely
          </p>
          {/* <p className="mx-auto mt-3 max-w-4xl text-xl text-gray-300 sm:mt-5 sm:text-2xl">
            Lorem ipsum dolor sit, amet consectetur adipisicing elit. Velit
            numquam eligendi quos odit doloribus molestiae voluptatum.
          </p> */}
        </div>
      </div>

      <div className="mt-16 bg-white pb-12 lg:mt-20 lg:pb-20">
        <div className="relative z-0">
          <div className="absolute inset-0 h-5/6 bg-asbrown-dark lg:h-2/3" />
          <div className="mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
            <div className="relative  lg:grid lg:grid-cols-12">
              <FocusedBlock />
              <HiddenBlock />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
