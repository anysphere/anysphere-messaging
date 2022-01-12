import * as React from "react";

function Main() {
  (window as any).send("sualeh", "bla bla bla");
  return (
    <div>
      <h1>Hello world!</h1>
    </div>
  );
}

export default Main;
