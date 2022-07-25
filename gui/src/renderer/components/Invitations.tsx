//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import {
  IncomingMessage,
  OutgoingMessage,
  OutgoingAsyncInvitation,
  OutgoingSyncInvitation,
  IncomingAsyncInvitation,
} from "../../types";
import * as daemon_pb from "../../daemon/schema/daemon_pb";
import { truncate, formatTime } from "../utils";
import { BackgroundCircles } from "./BackgroundCircles/BackgroundCircles";
import { StatusProps } from "./Status";
import { SelectableList } from "./SelectableList";
import { protobufDateToDate } from "../../types";

function EmptyState({ text }: { text: string }): JSX.Element {
  return (
    <div className="flex flex-col items-center justify-center">
      <div className="text-center text-sm text-asbrown-300">{text}</div>
    </div>
  );
}

function OutgoingAsyncInvitationComp({
  inv,
  active,
  cancelCallback,
  setStatus,
}: {
  inv: OutgoingAsyncInvitation;
  active: boolean;
  cancelCallback: () => void;
  setStatus: (status: StatusProps) => void;
}): JSX.Element {
  let timestampString = "";

  if (inv.sentAt === undefined) {
    timestampString = "Not yet sent.";
    console.error("Message has no sentAt");
  } else {
    timestampString = formatTime(protobufDateToDate(inv.sentAt));
  }

  return (
    <div
      className={`${
        active ? "border-asbrown-100 bg-asbeige" : "border-white bg-white"
      } my-2 rounded-sm border-l-4 px-4 py-4`}
    >
      <div className="flex flex-row gap-5">
        <div className="text-sm text-asbrown-dark">{`To: ${
          inv.displayName
        } (${truncate(inv.publicId, 10)})`}</div>
        <div className="text-sm text-asbrown-300">{inv.message}</div>
        <div className="flex-1"></div>
        <div className="text-sm text-asbrown-200">{timestampString}</div>
        <button
          className="h-fit rounded-lg bg-asbrown-100 px-2 py-2 text-sm text-asbrown-dark"
          onClick={() => {
            window
              .cancelAsyncInvitation({ publicId: inv.publicId })
              .then(() => {
                setStatus({
                  message: "Cancelled invitation to " + inv.displayName,
                  actionName: null,
                });
                cancelCallback();
              })
              .catch((err) => {
                setStatus({
                  message: `Failed to cancel invitation: ${err}`,
                  actionName: null,
                });
              });
          }}
        >
          Cancel
        </button>
      </div>
    </div>
  );
}

export function OutgoingInvitations({
  setStatus,
}: {
  setStatus: (status: StatusProps) => void;
}): JSX.Element {
  const [asyncInvitations, setAsyncInvitations] = React.useState<
    OutgoingAsyncInvitation[]
  >([]);

  const [refresh, setRefresh] = React.useState(true);

  React.useEffect(() => {
    if (!refresh) {
      return;
    }
    setRefresh(false);
    window
      .getOutgoingAsyncInvitations({})
      .then(
        ({
          invitationsList,
        }: {
          invitationsList: OutgoingAsyncInvitation[];
        }) => {
          setAsyncInvitations(invitationsList);
        }
      )
      .catch((err) => {
        setStatus({
          message: `Failed to get outgoing async invitations: ${err}`,
          actionName: null,
        });
      });
  }, [refresh, setStatus]);

  return (
    <div>
      <div className="mt-8 flex w-full place-content-center">
        <div className="flex w-full max-w-3xl flex-col place-self-center">
          <SelectableList
            items={asyncInvitations.map((inv) => {
              return {
                id: inv.uniqueName,
                data: inv,
                action: () => console.log("invitation clicked", inv),
              };
            })}
            searchable={false}
            onRender={({ item, active }) =>
              typeof item === "string" ? (
                <div className="unselectable">{item}</div>
              ) : (
                <OutgoingAsyncInvitationComp
                  active={active}
                  key={item.id}
                  inv={item.data}
                  setStatus={setStatus}
                  cancelCallback={() => setRefresh(true)}
                />
              )
            }
          />
        </div>
      </div>
      {asyncInvitations.length === 0 && (
        <EmptyState text="No outgoing invitations." />
      )}
    </div>
  );
}

export function Invitations({
  setStatus,
}: {
  setStatus: (status: StatusProps) => void;
}): JSX.Element {
  return <OutgoingInvitations setStatus={setStatus} />;
}
