//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { Friend } from "../types";

type MultiSelectData = {
  text: string;
};

type WriteData = {
  content: string;
  to: string;
  multiSelectState: MultiSelectData;
  focus: "content" | "to";
};

function MultiSelect(props: {
  options: Friend[];
  multiSelectState: MultiSelectData;
  onSelect: (selected: Friend) => void;
  onEdit: (state: MultiSelectData) => void;
  focused: boolean;
  className: string;
}) {
  const filteredOptions = props.options.filter((option) => {
    return true;
  });

  let selectBox = undefined;
  if (props.focused) {
    selectBox = (
      <>
        <h3 className="text-xs text-asbrown-200 mt-1 mb-0 unselectable">
          Select a friend
        </h3>
        {filteredOptions.map((option) => (
          <div key={option.name} className="text-sm">
            {option.name}
          </div>
        ))}
      </>
    );
  }

  return (
    <div className={props.className}>
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
}) {
  const content = props.data.content;
  const to = props.data.to;

  const [friends, setFriends] = React.useState<Friend[]>([]);

  React.useEffect(() => {
    (window as any).getFriendList().then((friends: Friend[]) => {
      setFriends(friends);
    });
  }, []);

  const send = React.useCallback(() => {
    if (to === "") {
      console.log("not sending! need to select whom to send to");
    }
    props.send(content, to);
  }, [content, to]);

  return (
    <div className="flex place-content-center w-full mt-8">
      <div className="place-self-center flex flex-col w-full max-w-3xl bg-white p-2 px-4">
        <div className="py-2">
          <div className="flex flex-row content-center items-start">
            <div className="place-content-center grid">
              <div className="align-bottom text-sm">To:</div>
            </div>
            <MultiSelect
              className="flex-1"
              options={friends}
              onEdit={(state: MultiSelectData) =>
                props.edit({ ...props.data, multiSelectState: state })
              }
              onSelect={(selected: Friend) =>
                props.edit({ ...props.data, to: selected.name })
              }
              multiSelectState={props.data.multiSelectState}
              focused={props.data.focus === "to"}
            />
          </div>
        </div>
        <hr className="border-asbrown-100" />
        <textarea
          className="whitespace-pre-wrap resize-none w-full focus:outline-none h-full grow h-96 pt-4"
          value={content}
          onChange={(e) =>
            props.edit({
              ...props.data,
              content: e.target.value,
            })
          }
          autoFocus={props.data.focus === "content"}
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
