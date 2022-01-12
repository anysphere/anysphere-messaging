import * as React from "react";
import { Message } from "../types";

function MessageList(props: {
  messages: Message[];
  readCallback: (message: Message) => void;
}) {
  return (
    <div>
      <div className="grid grid-cols-1 gap-2">
        {props.messages.map((message, index) => (
          <div
            className="bg-white px-4 py-2 rounded-sm hover:cursor-pointer"
            key={index}
            onClick={() => props.readCallback(message)}
          >
            <div className="flex flex-row gap-5">
              <p>{message.from}</p>
              <p>{message.timestamp}</p>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

export default MessageList;
