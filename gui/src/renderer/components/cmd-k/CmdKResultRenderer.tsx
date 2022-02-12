import { useMatches } from "./CmdKMatches";
import { CmdKResultHandler } from "./CmdKResultHandler";

export function CmdKResultRenderer() {
  const { results } = useMatches();

  return (
    <CmdKResultHandler
      items={results}
      onRender={({ item, active }) =>
        typeof item === "string" ? (
          <div className="">{item}</div>
        ) : (
          <div
            className={`text-sm px-2 py-1 mx-auto h-full border-l-4 border-white ${
              active ? "bg-asbeige border-asbrown-300" : ""
            }`}
          >
            {item.name}
          </div>
        )
      }
    />
  );
}
