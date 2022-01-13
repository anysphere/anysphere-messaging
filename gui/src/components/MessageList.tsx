import * as React from "react";
import { Message } from "../types";

function MessageList(props: {
  messages: string;
  readCallback: (message: Message) => void;
}) {
  const [messages, setMessages] = React.useState<Message[]>([]);

  React.useEffect(() => {
    if (props.messages === "new") {
      (window as any).getNewMessages().then((messages: Message[]) => {
        setMessages(messages);
      });
    } else {
      (window as any).getAllMessages().then((messages: Message[]) => {
        setMessages(messages);
      });
    }
  }, [props.messages]);

  return (
    <div>
      <div className="grid grid-cols-1 gap-2">
        {messages.map((message, index) => (
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
