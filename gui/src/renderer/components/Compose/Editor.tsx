//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { MutableRefObject, useEffect } from "react";
import { COMMAND_PRIORITY_LOW, EditorState, FOCUS_COMMAND } from "lexical";
import { LexicalComposer } from "@lexical/react/LexicalComposer";
import { PlainTextPlugin } from "@lexical/react/LexicalPlainTextPlugin";
import { ContentEditable } from "@lexical/react/LexicalContentEditable";
import { OnChangePlugin } from "@lexical/react/LexicalOnChangePlugin";
import { HistoryPlugin } from "@lexical/react/LexicalHistoryPlugin";
import { useLexicalComposerContext } from "@lexical/react/LexicalComposerContext";
import { LexicalEditor } from "lexical/LexicalEditor";

const theme = {
  paragraph: "mb-1", // tailwind classes work!
};

// Lexical React plugins are React components, which makes them
// highly composable. Furthermore, you can lazy load plugins if
// desired, so you don't pay the cost for plugins until you
// actually use them.
function FocusPlugin({
  focused,
  onFocus,
}: {
  focused: boolean;
  onFocus: () => void;
}): JSX.Element {
  const [editor] = useLexicalComposerContext();

  useEffect(() => {
    // Focus the editor when the effect fires!
    if (focused) {
      editor.focus();
    }
  }, [editor, focused]);

  useEffect(() => {
    return editor.registerCommand(
      FOCUS_COMMAND,
      () => {
        onFocus();
        return false;
      },
      COMMAND_PRIORITY_LOW
    );
  }, [editor, onFocus]);

  return null;
}

export function Editor({
  focused,
  onFocus,
  editorStateRef,
}: {
  focused: boolean;
  onFocus: () => void;
  editorStateRef: MutableRefObject<EditorState | null>;
}): JSX.Element {
  const initialConfig = {
    namespace: "MyEditor",
    theme,
    onError: (error: Error, _: LexicalEditor) => {
      console.error(error);
    },
  };

  return (
    <div className="relative bg-white">
      <LexicalComposer initialConfig={initialConfig}>
        <PlainTextPlugin
          contentEditable={
            <ContentEditable className="min-h-[100px] text-sm outline-none" />
          }
          placeholder=""
        />
        <OnChangePlugin
          onChange={(editorState) => (editorStateRef.current = editorState)}
        />
        <HistoryPlugin />
        <FocusPlugin focused={focused} onFocus={onFocus} />
      </LexicalComposer>
    </div>
  );
}
