//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { Command } from "./Command";

export type ActionId = string;

export type Action = {
  id: ActionId;
  name: string;
  shortcut?: string[];
  keywords?: string;
  section?: string;
  icon?: string | React.ReactElement | React.ReactNode;
  subtitle?: string;
  perform?: (currentActionImpl: ActionImpl) => any;
  parent?: ActionId;
};

export type ActionStore = Record<ActionId, ActionImpl>;

interface ActionImplOptions {
  store: ActionStore;
  ancestors?: ActionImpl[];
  history?: History;
}

/**
 * Extends the configured keywords to include the section
 * This allows section names to be searched for.
 */
const extendKeywords = ({ keywords = "", section = "" }: Action): string => {
  return `${keywords} ${section}`.trim();
};

export class ActionImpl implements Action {
  id: Action["id"];
  name: Action["name"];
  shortcut: Action["shortcut"];
  keywords: Action["keywords"];
  section: Action["section"];
  icon: Action["icon"];
  subtitle: Action["subtitle"];
  parent?: Action["parent"];
  // TODO(sualeh, urgent): remove this.
  /**
   * @deprecated use action.command.perform
   */
  perform: Action["perform"];

  // TODO: fix
  command?: Command;

  ancestors: ActionImpl[] = [];
  children: ActionImpl[] = [];

  constructor(action: Action, options: ActionImplOptions) {
    Object.assign(this, action);
    this.id = action.id;
    this.name = action.name;
    this.keywords = extendKeywords(action);

    const perform = action.perform;

    //   const perform = action.perform;
    this.command =
      perform &&
      new Command({
        perform: () => perform(this),
      });
    // Backwards compatibility
    this.perform = this.command?.perform;

    if (action.parent) {
      const parentActionImpl = options.store[action.parent];
      // invariant(
      //   parentActionImpl,
      //   `attempted to create an action whos parent: ${action.parent} does not exist in the store.`
      // );
      parentActionImpl.addChild(this);
    }
  }

  addChild(childActionImpl: ActionImpl) {
    // add all ancestors for the child action
    childActionImpl.ancestors.unshift(this);
    let parent = this.parentActionImpl;
    while (parent) {
      childActionImpl.ancestors.unshift(parent);
      parent = parent.parentActionImpl;
    }
    // we ensure that order of adding always goes
    // parent -> children, so no need to recurse
    this.children.push(childActionImpl);
  }

  removeChild(actionImpl: ActionImpl) {
    // recursively remove all children
    const index = this.children.indexOf(actionImpl);
    if (index !== -1) {
      this.children.splice(index, 1);
    }
    if (actionImpl.children) {
      actionImpl.children.forEach((child) => {
        this.removeChild(child);
      });
    }
  }

  // easily access parentActionImpl after creation
  get parentActionImpl() {
    return this.ancestors[this.ancestors.length - 1];
  }

  static create(action: Action, options: ActionImplOptions) {
    return new ActionImpl(action, options);
  }
}

export type ActionTree = Record<string, ActionImpl>;

export interface ActionGroup {
  name: string;
  actions: ActionImpl[];
}

export interface KBarOptions {
  animations?: {
    enterMs?: number;
    exitMs?: number;
  };
  callbacks?: {
    onOpen?: () => void;
    onClose?: () => void;
    onQueryChange?: (searchQuery: string) => void;
    onSelectAction?: (action: ActionImpl) => void;
  };
  /**
   * `disableScrollBarManagement` ensures that kbar will not
   * manipulate the document's `margin-right` property when open.
   * By default, kbar will add additional margin to the document
   * body when opened in order to prevent any layout shift with
   * the appearance/disappearance of the scrollbar.
   */
  disableScrollbarManagement?: boolean;
  /**
   * `disableDocumentLock` disables the "document lock" functionality
   * of kbar, where the body element's scrollbar is hidden and pointer
   * events are disabled when kbar is open. This is useful if you're using
   * a custom modal component that has its own implementation of this
   * functionality.
   */
  disableDocumentLock?: boolean;
  enableHistory?: boolean;
}

export interface KBarProviderProps {
  actions?: Action[];
  options?: KBarOptions;
}

export interface KBarState {
  searchQuery: string;
  visualState: VisualState;
  actions: ActionTree;
  currentRootActionId?: ActionId | null;
  activeIndex: number;
}

export interface KBarQuery {
  setCurrentRootAction: (actionId?: ActionId | null) => void;
  setVisualState: (
    cb: ((vs: VisualState) => VisualState) | VisualState
  ) => void;
  setSearch: (search: string) => void;
  registerActions: (actions: Action[]) => () => void;
  toggle: () => void;
  setActiveIndex: (cb: number | ((currIndex: number) => number)) => void;
}

export interface IKBarContext {
  getState: () => KBarState;
  query: KBarQuery;
  subscribe: (
    collector: <C>(state: KBarState) => C,
    cb: <C>(collected: C) => void
  ) => void;
  options: KBarOptions;
}

export enum VisualState {
  animatingIn = "animating-in",
  showing = "showing",
  animatingOut = "animating-out",
  hidden = "hidden",
}
