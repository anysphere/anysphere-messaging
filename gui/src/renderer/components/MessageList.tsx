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
        <div className="text-asbrown-dark text-sm">{message.from}</div>
        <div className="text-asbrown-300 text-sm">
          {truncate(message.message, 70)}
        </div>
        <div className="flex-1"></div>
        <div className="text-asbrown-200 text-sm">{timestamp_string}</div>
      </div>
    </div>
  );
}

function NoMessages({ explanation }: { explanation: string }) {
  return (
    <div className="grid h-full pt-48">
      <div className="place-self-center text-asbrown-200 text-xs">
        {explanation}
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

  const noMessageExplanation =
    props.messages === "new" ? "No new messages." : "No messages.";

  return (
    <div>
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
      {messages.length === 0 && (
        <NoMessages explanation={noMessageExplanation} />
      )}
    </div>
  );
}

export default MessageList;
