//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { VisualState } from "./types";
import { useKBar } from "./UseKBar";

export const KBAR_LISTBOX = "kbar-listbox";
export const getListboxItemId = (id: number) => `kbar-listbox-item-${id}`;

export function CmdKSearch(
  props: React.InputHTMLAttributes<HTMLInputElement> & {
    defaultPlaceholder?: string;
  }
) {
  const {
    query,
    search,
    actions,
    currentRootActionId,
    activeIndex,
    showing,
    options,
  } = useKBar((state) => ({
    search: state.searchQuery,
    currentRootActionId: state.currentRootActionId,
    actions: state.actions,
    activeIndex: state.activeIndex,
    showing: state.visualState === VisualState.showing,
  }));

  const ownRef = React.useRef<HTMLInputElement>(null);

  React.useEffect(() => {
    query.setSearch("");
    ownRef.current!.focus();
    return () => query.setSearch("");
  }, [currentRootActionId, query]);

  const placeholder = React.useMemo((): string => {
    const defaultText = props.defaultPlaceholder ?? "Type a command or search…";
    return currentRootActionId
      ? actions[currentRootActionId].name
      : defaultText;
  }, [actions, currentRootActionId, props.defaultPlaceholder]);

  return (
    <div className="flex flex-row">
      <input
        className="mb-2 flex-grow pl-2 placeholder:text-asbrown-100 focus:outline-none"
        ref={ownRef}
        autoFocus
        autoComplete="off"
        role="combobox"
        spellCheck="false"
        aria-expanded={showing}
        aria-controls={KBAR_LISTBOX}
        aria-activedescendant={getListboxItemId(activeIndex)}
        value={search}
        placeholder={placeholder}
        onChange={(event) => {
          props.onChange?.(event);
          query.setSearch(event.target.value);
          options?.callbacks?.onQueryChange?.(event.target.value);
        }}
        onKeyDown={(event) => {
          if (currentRootActionId && !search && event.key === "Backspace") {
            const parent = actions[currentRootActionId].parent;
            query.setCurrentRootAction(parent);
          }
        }}
        {...props}
      />
    </div>
  );
}
