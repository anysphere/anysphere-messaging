//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as daemon_pb from "daemon/schema/daemon_pb";
import * as React from "react";
import { Friend } from "../../../types";
import { StatusProps } from "../Status";
import { useSearch, useFocus, classNames } from "../../utils";
import { SelectableList, ListItem } from "../SelectableList";
import { Editor } from "./Editor";
import { $getRoot, EditorState } from "lexical";
import {$convertToMarkdownString} from '@lexical/markdown';

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

export type WriteFriend = {
  uniqueName: string;
  displayName: string;
  publicId?: string;
};

export type MultiSelectData = {
  friends: WriteFriend[];
  text: string;
};

export type WriteData = {
  content: string;
  multiSelectState: MultiSelectData;
  focus: "content" | "to";
};

function MultiSelect(props: {
  options: WriteFriend[];
  multiSelectState: MultiSelectData;
  onNext: () => void;
  onEdit: (state: MultiSelectData) => void;
  onClick: () => void;
  focused: boolean;
  className: string;
}): JSX.Element {
  const filteredOptions = useSearch(
    props.options,
    props.multiSelectState.text,
    ["uniqueName", "displayName"]
  );

  const selectableOptions: (ListItem<string> | string)[] = filteredOptions.map(
    (friend) => {
      return {
        id: friend.uniqueName,
        action: () => {
          console.log("action!");
          let oldFriends = props.multiSelectState.friends;
          if (!oldFriends.find((f) => f.uniqueName === friend.uniqueName)) {
            oldFriends = [...oldFriends, friend];
          }
          props.onEdit({
            ...props.multiSelectState,
            friends: oldFriends,
            text: "",
          });
        },
        data: friend.displayName,
      };
    }
  );

  if (
    selectableOptions.length === 0 &&
    props.multiSelectState.text.length > 0
  ) {
    selectableOptions.push(
      `No contacts matching ${props.multiSelectState.text}`
    );
  } else {
    if (
      props.options.length === 0 &&
      props.multiSelectState.friends.length === 0
    ) {
      selectableOptions.push("No contacts. Add some!");
    }
  }

  const [inputRef, setInputRef] = useFocus();

  let selectBox = undefined;
  if (props.focused && selectableOptions.length > 0) {
    selectBox = (
      <div
        className="mt-1 max-h-32 overflow-scroll"
        onClick={(e) => e.stopPropagation()}
      >
        <SelectableList
          items={selectableOptions}
          searchable={true}
          onRender={({ item, active }) =>
            typeof item === "string" ? (
              <div className="unselectable text-xs text-asbrown-300">
                {item}
              </div>
            ) : (
              <div
                className={`mx-auto border-l-4 px-2 py-1 text-sm ${
                  active ? "border-asbrown-100 bg-asbeige" : "border-white"
                }`}
              >
                {item.data}
              </div>
            )
          }
        />
      </div>
    );
  }

  React.useEffect(() => {
    if (props.focused) {
      setInputRef();
    }
  }, [props.focused, setInputRef]);

  return (
    <div className={`${props.className}`} onClick={props.onClick}>
      <div className="grid pl-2">
        <div className="flex flex-row gap-2">
          {props.multiSelectState.friends.map((friend) => {
            return (
              <div
                className="shrink-0 rounded-full bg-asbeige px-1 text-sm text-asbrown-dark"
                key={friend.uniqueName}
              >
                {friend.displayName}
              </div>
            );
          })}
          <input
            type="text"
            className={classNames(
              "w-full border-0 p-0 text-sm placeholder:text-asbrown-100 focus:outline-none focus:ring-0",
              !props.focused && props.multiSelectState.text.length > 0
                ? "underline decoration-red-500 decoration-dotted decoration-2 underline-offset-2"
                : ""
            )}
            onChange={(e) =>
              props.onEdit({
                ...props.multiSelectState,
                text: e.target.value,
              })
            }
            spellCheck={false}
            onKeyDown={(e) => {
              if (
                e.key === "Backspace" &&
                props.multiSelectState.text.length === 0
              ) {
                console.log("delete");
                props.onEdit({
                  ...props.multiSelectState,
                  friends: props.multiSelectState.friends.slice(0, -1),
                });
              }
            }}
            ref={inputRef}
            placeholder={
              props.multiSelectState.friends.length > 0
                ? ""
                : "Search for a contact..."
            }
            value={props.multiSelectState.text}
            autoFocus={props.focused}
          ></input>
        </div>
        {selectBox}
      </div>
    </div>
  );
}

function Write({
  data,
  onDone,
  edit,
  setStatus,
  onClose,
}: {
  data: WriteData;
  onDone: () => void;
  edit: (data: any) => void;
  setStatus: (status: StatusProps) => void;
  onClose: () => void;
}): JSX.Element {
  const [friends, setFriends] = React.useState<WriteFriend[]>([]);

  const editorStateRef = React.useRef<EditorState>(null);

  React.useEffect(() => {
    // get both the complete friends and the sync invitations
    // the sync invitations have verified each other so it is safe to treat as a real friend
    // in the daemon.proto we keep them separate because we still want to display progress information
    window
      .getFriendList()
      .then((friends: Friend[]) => {
        setFriends((f) => [
          ...f,
          ...friends.map((friend: Friend) => {
            return {
              uniqueName: friend.uniqueName,
              displayName: friend.displayName,
              publicId: friend.publicId,
            };
          }),
        ]);
      })
      .catch((err) => {
        console.error(err);
      });
  }, []);
  React.useEffect(() => {
    // get both the complete friends and the sync invitations
    // the sync invitations have verified each other so it is safe to treat as a real friend
    // in the daemon.proto we keep them separate because we still want to display progress information
    window
      .getOutgoingSyncInvitations(new daemon_pb.GetOutgoingSyncInvitationsRequest())
      .then(
        (
          invitations: daemon_pb.GetOutgoingSyncInvitationsResponse.AsObject
        ) => {
          setFriends((f) => [
            ...f,
            ...invitations.invitationsList.map(
              (
                friend: daemon_pb.GetOutgoingSyncInvitationsResponse.OutgoingSyncInvitationInfo.AsObject
              ) => {
                return {
                  uniqueName: friend.uniqueName,
                  displayName: friend.displayName,
                };
              }
            ),
          ]);
        }
      )
      .catch((err) => {
        console.error(err);
      });
  }, []);

  const send = React.useCallback(() => {
    if (data.multiSelectState.text.length > 0) {
      setStatus({
        message: `Unknown contact: ${data.multiSelectState.text}.`,
        actionName: null,
      });
      return;
    }
    if (data.multiSelectState.friends.length === 0) {
      setStatus({
        message: `No contacts selected.`,
        actionName: null,
      });
      return;
    }

    if (editorStateRef.current === null) {
      setStatus({
        message: `Internal error. Please try again.`,
        actionName: null,
      });
      return;
    }

    let content = "";
    editorStateRef.current.read(() => {
      content = $convertToMarkdownString();
    });

    const uniqueNames = data.multiSelectState.friends.map(
      (friend) => friend.uniqueName
    );
    const displayNames = data.multiSelectState.friends.map(
      (friend) => friend.displayName
    );
    window
      .sendMessage({
        uniqueNameList: uniqueNames,
        message: content,
      })
      .then((_) => {
        setStatus({
          message: `Message sent to ${displayNames.join(", ")}.`,
          actionName: null,
        });
        onDone();
      })
      .catch((err) => {
        console.error(err);
        setStatus({
          message: `Message failed to send.`,
          actionName: null,
        });
      });
  }, [
    data.multiSelectState.text,
    data.multiSelectState.friends,
    setStatus,
    onDone,
  ]);

  React.useEffect(() => {
    const handler = (event: KeyboardEvent): void => {
      if (event.key === "Tab") {
        event.preventDefault();
        edit({
          ...data,
          focus: data.focus === "to" ? "content": "content",
        });
      } else if (event.metaKey && event.key === "Enter") {
        event.preventDefault();
        send();
      } else if (event.key === "Escape") {
        event.preventDefault();
        onClose();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [data, edit, onClose, send]);

  return (
    <div
      className={classNames(
        "mt-8 flex w-full place-content-center",
        DEBUG_COLORS ? "bg-yellow-300" : ""
      )}
    >
      <div
        className={classNames(
          "flex w-full max-w-3xl flex-col bg-white p-2 px-4",
          DEBUG_COLORS ? "bg-gray-200" : ""
        )}
      >
        <div className="py-2">
          <div className="flex flex-row content-center items-start">
            <div className="grid place-content-center">
              <div className="unselectable text-sm">To:</div>
            </div>
            <MultiSelect
              className="flex-1"
              options={friends.filter((friend) => {
                return !data.multiSelectState.friends.find(
                  (f) => f.uniqueName === friend.uniqueName
                );
              })}
              onEdit={(state: MultiSelectData) =>
                edit({ ...data, multiSelectState: state })
              }
              onNext={() =>
                edit({
                  ...data,
                  focus: data.focus === "content" ? "to" : "content",
                })
              }
              multiSelectState={data.multiSelectState}
              focused={data.focus === "to"}
              onClick={() => {
                edit({
                  ...data,
                  focus: "to",
                });
              }}
            />
          </div>
        </div>
        <hr className="border-asbrown-100" />
        <div className="py-2" />
        <Editor
          focused={data.focus === "content"}
          onFocus={() => {
            edit({
              ...data,
              focus: "content",
            });
          }}
          editorStateRef={editorStateRef}
        />
        <div className="flex flex-row content-center py-2">
          <div className="flex-1"></div>
          <button
            className="unselectable rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
            onClick={send}
          >
            Send
          </button>
        </div>
      </div>
    </div>
  );
}

export default Write;
