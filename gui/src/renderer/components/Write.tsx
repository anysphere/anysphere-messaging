//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { Friend } from "../types";
import { useSearch, useFocus } from "../utils";
import { SelectableList, ListItem } from "./SelectableList";

type MultiSelectData = {
  text: string;
};

type WriteData = {
  content: string;
  multiSelectState: MultiSelectData;
  focus: "content" | "to";
};

function MultiSelect(props: {
  options: Friend[];
  multiSelectState: MultiSelectData;
  onSelect: (state: MultiSelectData) => void;
  onEdit: (state: MultiSelectData) => void;
  onClick: () => void;
  focused: boolean;
  className: string;
}) {
  const filteredOptions = useSearch(
    props.options,
    props.multiSelectState.text,
    ["name"]
  );

  let selectableOptions: (ListItem<string> | string)[] = filteredOptions.map(
    (friend) => {
      return {
        id: friend.name,
        action: () => {
          console.log("action!");
          props.onSelect({
            ...props.multiSelectState,
            text: friend.name,
          });
        },
        data: friend.name,
      };
    }
  );

  if (selectableOptions.length === 0) {
    selectableOptions.push(
      `No friends matching ${props.multiSelectState.text}`
    );
  }

  const [inputRef, setInputRef] = useFocus();

  let selectBox = undefined;
  if (props.focused) {
    selectBox = (
      <div
        className="mt-1 max-h-32 overflow-scroll"
        onClick={(e) => e.stopPropagation()}
      >
        <SelectableList
          items={selectableOptions}
          searchable={true}
          globalAction={() => {}}
          onRender={({ item, active }) =>
            typeof item === "string" ? (
              <div className="unselectable text-asbrown-300 text-xs">
                {item}
              </div>
            ) : (
              <div
                className={`text-sm px-2 py-1 mx-auto border-l-4 ${
                  active ? "bg-asbeige border-asbrown-100" : "border-white"
                }`}
              >
                {item.data}
              </div>
            )
          }
        />
      </div>
    );
  }

  React.useEffect(() => {
    if (props.focused) {
      setInputRef();
    }
  }, [props.focused, setInputRef]);

  return (
    <div className={`${props.className}`} onClick={props.onClick}>
      <div className="grid pl-2">
        <input
          type="text"
          className="focus:outline-none w-full placeholder:text-asbrown-100 text-sm"
          onChange={(e) =>
            props.onEdit({
              ...props.multiSelectState,
              text: e.target.value,
            })
          }
          ref={inputRef}
          placeholder="Search for a friend..."
          value={props.multiSelectState.text}
          autoFocus={props.focused}
        ></input>
        {selectBox}
      </div>
    </div>
  );
}

function Write(props: {
  data: WriteData;
  send: (content: string, to: string) => void;
  edit: (data: any) => void;
  onClose: () => void;
}) {
  const content = props.data.content;
  const to = props.data.multiSelectState.text;

  const [friends, setFriends] = React.useState<Friend[]>([]);

  const [contextTestareaFocusRef, setContextTestareaFocusRef] = useFocus();

  React.useEffect(() => {
    (window as any).getFriendList().then((friends: Friend[]) => {
      setFriends(friends);
    });
  }, []);

  const send = React.useCallback(() => {
    if (to === "") {
      console.log("not sending! need to select whom to send to");
      return;
    }
    props.send(content, to);
  }, [content, to]);

  React.useEffect(() => {
    const handler = (event: any) => {
      if (event.key === "Tab") {
        event.preventDefault();
        props.edit({
          ...props.data,
          focus: props.data.focus === "content" ? "to" : "content",
        });
      } else if (event.metaKey && event.key === "Enter") {
        event.preventDefault();
        send();
      } else if (event.key === "Escape") {
        event.preventDefault();
        props.onClose();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [props, send]);

  React.useEffect(() => {
    if (props.data.focus === "content") {
      setContextTestareaFocusRef();
    }
  }, [props.data.focus, setContextTestareaFocusRef]);

  return (
    <div className="flex place-content-center w-full mt-8">
      <div className="place-self-center flex flex-col w-full max-w-3xl bg-white p-2 px-4">
        <div className="py-2">
          <div className="flex flex-row content-center items-start">
            <div className="place-content-center grid">
              <div className="text-sm unselectable">To:</div>
            </div>
            <MultiSelect
              className="flex-1"
              options={friends.filter((friend) => friend.status === "added")}
              onEdit={(state: MultiSelectData) =>
                props.edit({ ...props.data, multiSelectState: state })
              }
              onSelect={(state: MultiSelectData) =>
                props.edit({
                  ...props.data,
                  multiSelectState: state,
                  focus: props.data.focus === "content" ? "to" : "content",
                })
              }
              multiSelectState={props.data.multiSelectState}
              focused={props.data.focus === "to"}
              onClick={() => {
                props.edit({
                  ...props.data,
                  focus: "to",
                });
              }}
            />
          </div>
        </div>
        <hr className="border-asbrown-100" />
        <textarea
          onClick={() => {
            props.edit({
              ...props.data,
              focus: "content",
            });
          }}
          className="whitespace-pre-wrap resize-none w-full focus:outline-none h-full grow h-96 pt-4 text-sm"
          value={content}
          onChange={(e) =>
            props.edit({
              ...props.data,
              content: e.target.value,
            })
          }
          autoFocus={props.data.focus === "content"}
          ref={contextTestareaFocusRef}
        />
        <div className="flex flex-row content-center py-2">
          <div className="flex-1"></div>
          <button
            className="rounded-lg unselectable bg-asbrown-100 text-asbrown-light px-3 py-1"
            onClick={send}
          >
            Send
          </button>
        </div>
      </div>
    </div>
  );
}

export default Write;
