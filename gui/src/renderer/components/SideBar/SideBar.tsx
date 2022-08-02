import * as React from "react";
import { SideBarItemGroup } from "./SideBarItemGroup";
import { SideBarButton, SideBarProps } from "./SideBarProps";

import { SelectableList, ListItem } from "../SelectableList";

import { Dialog, Transition } from "@headlessui/react";
// import { XIcon } from "@heroicons/react/outline";

/* Defining the properties of the component. */
export interface SideBarItemProps {
  title: string;
  active: boolean;
  // icon: string;
  onClick: () => void;
}

export interface InternalSideBarProps {
  title: string;
  open: boolean;
  setOpen: (open: boolean) => void;
  children: React.ReactNode;
}

export function SideBarItem(props: SideBarItemProps) {
  let active = props.active;
  return (
    <div
      className={`rounded-md py-1 px-4 text-asbrown-dark ${
        active ? "bg-asbeige" : ""
      }`}
      onClick={props.onClick}
    >
      <i className=""></i>
      <div className="">{props.title}</div>
    </div>
  );
}

export function HeadlessSlideOver({
  open,
  setOpen,
  title,
  children,
}: InternalSideBarProps) {
  return (
    <Transition.Root show={open} as={React.Fragment}>
      <Dialog
        as="div"
        static
        className="fixed inset-0 overflow-hidden"
        open={open}
        onClose={setOpen}
      >
        <div className="absolute inset-0 overflow-hidden">
          <Transition.Child
            as={React.Fragment}
            enter="ease-in-out duration-120"
            enterFrom="opacity-0"
            enterTo="opacity-100"
            leave="ease-in-out duration-100"
            leaveFrom="opacity-100"
            leaveTo="opacity-0"
          >
            <Dialog.Overlay className="absolute inset-0 bg-asbrown-100 bg-opacity-75 transition-opacity" />
          </Transition.Child>
          <div className="fixed inset-y-0 left-0 flex max-w-full pr-8">
            <Transition.Child
              as={React.Fragment}
              enter="transform transition ease-in-out duration-120"
              enterFrom="-translate-x-full"
              enterTo="translate-x-0"
              leave="transform transition ease-in-out duration-100"
              leaveFrom="translate-x-0"
              leaveTo="-translate-x-full"
            >
              <div className="relative w-screen max-w-md">
                <Transition.Child
                  as={React.Fragment}
                  enter="ease-in-out duration-120"
                  enterFrom="opacity-0"
                  enterTo="opacity-100"
                  leave="ease-in-out duration-100"
                  leaveFrom="opacity-100"
                  leaveTo="opacity-0"
                >
                  <div className="absolute top-0 right-0 ml-8 flex pt-4 pr-2 sm:ml-10 sm:pr-4">
                    <button
                      className="rounded-md text-gray-300 hover:text-asbrown-dark focus:outline-none focus:ring-2 focus:ring-asbrown-light focus:ring-opacity-50"
                      onClick={() => setOpen(false)}
                    >
                      <span className="sr-only">Close panel</span>
                      {/* <XIcon className="h-6 w-6" aria-hidden="true" /> */}
                    </button>
                  </div>
                </Transition.Child>
                <div className="flex h-full flex-col overflow-y-scroll bg-white py-6 shadow-xl">
                  <div className="px-4 sm:px-6">
                    <Dialog.Title className="text-lg  font-medium text-gray-900">
                      {title}
                    </Dialog.Title>
                  </div>
                  <div className="relative mt-6 flex-1 px-0 sm:px-6">
                    {/* Where the children live*/}
                    {children}
                  </div>
                </div>
              </div>
            </Transition.Child>
          </div>
        </div>
      </Dialog>
    </Transition.Root>
  );
}
export function SideBar(props: SideBarProps) {
  let onClick = () => {};

  let selectableOption: (
    | ListItem<{ type: "subtitle" | "item"; name: string }>
    | string
  )[] = [
    // Inbox
    {
      id: "inbox",
      action: () => {
        props.sideBarCallback(SideBarButton.INBOX);
      },
      data: {
        type: "item",
        name: "Inbox",
      },
    },
    // "Outbox",
    {
      id: "outbox",
      action: () => {
        props.sideBarCallback(SideBarButton.OUTBOX);
      },
      data: {
        type: "item",
        name: "Outbox",
      },
    },
    // "Sent",
    {
      id: "sent",
      action: () => {
        props.sideBarCallback(SideBarButton.SENT);
      },
      data: {
        type: "item",
        name: "Sent",
      },
    },
    // "Sent",
    {
      id: "add-friend",
      action: () => {
        console.log("add friend");
        props.sideBarCallback(SideBarButton.ADD_FRIEND);
      },
      data: {
        type: "item",
        name: "Add Friend",
      },
    },
    {
      id: "invitations",
      action: () => {
        props.sideBarCallback(SideBarButton.INVITATIONS);
      },
      data: {
        type: "item",
        name: "Invitations",
      },
    },
  ];

  return (
    <HeadlessSlideOver
      open={props.open}
      setOpen={props.setOpen}
      title={props.title}
    >
      <SelectableList
        items={selectableOption}
        searchable={true}
        globalAction={() => {}}
        onRender={({ item, active }) => {
          if (typeof item === "string") {
            return <SideBarItemGroup title={item} />;
          }
          // check if its a ListItem
          if (item.data.type === "item") {
            return (
              <SideBarItem
                title={item.data.name}
                onClick={onClick}
                active={active}
              />
            );
          }
          return <div> </div>;
        }}
      />
    </HeadlessSlideOver>
  );
}
