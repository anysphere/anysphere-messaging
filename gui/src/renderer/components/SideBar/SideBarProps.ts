import * as React from "react";

export interface SideBarProps {
  title: string;
  open: boolean;
  setOpen: (open: boolean) => void;
  children: React.ReactNode;
}
