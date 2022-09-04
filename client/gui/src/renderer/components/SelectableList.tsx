//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

import * as React from "react";
import { useVirtual } from "react-virtual";

import { usePointerMovedSinceMount } from "../utils";

export interface ListItem<T> {
  id: string | number;
  data: T;
  action: (() => void) | null;
}

// string is a divider thing
interface RenderParams<T, TT = ListItem<T> | string> {
  item: TT;
  active: boolean;
}

interface SelectableListProps<T, TT = ListItem<T> | string> {
  items: TT[];
  onRender: (params: RenderParams<T>) => React.ReactElement;
  globalAction?: () => void;
  searchable: boolean;
}

export function SelectableList<T>({
  items,
  onRender,
  globalAction,
  searchable,
}: SelectableListProps<T>): JSX.Element {
  const activeRef = React.useRef<HTMLDivElement>(null);
  const parentRef = React.useRef(null);

  let startIndex = 0;
  while (typeof items[startIndex] === "string") startIndex++;

  const [activeIndex, setActiveIndex] = React.useState(startIndex);

  // store a ref to all items so we do not have to pass
  // them as a dependency when setting up event listeners.
  const itemsRef = React.useRef(items);
  itemsRef.current = items;

  const rowVirtualizer = useVirtual({
    size: itemsRef.current.length,
    parentRef,
  });

  const previousIndex = React.useCallback(
    (oldIndex: number) => {
      let nextIndex = oldIndex > startIndex ? oldIndex - 1 : oldIndex;
      while (typeof items[nextIndex] === "string") {
        if (nextIndex === startIndex) break;
        nextIndex -= 1;
      }
      return nextIndex;
    },
    [items, startIndex]
  );

  const nextIndex = React.useCallback(
    (oldIndex: number) => {
      let nextIndex = oldIndex < items.length - 1 ? oldIndex + 1 : oldIndex;
      while (typeof items[nextIndex] === "string") {
        if (nextIndex === items.length - 1) break;
        nextIndex += 1;
      }
      return nextIndex;
    },
    [items]
  );

  React.useEffect(() => {
    if (activeIndex > items.length - 1 && items.length > 0) {
      setActiveIndex(items.length - 1);
    }
  }, [items.length, activeIndex, setActiveIndex]);

  // Handle keyboard up and down events.
  React.useEffect(() => {
    const handler = (event: KeyboardEvent): void => {
      if (
        event.key === "ArrowUp" ||
        ((event.ctrlKey || !searchable) && event.key === "k")
      ) {
        setActiveIndex((old) => {
          console.log("oldIndex: " + old);
          return previousIndex(old);
        });
      } else if (
        event.key === "ArrowDown" ||
        ((event.ctrlKey || !searchable) && event.key === "j")
      ) {
        setActiveIndex((old) => {
          console.log("oldIndex: " + old);
          console.log("nextIndex: " + nextIndex(old));
          return nextIndex(old);
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
  }, [setActiveIndex, previousIndex, nextIndex, searchable]);

  // destructuring here to prevent linter warning to pass
  // entire rowVirtualizer in the dependencies array.
  const { scrollToIndex } = rowVirtualizer;
  React.useEffect(() => {
    scrollToIndex(activeIndex, {
      // ensure that if the first item in the list is a group
      // name and we are focused on the second item, to not
      // scroll past that group, hiding it.
      align: activeIndex <= 1 ? "end" : "auto",
    });
  }, [activeIndex, scrollToIndex]);

  const execute = React.useCallback(
    (item: RenderParams<T>["item"]) => {
      if (typeof item === "string") return;
      if (item.action) {
        item.action();
        if (globalAction) globalAction();
        // if you click divider, you should only display things in that section
        // TODO(sualeh): this is a hack, we should be able to do this
        //   } else {
        //     query.setSearch("");
        //     query.setCurrentRootAction(item.id);
      }
    },
    [globalAction]
  );

  const pointerMoved = usePointerMovedSinceMount();

  return (
    <div>
      <div ref={parentRef} className="relative max-h-full">
        <div
          role="listbox"
          className="w-full"
          /* This is a hack to make the listbox scrollable. */
          style={{
            height: `${rowVirtualizer.totalSize}px`,
          }}
        >
          {rowVirtualizer.virtualItems.map((virtualRow) => {
            const item = itemsRef.current[virtualRow.index] ?? "";
            const handlers = typeof item !== "string" && {
              onPointerMove: () =>
                pointerMoved &&
                activeIndex !== virtualRow.index &&
                setActiveIndex(virtualRow.index),
              onPointerDown: () => setActiveIndex(virtualRow.index),
              onClick: () => execute(item),
            };
            const active = virtualRow.index === activeIndex;

            return (
              <div
                ref={active ? activeRef : null}
                role="option"
                aria-selected={active}
                key={virtualRow.index}
                className="absolute top-0 left-0 w-full text-asbrown-light"
                style={{
                  transform: `translateY(${virtualRow.start}px)`,
                }}
                {...handlers}
              >
                {/* This is a hack to make the list resize when a single item changes. See https://github.com/TanStack/virtual/issues/85. 
                React-virtual v3 has the function virtualizer.measure() which we could call instead, and which would be better than this.
                When that becomes stable we should use that instead.
                */}
                <div ref={(element) => virtualRow.measureRef(element)}>
                  {onRender({ item, active })}
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
}
