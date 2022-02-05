import * as React from "react";
import Modal from "../Modal";
import { VisualState } from "./types";
import { useKBar } from "./useKBar";

interface Props {
  children: React.ReactNode;
}

export function CmdKPortal(props: Props, onClose: () => void) {
  const { showing } = useKBar((state) => ({
    showing: state.visualState !== VisualState.hidden,
  }));

  if (!showing) {
    return null;
  }

  return <Modal onClose={onClose}>{props.children}</Modal>;
}
