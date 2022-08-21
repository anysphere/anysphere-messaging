//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import { MutableRefObject, useEffect } from "react";
import { $getRoot, $getSelection, $createParagraphNode, $createTextNode,
  $createLineBreakNode, COMMAND_PRIORITY_LOW, EditorState, FOCUS_COMMAND } from "lexical";
import { LexicalComposer } from "@lexical/react/LexicalComposer";
import { RichTextPlugin } from "@lexical/react/LexicalRichTextPlugin";
import { PlainTextPlugin } from "@lexical/react/LexicalPlainTextPlugin";
import { HeadingNode, QuoteNode, $createQuoteNode} from "@lexical/rich-text";
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

import { ResponseData } from "./Write";
import { root } from "postcss";

const prose = "prose-stone";

const theme = {
  paragraph: "mt-0 mb-0",
  placeholder: prose,
  quote: "mt-0 mb-0",
  heading: {
    h1: "mt-0 mb-0",
    h2: "mt-0 mb-0",
    h3: "mt-0 mb-0",
    h4: "mt-0 mb-0",
    h5: "mt-0 mb-0",
  },
  list: {
    nested: {
      listitem: prose,
    },
    ol: "mt-0 mb-0",
    ul: "mt-0 mb-0",
    listitem: "mt-1 mb-1",
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
  responseTemplate,
}: {
  focused: boolean;
  onFocus: () => void;
  editorStateRef: MutableRefObject<EditorState | null>;
  responseTemplate: ResponseData | undefined;
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
    <div className="prose prose-sm prose-stone prose-stonex relative bg-white">
      <LexicalComposer initialConfig={initialConfig}>
        <RichTextPlugin
          contentEditable={
            <ContentEditable className="min-h-[100px] text-sm outline-none" />
          }
          initialEditorState={      // Put some initial text in the message if there is none
            () => {
              // mimics https://github.com/facebook/lexical/blob/b804f2a3eeb407a58d2b78b91d51cab62ed09bd0/packages/lexical-playground/src/App.tsx
              const root = $getRoot();
              // if responseTemplate is not undefined,
              // prefill responseTemplate into the editor
              if (typeof responseTemplate != "undefined") {
                const paragraph = $createParagraphNode();
                // first line of response
                const header = "On " + responseTemplate.timeStamp + ", " + responseTemplate.sender + " wrote:";
                const headerNode = $createTextNode(header);
                // put line breaks before the header
                const lineBreak1 = $createLineBreakNode();
                const lineBreak2 = $createLineBreakNode();
                const lineBreak3 = $createLineBreakNode();
                paragraph.append(lineBreak1);
                paragraph.append(lineBreak2);
                paragraph.append(lineBreak3);
                paragraph.append(headerNode);
                root.append(paragraph);
                const quote = $createQuoteNode(); 
                const text = $createTextNode(responseTemplate.content);
                text.setStyle("color: #BEBEBE");
                quote.append(text);
                root.append(quote);
              }
          }}
          placeholder=""
        />
        <OnChangePlugin
          onChange={(editorState) => (editorStateRef.current = editorState)}
        />
        <HistoryPlugin />
        <FocusPlugin focused={focused} onFocus={() => {
          onFocus();
          // put the cursor at the start of the editor
          $getRoot().selectStart();
        }} />
        <AsphrAutoLinkPlugin />
        <LinkPlugin />
        <ListPlugin />
        <MarkdownShortcutPlugin transformers={TRANSFORMERS} />
      </LexicalComposer>
    </div>
  );
}
