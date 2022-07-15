//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { IncomingMessage, OutgoingMessage } from "../../types";
import * as daemon_pb from "../../daemon/schema/daemon_pb";
import { truncate, formatTime } from "../utils";
import { BackgroundCircles } from "./BackgroundCircles/BackgroundCircles";
import { SelectableList } from "./SelectableList";
import { protobufDateToDate } from "../../types";

function IncomingMessageBlurb({
  message,
  active,
}: {
  message: IncomingMessage;
  active: boolean;
}): JSX.Element {
  let timestampString = "";

  if (message.deliveredAt === undefined) {
    timestampString = "Not yet delivered.";
    console.error("Message has no deliveredAt");
  } else {
    timestampString = formatTime(protobufDateToDate(message.deliveredAt));
  }

  return (
    <div
      className={`${
        active ? "border-asbrown-100 bg-asbeige" : "border-white bg-white"
      } my-2 rounded-sm border-l-4 px-4 py-4`}
    >
      <div className="flex flex-row gap-5">
        <div className="text-sm text-asbrown-dark">
          {message.fromDisplayName}
        </div>
        <div className="text-sm text-asbrown-300">
          {truncate(message.message, 65)}
        </div>
        <div className="flex-1"></div>
        <div className="text-sm text-asbrown-200">{timestampString}</div>
      </div>
    </div>
  );
}

function OutgoingMessageBlurb({
  message,
  active,
}: {
  message: OutgoingMessage;
  active: boolean;
}): JSX.Element {
  let timestampString = "";

  if (message.sentAt === undefined) {
    timestampString = "Not yet sent.";
    console.error("Message has no sentAt");
  } else {
    timestampString = formatTime(protobufDateToDate(message.sentAt));
  }

  return (
    <div
      className={`${
        active ? "border-asbrown-100 bg-asbeige" : "border-white bg-white"
      } my-2 rounded-sm border-l-4 px-4 py-4`}
    >
      <div className="flex flex-row gap-5">
        <div className="text-sm text-asbrown-dark">{`To: ${message.toFriendsList
          .map((x) => x.displayName)
          .join(", ")}`}</div>
        <div className="text-sm text-asbrown-300">
          {truncate(message.message, 65)}
        </div>
        <div className="flex-1"></div>
        <div className="text-sm text-asbrown-200">{timestampString}</div>
      </div>
    </div>
  );
}

export function IncomingMessageList({
  type,
  readCallback,
}: {
  type: "new" | "all";
  readCallback: (message: IncomingMessage) => void;
}): JSX.Element {
  const [messages, setMessages] = React.useState<IncomingMessage[]>([]);

  React.useEffect(() => {
    if (type === "new") {
      setMessages([]);
      const cancel = window.getMessagesStreamed(
        { filter: daemon_pb.GetMessagesRequest.Filter.NEW },
        (messages: IncomingMessage[]) => {
          setMessages((prev: IncomingMessage[]) => {
            // merge new messages with old messages, and sort them by timestamp
            let newMessages = messages.concat(prev);
            newMessages = newMessages.sort((a, b) => {
              // sort based on timestamp
              const aTime = a.deliveredAt ?? new Date();
              const bTime = b.deliveredAt ?? new Date();
              if (aTime > bTime) {
                return -1;
              } else if (aTime < bTime) {
                return 1;
              } else {
                return 0;
              }
            });
            return newMessages;
          });
        }
      );

      return cancel;
    } else if (type === "all") {
      setMessages([]);
      const cancel = window.getMessagesStreamed(
        { filter: daemon_pb.GetMessagesRequest.Filter.ALL },
        (messages: IncomingMessage[]) => {
          setMessages((prev: IncomingMessage[]) => {
            // merge new messages with old messages, and sort them by timestamp
            const newMessages = messages.concat(prev);
            newMessages.sort((a, b) => {
              // sort based on timestamp
              const aTime = a.deliveredAt ?? new Date();
              const bTime = b.deliveredAt ?? new Date();
              if (aTime > bTime) {
                return -1;
              } else if (aTime < bTime) {
                return 1;
              } else {
                return 0;
              }
            });

            return newMessages;
          });
        }
      );

      return cancel;
    }

    // Error out
    throw new Error("Invalid messages type");
  }, [type]);

  return (
    <div>
      <div className="mt-8 flex w-full place-content-center">
        <div className="flex w-full max-w-3xl flex-col place-self-center">
          <SelectableList
            items={messages.map((message) => {
              return {
                id: message.uid,
                data: message,
                action: () => readCallback(message),
              };
            })}
            searchable={false}
            onRender={({ item, active }) =>
              typeof item === "string" ? (
                <div className="unselectable">{item}</div>
              ) : (
                <IncomingMessageBlurb
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

export function OutgoingMessageList({
  type,
  readCallback,
}: {
  type: "outbox" | "sent";
  readCallback: (message: OutgoingMessage) => void;
}): JSX.Element {
  const [messages, setMessages] = React.useState<OutgoingMessage[]>([]);

  React.useEffect(() => {
    if (type === "outbox") {
      window
        .getOutboxMessages({})
        .then(({ messagesList }: { messagesList: OutgoingMessage[] }) => {
          setMessages(messagesList);
        })
        .catch((err) => {
          console.error(err);
        });
    } else if (type === "sent") {
      window
        .getSentMessages({})
        .then(({ messagesList }: { messagesList: OutgoingMessage[] }) => {
          setMessages(messagesList);
        })
        .catch((err) => {
          console.error(err);
        });
    }
  }, [type]);

  return (
    <div>
      <div className="mt-8 flex w-full place-content-center">
        <div className="flex w-full max-w-3xl flex-col place-self-center">
          <SelectableList
            items={messages.map((message) => {
              return {
                id: message.uid,
                data: message,
                action: () => readCallback(message),
              };
            })}
            searchable={false}
            onRender={({ item, active }) =>
              typeof item === "string" ? (
                <div className="unselectable">{item}</div>
              ) : (
                <OutgoingMessageBlurb
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
