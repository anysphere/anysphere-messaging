import * as React from "react";
import { VisualState } from "./types";
import { useKBar } from "./useKBar";
import { isModKey, shouldRejectKeystrokes } from "./utils";

type Timeout = ReturnType<typeof setTimeout>;

export function InternalEvents() {
  useToggleHandler();
  useShortcuts();
  return null;
}

/**
 * `useToggleHandler` handles the keyboard events for toggling kbar.
 */
function useToggleHandler() {
  const { query, options, visualState, showing } = useKBar((state) => ({
    visualState: state.visualState,
    showing: state.visualState !== VisualState.hidden,
  }));

  React.useEffect(() => {
    function handleKeyDown(event: KeyboardEvent) {
      if (
        isModKey(event) &&
        event.key === "k" &&
        event.defaultPrevented === false
      ) {
        event.preventDefault();
        query.toggle();

        if (showing) {
          options.callbacks?.onClose?.();
        } else {
          options.callbacks?.onOpen?.();
        }
      }
      if (event.key === "Escape") {
        if (showing) {
          event.stopPropagation();
          options.callbacks?.onClose?.();
        }

        query.setVisualState((vs) => {
          if (vs === VisualState.hidden || vs === VisualState.animatingOut) {
            return vs;
          }
          return VisualState.animatingOut;
        });
      }
    }

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [options.callbacks, query, showing]);

  const timeoutRef = React.useRef<Timeout>();
  const runAnimateTimer = React.useCallback(
    (vs: VisualState.animatingIn | VisualState.animatingOut) => {
      let ms = 0;
      if (vs === VisualState.animatingIn) {
        ms = options.animations?.enterMs || 0;
      }
      if (vs === VisualState.animatingOut) {
        ms = options.animations?.exitMs || 0;
      }

      clearTimeout(timeoutRef.current as Timeout);
      timeoutRef.current = setTimeout(() => {
        let backToRoot = false;

        // TODO: setVisualState argument should be a function or just a VisualState value.
        query.setVisualState(() => {
          const finalVs =
            vs === VisualState.animatingIn
              ? VisualState.showing
              : VisualState.hidden;

          if (finalVs === VisualState.hidden) {
            backToRoot = true;
          }

          return finalVs;
        });

        if (backToRoot) {
          query.setCurrentRootAction(null);
        }
      }, ms);
    },
    [options.animations?.enterMs, options.animations?.exitMs, query]
  );

  React.useEffect(() => {
    switch (visualState) {
      case VisualState.animatingIn:
      case VisualState.animatingOut:
        runAnimateTimer(visualState);
        break;
    }
  }, [runAnimateTimer, visualState]);
}

/**
 * `useShortcuts` registers and listens to keyboard strokes and
 * performs actions for patterns that match the user defined `shortcut`.
 */
function useShortcuts() {
  const { actions, query, options } = useKBar((state) => ({
    actions: state.actions,
  }));

  React.useEffect(() => {
    const actionsList = Object.keys(actions).map((key) => actions[key]);

    let buffer: string[] = [];
    let lastKeyStrokeTime = Date.now();

    function handleKeyDown(event: KeyboardEvent) {
      const key = event.key?.toLowerCase();

      if (shouldRejectKeystrokes() || event.metaKey || key === "shift") {
        return;
      }

      const currentTime = Date.now();

      if (currentTime - lastKeyStrokeTime > 400) {
        buffer = [];
      }

      buffer.push(key);
      lastKeyStrokeTime = currentTime;
      const bufferString = buffer.join("");

      for (let action of actionsList) {
        if (!action.shortcut) {
          continue;
        }
        if (action.shortcut.join("") === bufferString) {
          event.preventDefault();
          if (action.children?.length) {
            query.setCurrentRootAction(action.id);
            query.toggle();
            options.callbacks?.onOpen?.();
          } else {
            action.command?.perform();
            options.callbacks?.onSelectAction?.(action);
          }

          buffer = [];
          break;
        }
      }
    }

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [actions, query]);
}
