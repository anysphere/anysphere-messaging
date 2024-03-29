//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

import * as React from "react";
import { useVirtual } from "react-virtual";

import { ActionImpl } from "./types";
import { getListboxItemId, KBAR_LISTBOX } from "./CmdKSearch";
import { useKBar } from "./UseKBar";
import { usePointerMovedSinceMount } from "../../utils";

const START_INDEX = 0;

interface RenderParams<T = ActionImpl | string> {
  item: T;
  active: boolean;
}

interface KBarResultsProps {
  items: any[];
  onRender: (params: RenderParams) => React.ReactElement;
  maxHeight?: number;
}

export const CmdKResultHandler: React.FC<KBarResultsProps> = (props) => {
  const activeRef = React.useRef<HTMLDivElement>(null);
  const parentRef = React.useRef(null);

  // store a ref to all items so we do not have to pass
  // them as a dependency when setting up event listeners.
  const itemsRef = React.useRef(props.items);
  itemsRef.current = props.items;

  const rowVirtualizer = useVirtual({
    size: itemsRef.current.length,
    parentRef,
  });

  const { query, search, currentRootActionId, activeIndex, options } = useKBar(
    (state) => ({
      search: state.searchQuery,
      currentRootActionId: state.currentRootActionId,
      activeIndex: state.activeIndex,
    })
  );

  // Handle keyboard up and down events.
  React.useEffect(() => {
    const handler = (event: any) => {
      if (event.key === "ArrowUp" || (event.ctrlKey && event.key === "k")) {
        event.preventDefault();
        query.setActiveIndex((index) => {
          let nextIndex = index > START_INDEX ? index - 1 : index;
          // avoid setting active index on a group
          if (typeof itemsRef.current[nextIndex] === "string") {
            if (nextIndex === 0) return index;
            nextIndex -= 1;
          }
          return nextIndex;
        });
      } else if (
        event.key === "ArrowDown" ||
        (event.ctrlKey && event.key === "j")
      ) {
        event.preventDefault();
        query.setActiveIndex((index) => {
          let nextIndex =
            index < itemsRef.current.length - 1 ? index + 1 : index;
          // avoid setting active index on a group
          if (typeof itemsRef.current[nextIndex] === "string") {
            if (nextIndex === itemsRef.current.length - 1) return index;
            nextIndex += 1;
          }
          return nextIndex;
        });
      } else if (event.key === "Enter") {
        event.preventDefault();
        // storing the active dom element in a ref prevents us from
        // having to calculate the current action to perform based
        // on the `activeIndex`, which we would have needed to add
        // as part of the dependencies array.
        activeRef.current?.click();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [query]);

  // destructuring here to prevent linter warning to pass
  // entire rowVirtualizer in the dependencies array.
  const { scrollToIndex } = rowVirtualizer;
  React.useEffect(() => {
    console.log("scroll to index pls");
    scrollToIndex(activeIndex, {
      // ensure that if the first item in the list is a group
      // name and we are focused on the second item, to not
      // scroll past that group, hiding it.
      align: activeIndex <= 1 ? "end" : "center",
    });
  }, [activeIndex, scrollToIndex]);

  React.useEffect(() => {
    // TODO(sualeh): fix scenario where async actions load in
    // and active index is reset to the first item. i.e. when
    // users register actions and bust the `useRegisterActions`
    // cache, we won't want to reset their active index as they
    // are navigating the list.
    query.setActiveIndex(
      // avoid setting active index on a group
      typeof props.items[START_INDEX] === "string"
        ? START_INDEX + 1
        : START_INDEX
    );
  }, [search, currentRootActionId, props.items, query]);

  const execute = React.useCallback(
    (item: RenderParams["item"]) => {
      if (typeof item === "string") return;
      if (item.command) {
        item.command.perform(item);
        query.toggle();
      } else {
        query.setSearch("");
        query.setCurrentRootAction(item.id);
      }
      options.callbacks?.onSelectAction?.(item);
    },
    [query, options]
  );

  const pointerMoved = usePointerMovedSinceMount();

  return (
    <div>
      <div ref={parentRef} className="relative max-h-full overflow-auto">
        <div
          role="listbox"
          id={KBAR_LISTBOX}
          // TODO(sualeh): ask Arvid about this.
          className="w-full"
          /* This is a hack to make the listbox scrollable. */
          style={{
            height: `${rowVirtualizer.totalSize}px`,
          }}
        >
          {rowVirtualizer.virtualItems.map((virtualRow) => {
            const item = itemsRef.current[virtualRow.index];
            const handlers = typeof item !== "string" && {
              onPointerMove: () =>
                pointerMoved &&
                activeIndex !== virtualRow.index &&
                query.setActiveIndex(virtualRow.index),
              onPointerDown: () => query.setActiveIndex(virtualRow.index),
              onClick: () => execute(item),
            };
            const active = virtualRow.index === activeIndex;

            return (
              <div
                ref={active ? activeRef : null}
                id={getListboxItemId(virtualRow.index)}
                role="option"
                aria-selected={active}
                key={virtualRow.index}
                className="absolute top-0 left-0 h-8 w-full text-asbrown-light"
                style={{
                  transform: `translateY(${virtualRow.start}px)`,
                }}
                {...handlers}
              >
                {React.cloneElement(
                  props.onRender({
                    item,
                    active,
                  }),
                  {
                    ref: virtualRow.measureRef,
                  }
                )}
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
