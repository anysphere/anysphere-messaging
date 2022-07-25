//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { MutableRefObject, useEffect } from "react";
import { COMMAND_PRIORITY_LOW, EditorState, FOCUS_COMMAND } from "lexical";
import { LexicalComposer } from "@lexical/react/LexicalComposer";
import { RichTextPlugin } from "@lexical/react/LexicalRichTextPlugin";
import { HeadingNode, QuoteNode } from "@lexical/rich-text";
import { ContentEditable } from "@lexical/react/LexicalContentEditable";
import { OnChangePlugin } from "@lexical/react/LexicalOnChangePlugin";
import { LinkPlugin } from "@lexical/react/LexicalLinkPlugin";
import { AutoLinkNode, LinkNode } from "@lexical/link";
import { TableCellNode, TableNode, TableRowNode } from "@lexical/table";
import { ListPlugin } from "@lexical/react/LexicalListPlugin";
import { ListItemNode, ListNode } from "@lexical/list";
import { HistoryPlugin } from "@lexical/react/LexicalHistoryPlugin";
import { useLexicalComposerContext } from "@lexical/react/LexicalComposerContext";
import { LexicalEditor } from "lexical/LexicalEditor";
import AsphrAutoLinkPlugin from "./Plugins/AsphrAutoLinkPlugin";

import { MarkdownShortcutPlugin } from "@lexical/react/LexicalMarkdownShortcutPlugin";
import { TRANSFORMERS } from "@lexical/markdown";


const prose = "prose-stone";

const theme = {
  paragraph: prose,
  placeholder: prose,
  quote: prose,
  heading: {
    h1: prose,
    h2: prose,
    h3: prose,
    h4: prose,
    h5: prose,
  },
  list: {
    nested: {
      listitem: prose,
    },
    ol: prose,
    ul: prose,
    listitem: prose,
  },
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
}): JSX.Element | null {
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
    nodes: [
      AutoLinkNode,
      LinkNode,
      TableCellNode,
      TableRowNode,
      TableNode,
      ListNode,
      ListItemNode,
      HeadingNode,
      QuoteNode,
    ],
  };

  return (
    <div className="relative bg-white prose prose-sm">
      <LexicalComposer initialConfig={initialConfig}>
        <RichTextPlugin
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
        <AsphrAutoLinkPlugin />
        <LinkPlugin />
        <ListPlugin />
        <MarkdownShortcutPlugin transformers={TRANSFORMERS} />
      </LexicalComposer>
    </div>
  );
}
