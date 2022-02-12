import { NONAME } from "dns";
import * as React from "react";
import Modal from "../Modal";
import { VisualState } from "./types";
import { useKBar } from "./useKBar";

interface Props {
  children: React.ReactNode;
  onClose: () => void;
}

export function CmdKPortal(props: Props) {
  const { showing } = useKBar((state) => ({
    showing: state.visualState !== VisualState.hidden,
  }));

  if (!showing) {
    return null;
  }

  return <Modal onClose={props.onClose}>{props.children}</Modal>;
}
