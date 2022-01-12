import * as React from "react";
import { Message } from "../types";

function MessageList(props: {
  messages: Message[];
  readCallback: (message: Message) => void;
}) {
  return (
    <div>
      <div className="grid grid-cols-1">
        {props.messages.map((message, index) => (
          <div key={index} onClick={() => props.readCallback(message)}>
            {message.from}: {message.message}
          </div>
        ))}
      </div>
    </div>
  );
}

export default MessageList;
