//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import * as React from "react";
import AddFriendChoice from "./AddFriendChoice";
import AddFriendInPerson from "./AddFriendInPerson";
import AddFriendRemote from "./AddFriendRemote";
import AddFriendRemotePartTwo from "./AddFriendRemotePartTwo";
import AddFriendIncoming from "./AddFriendIncoming";
import { IncomingAsyncInvitation } from "types";

export enum AddFriendScreen {
  Choice,
  InPerson,
  Remote,
  RemotePartTwo,
  Incoming,
}

export default function AddFriend({
  onClose,
  setStatus,
  initialScreen,
  incomingInvitation,
  theirPublicId,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
  initialScreen?: AddFriendScreen;
  incomingInvitation?: IncomingAsyncInvitation;
  theirPublicId?: string;
}): JSX.Element {
  const [screen, setScreen] = React.useState<AddFriendScreen>(
    initialScreen ?? AddFriendScreen.Choice
  );
  const [story, setStory] = React.useState<string>("");
  const [publicID, setPublicID] = React.useState<string>("");
  const [theirID, setTheirID] = React.useState<string>(theirPublicId ?? "");

  React.useEffect(() => {
    window
      .getMyPublicID()
      .then((publicID) => {
        setStory(publicID.story);
        setPublicID(publicID.publicId);
      })
      .catch((err) => {
        setStatus({
          message: `Internal error: ${err}`,
          actionName: null,
        });
      });
  }, [setStatus]);

  let component;
  switch (screen) {
    case AddFriendScreen.Choice:
      component = (
        <AddFriendChoice
          onClose={onClose}
          setStatus={setStatus}
          chooseInperson={() => {
            setScreen(AddFriendScreen.InPerson);
          }}
          chooseRemote={() => {
            setScreen(AddFriendScreen.Remote);
          }}
        />
      );
      break;
    case AddFriendScreen.InPerson:
      component = (
        <AddFriendInPerson
          onClose={onClose}
          setStatus={setStatus}
          story={story}
        />
      );
      break;
    case AddFriendScreen.Remote:
      component = (
        <AddFriendRemote
          onClose={onClose}
          setStatus={setStatus}
          publicId={publicID}
          onPastePublicId={(publicId: string) => {
            setTheirID(publicId);
            setScreen(AddFriendScreen.RemotePartTwo);
          }}
        />
      );
      break;
    case AddFriendScreen.RemotePartTwo:
      component = (
        <AddFriendRemotePartTwo
          onClose={onClose}
          setStatus={setStatus}
          publicId={publicID}
          theirId={theirID}
        />
      );
      break;
    case AddFriendScreen.Incoming:
      if (incomingInvitation === undefined) {
        throw new Error("invalid state");
      }
      component = (
        <AddFriendIncoming
          onClose={onClose}
          setStatus={setStatus}
          publicId={publicID}
          inv={incomingInvitation}
        />
      );
      break;
  }

  return (
    <Modal onClose={onClose} type={ModalType.Large}>
      <div className="justify-content-center grid h-full w-full grid-cols-1 items-center">
        <div className="mx-auto h-[min(40rem,100%)] w-[min(60rem,100%)] self-center">
          {component}
        </div>
      </div>
    </Modal>
  );
}
