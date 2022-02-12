// TODO: there should be some sort of status manager
// a global setStatus should exist, probably through some context
// should be an enum of acceptable statuses, global list

import * as React from "react";

interface StatusProps {
  message: string;
  action: () => void;
  actionName: string | null;
}

interface StatusInterface {
  setStatus: (status: string) => void;
}

export const StatusContext = React.createContext<StatusInterface>(
  {} as StatusInterface
);

export function Status(props: StatusProps) {
  const [visible, setVisible] = React.useState(true);

  const onClick = React.useCallback(() => {
    setVisible(false);
    props.action();
  }, [props.action]);

  return (
    <div
      className={`fixed top-0 left-0 right-0 bottom-0 bg-black opacity-50 z-50 ${
        visible ? "" : "hidden"
      }`}
    >
      <div
        className={`fixed top-0 left-0 right-0 bottom-0 flex items-center justify-center ${
          visible ? "" : "hidden"
        }`}
      >
        <div
          className={`bg-white rounded-lg shadow-lg p-4 ${
            props.actionName === null ? "" : "flex"
          }`}
        >
          <div className="flex-1">
            <div className="text-center">{props.message}</div>
          </div>
          {props.actionName !== null && (
            <div className="flex-none">
              <button className="btn btn-primary" onClick={onClick}>
                {props.actionName}
              </button>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

export function StatusHandler(props: { children: React.ReactNode }) {
  const [status, setStatus] = React.useState<StatusProps | null>(null);

  const handleStatus = React.useCallback((status: StatusProps) => {
    setStatus(status);
  }, []);

  return (
    <div>
      {status && (
        <Status
          message={status.message}
          action={status.action}
          actionName={status.actionName}
        />
      )}
      <StatusContext.Provider value={{ handleStatus }}>
        {props.children}
      </StatusContext.Provider>
    </div>
  );
}
