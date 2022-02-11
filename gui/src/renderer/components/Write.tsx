//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

type WriteData = {
  content: string;
  to: string;
};

function Write(props: {
  data: WriteData;
  send: (content: string, to: string) => void;
  edit: (data: any) => void;
}) {
  const content = props.data.content;
  const to = props.data.to;
  return (
    <div className="flex place-content-center w-full mt-8">
      <div className="place-self-center flex flex-col w-full max-w-3xl bg-white p-2 px-4">
        <div className="flex flex-row content-center py-2">
          <div className="place-content-center grid">
            <div className="align-bottom">To:</div>
          </div>
          <input
            type="text"
            className="focus:outline-none pl-2"
            onChange={(e) =>
              props.edit({
                ...props.data,
                to: e.target.value,
              })
            }
            value={to}
            autoFocus
          ></input>
          <div className="flex-1"></div>
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
        />
        <div className="flex flex-row content-center py-2">
          <div className="flex-1"></div>
          <button
            className="rounded-lg unselectable bg-asbrown-100 text-asbrown-light px-3 py-1"
            onClick={() => props.send(content, to)}
          >
            Send
          </button>
        </div>
      </div>
    </div>
  );
}

export default Write;
