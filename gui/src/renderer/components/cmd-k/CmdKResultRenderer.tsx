import { useMatches } from "./CmdKMatches";
import { CmdKResultHandler } from "./CmdKResultHandler";

export function CmdKResultRenderer() {
  const { results } = useMatches();

  return (
    <CmdKResultHandler
      items={results}
      onRender={({ item, active }) =>
        typeof item === "string" ? (
          <div>{item}</div>
        ) : (
          <div
            style={{
              background: active ? "#eee" : "transparent",
            }}
          >
            {item.name}
          </div>
        )
      }
    />
  );
}
