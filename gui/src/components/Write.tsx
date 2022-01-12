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
    <div>
      <div className="place-self-center">
        <textarea
          className="whitespace-pre-wrap resize-none w-full"
          value={content}
          onChange={(e) =>
            props.edit({
              ...props.data,
              content: e.target.value,
            })
          }
        />
        <div className="flex flex-row">
          <div className="flex-1"></div>
          <input
            type="text"
            onChange={(e) =>
              props.edit({
                ...props.data,
                to: e.target.value,
              })
            }
            value={to}
          ></input>
          <button onClick={() => props.send(content, to)}>Send</button>
        </div>
      </div>
    </div>
  );
}

export default Write;
