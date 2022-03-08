import * as React from "react";

// This enum is used to hold the names of different buttons
export enum SideBarButton {
  INBOX = "INBOX",
  OUTBOX = "OUTBOX",
  SENT = "SENT",
}

export interface SideBarProps {
  title: string;
  open: boolean;
  setOpen: (open: boolean) => void;
  // return a function
  sideBarCallback: (b: SideBarButton) => (() => void);
}
