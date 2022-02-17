//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

interface CommandOptions {}

export class Command {
  perform: (...args: any) => any;

  constructor(command: { perform: Command["perform"] }) {
    this.perform = () => {
      const negate = command.perform();
      // no need for history if non negatable
      if (typeof negate !== "function") return;
      // return if no history enabled

      //TODO(sualeh): do we need history.
      //   const history = options.history;
      //   if (!history) return;
      // since we are performing the same action, we'll clean up the
      // previous call to the action and create a new history record
      //   if (this.historyItem) {
      //     history.remove(this.historyItem);
      //   }
      //   this.historyItem = history.add({
      //     perform: command.perform,
      //     negate,
      //   });

      //   this.history = {
      //     undo: () => history.undo(this.historyItem),
      //     redo: () => history.redo(this.historyItem),
      //   };
    };
  }
}
