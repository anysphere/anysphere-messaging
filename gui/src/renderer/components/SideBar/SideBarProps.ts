// This enum is used to hold the names of different buttons
export enum SideBarButton {
  INBOX = "INBOX",
  OUTBOX = "OUTBOX",
  SENT = "SENT",
  ADD_FRIEND = "ADD_FRIEND",
}

export interface SideBarProps {
  title: string;
  open: boolean;
  setOpen: (open: boolean) => void;
  // return a function
  sideBarCallback: (b: SideBarButton) => void;
}
