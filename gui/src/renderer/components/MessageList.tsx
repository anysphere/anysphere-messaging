//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { Message } from "../../types";
import { truncate, formatTime } from "../utils";
import { BackgroundCircles } from "./BackgroundCircles/BackgroundCircles";
import { SelectableList } from "./SelectableList";

function MessageBlurb({
  message,
  active,
}: {
  message: Message;
  active: boolean;
}) {
  let timestamp_string = "";
  try {
    timestamp_string = formatTime(message.timestamp);
  } catch {}

  return (
    <div
      className={`${
        active ? "border-asbrown-100 bg-asbeige" : "border-white bg-white"
      } my-2 rounded-sm border-l-4 px-4 py-4`}
    >
      <div className="flex flex-row gap-5">
        <div className="text-sm text-asbrown-dark">
          {message.type === "incoming" ? message.from : `To: ${message.to}`}
        </div>
        <div className="text-sm text-asbrown-300">
          {truncate(message.message, 65)}
        </div>
        <div className="flex-1"></div>
        <div className="text-sm text-asbrown-200">{timestamp_string}</div>
      </div>
    </div>
  );
}

function NoMessages({ explanation }: { explanation: string }) {
  return (
    <div className="grid h-full pt-48">
      <div className="unselectable place-self-center text-xs text-asbrown-200">
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
      setMessages([]);
      let cancel = window.getNewMessagesStreamed((messages: Message[]) => {
        setMessages((prev: Message[]) => {
          // merge new messages with old messages, and sort them by timestamp
          let new_messages = messages.concat(prev);
          new_messages.sort((a, b) => {
            // sort based on timestamp
            if (a.timestamp > b.timestamp) {
              return -1;
            } else if (a.timestamp < b.timestamp) {
              return 1;
            } else {
              return 0;
            }
          });
          return new_messages;
        });
      });

      return cancel;
    } else if (props.messages === "all") {
      setMessages([]);
      let cancel = window.getAllMessagesStreamed((messages: Message[]) => {
        setMessages((prev: Message[]) => {
          // merge new messages with old messages, and sort them by timestamp
          let new_messages = messages.concat(prev);
          new_messages.sort((a, b) => {
            // sort based on timestamp
            if (a.timestamp > b.timestamp) {
              return -1;
            } else if (a.timestamp < b.timestamp) {
              return 1;
            } else {
              return 0;
            }
          });

          return new_messages;
        });
      });

      return cancel;
    } else if (props.messages === "outbox") {
      window.getOutboxMessagesOLD().then((messages: Message[]) => {
        setMessages(messages);
      });

      return () => {};
    } else if (props.messages === "sent") {
      window.getSentMessagesOLD().then((messages: Message[]) => {
        setMessages(messages);
      });

      return () => {};
    }

    // Error out
    throw new Error("Invalid messages type");
  }, [props.messages]);

  return (
    <div>
      <div className="mt-8 flex w-full place-content-center">
        <div className="flex w-full max-w-3xl flex-col place-self-center">
          <SelectableList
            items={messages.map((message) => {
              return {
                id: message.id,
                data: message,
                action: () => props.readCallback(message),
              };
            })}
            searchable={false}
            globalAction={() => {}}
            onRender={({ item, active }) =>
              typeof item === "string" ? (
                <div className="unselectable">{item}</div>
              ) : (
                <MessageBlurb
                  active={active}
                  key={item.id}
                  message={item.data}
                />
              )
            }
          />
        </div>
      </div>
      {messages.length === 0 && <BackgroundCircles />}
    </div>
  );
}

export default MessageList;
