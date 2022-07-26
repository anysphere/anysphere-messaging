//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";

export interface StatusProps {
  message: string;
  action?: () => void;
  actionName: string | null;
}

interface StatusInterface {
  status: StatusProps;
  setStatus: (status: StatusProps) => void;
  display: boolean;
  setVisible: () => void;
}

export const statusContext = React.createContext<StatusInterface>(
  {
    status: {
      message: "",
      actionName: null,
    },
    setStatus: () => {},
    display: false,
    setVisible: () => {},
  }
);

export function Status(props: { status: StatusProps; onClose: () => void }): JSX.Element {
  return (
    <div
      className={`unselectable fixed bottom-5 left-4 z-50 flex flex-row gap-1 rounded-md bg-asbrown-100 px-2 py-2 text-sm text-asbrown-light`}
    >
      <div className="pl-1">{props.status.message}</div>
      <div className={`grid place-content-center`}>
        <div
          className="codicon codicon-close"
          onClick={(event) => {
            props.onClose();
            event.stopPropagation();
          }}
        ></div>
      </div>
    </div>
  );
}

export function StatusHandler(props: { children: React.ReactNode }): JSX.Element {
  const [status, setStatus] = React.useState<StatusProps>({
    message: "",
    actionName: null,
  });

  const [display, setDisplay] = React.useState(false);

  const setVisible = React.useCallback(() => {
    setDisplay(true);
    setTimeout(() => {
      setDisplay(false);
    }, 10000);
  }, [setDisplay]);

  return (
    <div>
      {display && (
        <Status
          status={status}
          onClose={() => {
            setDisplay(false);
          }}
        />
      )}
      <statusContext.Provider
        value={{
          status,
          setStatus,
          display,
          setVisible,
        }}
      >
        {props.children}
      </statusContext.Provider>
    </div>
  );
}
