//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { Message } from "../types";
import { truncate, formatTime } from "../utils";
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
    console.log(message.timestamp);
    timestamp_string = formatTime(message.timestamp);
  } catch {}

  return (
    <div
      className={`${
        active ? "bg-asbeige border-asbrown-100" : "bg-white border-white"
      } border-l-4 px-4 py-4 rounded-sm my-2`}
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
      <div className="place-self-center text-asbrown-200 text-xs unselectable">
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
    } else if (props.messages === "all") {
      let cancel = (window as any).getAllMessagesStreamed(
        (messages: Message[]) => {
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
        }
      );
      return cancel;
    } else if (props.messages === "outbox") {
      (window as any).getOutboxMessages().then((messages: Message[]) => {
        setMessages(messages);
      });
    } else if (props.messages === "sent") {
      (window as any).getSentMessages().then((messages: Message[]) => {
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
      {messages.length === 0 && (
        <NoMessages explanation={noMessageExplanation} />
      )}
    </div>
  );
}

export default MessageList;
