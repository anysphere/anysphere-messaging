//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

import * as React from "react";
import { InternalEvents } from "./InternalEvents";
import { IKBarContext, KBarProviderProps } from "./types";
import { useStore } from "./UseStore";

export const KBarContext = React.createContext<IKBarContext>(
  {} as IKBarContext
);

export const CmdK: React.FC<KBarProviderProps> = (props) => {
  const contextValue = useStore(props);

  return (
    <KBarContext.Provider value={contextValue}>
      <InternalEvents />
      {props.children}
    </KBarContext.Provider>
  );
};
