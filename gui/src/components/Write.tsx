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
      <div className="place-self-center flex flex-col">
        <textarea
          className="whitespace-pre-wrap resize-none w-full focus:outline-none h-full grow bg-[#F9F7F1] h-96"
          value={content}
          onChange={(e) =>
            props.edit({
              ...props.data,
              content: e.target.value,
            })
          }
          autoFocus
        />
        <div className="flex flex-row">
          <div className="flex-1"></div>
          <label>To: </label>
          <input
            type="text"
            className="bg-[#F9F7F1] focus:outline-none pl-2"
            onChange={(e) =>
              props.edit({
                ...props.data,
                to: e.target.value,
              })
            }
            value={to}
          ></input>
          <button
            className="rounded-lg unselectable bg-[#e3e0d8] text-[#7a776d] px-3 py-1"
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
