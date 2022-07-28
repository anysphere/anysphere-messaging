//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { render } from "react-dom";
import Main from "./Main";
import { InitialModal } from "./Main";
import "./index.css";

// get path
const url = new URL(window.location.href);
const params = new URLSearchParams(url.search);
const publicID = params.get("publicID");

let component;
if (publicID != null) {
  component = (
    <Main
      initialModal={InitialModal.AddFriendByPublicId}
      initialModalData={publicID}
    />
  );
} else {
  component = <Main />;
}

render(component, document.getElementById("root"));
