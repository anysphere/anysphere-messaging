import * as React from "react";
import invariant from "tiny-invariant";
import { deepEqual } from "fast-equals";

import {
  Action,
  ActionId,
  ActionImpl,
  IKBarContext,
  KBarOptions,
  KBarProviderProps,
  KBarState,
} from "./types";

import { VisualState } from "./types";

/**
 * The useStore hook returns the store's state and dispatch function.
 */
type useStoreProps = KBarProviderProps;

interface ActionInterfaceOptions {
  historyManager?: History;
}

// TODO: fix this later.
export class ActionInterface {
  actions: Record<ActionId, ActionImpl> = {};
  options: ActionInterfaceOptions;

  constructor(actions: Action[] = [], options: ActionInterfaceOptions = {}) {
    this.options = options;
    this.add(actions);
  }

  add(actions: Action[]) {
    for (let i = 0; i < actions.length; i++) {
      const action = actions[i];
      if (action.parent) {
        invariant(
          this.actions[action.parent],
          `Attempted to create action "${action.name}" without registering its parent "${action.parent}" first.`
        );
      }
      this.actions[action.id] = ActionImpl.create(action, {
        history: this.options.historyManager,
        store: this.actions,
      });
    }

    return { ...this.actions };
  }

  remove(actions: Action[]) {
    actions.forEach((action) => {
      const actionImpl = this.actions[action.id];
      if (!actionImpl) return;
      let children = actionImpl.children;
      while (children.length) {
        let child = children.pop();
        if (!child) return;
        delete this.actions[child.id];
        if (child.parentActionImpl) child.parentActionImpl.removeChild(child);
        if (child.children) children.push(...child.children);
      }
      if (actionImpl.parentActionImpl) {
        actionImpl.parentActionImpl.removeChild(actionImpl);
      }
      delete this.actions[action.id];
    });

    return { ...this.actions };
  }
}

export function useStore(props: useStoreProps) {
  const optionsRef = React.useRef({
    animations: {
      enterMs: 0,
      exitMs: 0,
    },
    ...props.options,
  } as KBarOptions);

  // TODO: this doesn't work with the model we're currently using for the callbacks, since the perform functions are changing for us but this thing expects the perform functions to never change. this causes a bunch of bugs.
  const actionsInterface = React.useMemo(
    () =>
      new ActionInterface(props.actions || [], {
        historyManager: optionsRef.current.enableHistory ? history : undefined,
      }),
    []
  );

  // TODO: at this point useReducer might be a better approach to managing state.
  const [state, setState] = React.useState<KBarState>({
    searchQuery: "",
    currentRootActionId: null,
    visualState: VisualState.hidden,
    actions: { ...actionsInterface.actions },
    activeIndex: 0,
  });

  const currState = React.useRef(state);
  currState.current = state;

  const getState = React.useCallback(() => currState.current, []);
  const publisher = React.useMemo(() => new Publisher(getState), [getState]);

  React.useEffect(() => {
    currState.current = state;
    publisher.notify();
  }, [state, publisher]);

  const registerActions = React.useCallback(
    (actions: Action[]) => {
      setState((state) => {
        return {
          ...state,
          actions: actionsInterface.add(actions),
        };
      });

      return function unregister() {
        setState((state) => {
          return {
            ...state,
            actions: actionsInterface.remove(actions),
          };
        });
      };
    },
    [actionsInterface]
  );

  return React.useMemo(() => {
    return {
      getState,
      query: {
        setCurrentRootAction: (actionId) => {
          setState((state) => ({
            ...state,
            currentRootActionId: actionId,
          }));
        },
        setVisualState: (cb) => {
          setState((state) => ({
            ...state,
            visualState: typeof cb === "function" ? cb(state.visualState) : cb,
          }));
        },
        setSearch: (searchQuery) =>
          setState((state) => ({
            ...state,
            searchQuery,
          })),
        registerActions,
        toggle: () =>
          setState((state) => ({
            ...state,
            visualState: [
              VisualState.animatingOut,
              VisualState.hidden,
            ].includes(state.visualState)
              ? VisualState.animatingIn
              : VisualState.animatingOut,
          })),
        setActiveIndex: (cb) =>
          setState((state) => ({
            ...state,
            activeIndex: typeof cb === "number" ? cb : cb(state.activeIndex),
          })),
      },
      options: optionsRef.current,
      subscribe: (collector, cb) => publisher.subscribe(collector, cb),
    } as IKBarContext;
  }, [getState, publisher, registerActions]);
}

/* The Publisher class is a class that holds a state and a list of subscribers. 
The subscribers are a list of Subscriber objects that hold a function that collects 
the state and a function that is called when the state changes.

The subscribe method sets up a new Subscriber object and adds it to the list of subscribers.
The notify method calls the notify method of each subscriber.
*/
class Publisher {
  getState;
  subscribers: Subscriber[] = [];

  constructor(getState: () => KBarState) {
    this.getState = getState;
  }

  subscribe<C>(
    collector: (state: KBarState) => C,
    onChange: (collected: C) => void
  ) {
    const subscriber = new Subscriber(
      () => collector(this.getState()),
      onChange
    );
    this.subscribers.push(subscriber);
    return this.unsubscribe.bind(this, subscriber);
  }

  unsubscribe(subscriber: Subscriber) {
    if (this.subscribers.length) {
      const index = this.subscribers.indexOf(subscriber);
      if (index > -1) {
        return this.subscribers.splice(index, 1);
      }
    }

    return;
  }

  notify() {
    this.subscribers.forEach((subscriber) => subscriber.collect());
  }
}

/* The Subscriber class is a class that collects the latest state of a component and passes it to a
callback function. */
class Subscriber {
  collected: any;
  collector;
  onChange;

  constructor(collector: () => any, onChange: (collected: any) => any) {
    this.collector = collector;
    this.onChange = onChange;
  }

  /**
   * Collect the latest state of the component and pass it to the onChange function.
   */
  collect() {
    try {
      // grab latest state
      const recollect = this.collector();

      if (!deepEqual(recollect, this.collected)) {
        this.collected = recollect;
        if (this.onChange) {
          this.onChange(this.collected);
        }
      }
    } catch (error) {
      console.warn(error);
    }
  }
}
