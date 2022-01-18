import * as React from "react";
import { Message } from "../types";

function Read({ message }: { message: Message }) {
  return (
    <div className="flex place-content-center w-full mt-8">
      <div className="place-self-center flex flex-col w-full max-w-3xl bg-white p-2 px-4">
        <div className="flex flex-row content-center py-2">
          <div className="place-content-center grid">
            <div className="align-bottom">From:</div>
          </div>
          <div className="pl-2">{message.from}</div>
          <div className="flex-1"></div>
        </div>
        <hr className="border-asbrown-100" />
        <div className="whitespace-pre-wrap resize-none w-full focus:outline-none h-full grow pt-4 pb-4">
          {message.message}
        </div>
      </div>
    </div>
  );
}

export default Read;
