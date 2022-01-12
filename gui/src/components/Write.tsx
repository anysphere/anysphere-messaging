import * as React from "react";

function Write(props: {
  editCallback: (content: string) => void;
  draft: string;
}) {
  return (
    <>
      <h1>write a message!</h1>
      <div
        contentEditable={true}
        onBlur={(e) => props.editCallback(e.currentTarget.textContent)}
        className="whitespace-pre-wrap"
      >
        {props.draft}
      </div>
      <div className="flex flex-row">
        <div className="flex-1"></div>
        <button>Send</button>
      </div>
    </>
  );
}

export default Write;
