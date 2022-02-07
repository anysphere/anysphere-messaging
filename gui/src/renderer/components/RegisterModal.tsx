import * as React from "react";
import Modal from "./Modal";

export function RegisterModal({
  onClose,
  onRegister,
}: {
  onClose: () => void;
  onRegister: (username: string, key: string) => void;
}) {
  const [username, setUsername] = React.useState<string>("");
  const [accesskey, setAccesskey] = React.useState<string>("");
  return (
    <Modal onClose={onClose}>
      <div className="grid gap-2">
        <div className="text-center font-bold">Register</div>
        <div className="text-sm text-center unselectable py-1">
          Anysphere is in alpha! For support, please see{" "}
          <a className="underline" href="https://discord.gg/FxCBpJRbT9">
            our Discord
          </a>
          .
        </div>
        <div className="flex flex-row my-2 gap-1 justify-center">
          <label className="unselectable">Name: </label>
          <input
            autoFocus
            type="text"
            value={username}
            onChange={(e) => {
              setUsername(e.target.value);
            }}
            className="bg-red-100/[0] px-1 mx-2 font-mono focus:outline-none
              focus:ring-0 border-b-2 flex-grow border-asbrown-100"
          />
        </div>
        <div className="flex flex-row my-2 gap-1 justify-center">
          <label className="unselectable">Access key: </label>
          <input
            autoFocus
            type="text"
            value={accesskey}
            onChange={(e) => {
              setAccesskey(e.target.value);
            }}
            className="bg-red-100/[0] mx-2 px-1 font-mono focus:outline-none
              focus:ring-0 border-b-2 flex-grow border-asbrown-100"
          />
        </div>
        <div className="grid">
          <button
            className="place-self-center rounded-lg unselectable bg-asbrown-100 text-asbrown-light px-3 py-1"
            onClick={() => onRegister(username, accesskey)}
          >
            Register
          </button>
        </div>
        <div className="text-xs text-asbrown-dark pt-1">
          Please remember that this is an alpha product. While we think that our
          current implementation meets our privacy guarantees, we probably have
          bugs, and those bugs may have privacy consequences! For now, don't
          send anything on Anysphere that you wouldn't send on Signal.
        </div>
      </div>
    </Modal>
  );
}
