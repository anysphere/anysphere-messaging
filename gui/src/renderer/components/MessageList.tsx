import * as React from "react";
import { Message } from "../types";
import { truncate, formatTime } from "../utils";

function MessageBlurb({
  message,
  onClick,
}: {
  message: Message;
  onClick: () => void;
}) {
  let timestamp_string = "";
  try {
    console.log(message.timestamp);
    timestamp_string = formatTime(message.timestamp);
  } catch {}

  return (
    <div
      className="bg-white px-4 py-4 rounded-sm hover:cursor-pointer"
      onClick={onClick}
    >
      <div className="flex flex-row gap-5">
        <div>{message.from}</div>
        <div>{truncate(message.message, 50)}</div>
        <div className="flex-1"></div>
        <div className="text-asbrown-200 text-sm">{timestamp_string}</div>
      </div>
    </div>
  );
}

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
    <div className="flex place-content-center w-full mt-8">
      <div className="place-self-center flex flex-col w-full max-w-3xl">
        <div className="grid grid-cols-1 gap-2">
          {messages.map((message, index) => (
            <MessageBlurb
              key={index}
              message={message}
              onClick={() => props.readCallback(message)}
            />
          ))}
        </div>
      </div>
    </div>
  );
}

export default MessageList;
