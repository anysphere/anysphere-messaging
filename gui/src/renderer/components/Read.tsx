//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import { IncomingMessage, OutgoingMessage } from "../../types";
import { micromark } from "micromark";
import DOMPurify from "dompurify";

function Read({
  message,
  onClose,
}: {
  message: IncomingMessage | OutgoingMessage;
  onClose: () => void;
}): JSX.Element {
  React.useEffect(() => {
    const handler = (event: KeyboardEvent): void => {
      if (event.key === "Escape") {
        console.log(event.key);
        event.preventDefault();
        onClose();
      }
    };
    window.addEventListener("keydown", handler);
    return () => window.removeEventListener("keydown", handler);
  }, [onClose]);

  const parsedMessage = micromark(message.message);

  // We shouldn't need this because we're using micromark which makes sure that it is clean.
  // But an extra layer of protection is nice.
  const purifiedMessage = DOMPurify.sanitize(parsedMessage, {
    // eslint-disable-next-line @typescript-eslint/naming-convention
    USE_PROFILES: {
      html: true,
    },
  });

  const htmlMessage = (
    <div
      dangerouslySetInnerHTML={{
        // eslint-disable-next-line @typescript-eslint/naming-convention
        __html: purifiedMessage,
      }}
    />
  );

  return (
    <div className="mt-8 flex w-full place-content-center text-sm">
      <div className="flex w-full max-w-3xl flex-col place-self-center bg-white p-2 px-4">
        <div className="flex flex-row content-center pt-2">
          <div className="grid place-content-center">
            <div className="unselectable align-bottom">From:</div>
          </div>
          <div className="pl-2">
            {"fromDisplayName" in message ? message.fromDisplayName : "me"}
          </div>
          <div className="flex-1"></div>
        </div>
        <div className="flex flex-row content-center pb-2 pt-1 text-xs text-asbrown-300">
          <div className="grid place-content-center">
            <div className="unselectable align-bottom">To:</div>
          </div>
          <div className="pl-2">
            {"otherRecipientsList" in message
              ? "me, " +
                message.otherRecipientsList
                  .map((recipient) => {
                    if (recipient.displayName !== "") {
                      return recipient.displayName;
                    } else {
                      return recipient.publicId;
                    }
                  })
                  .join(", ")
              : message.toFriendsList
                  .map((recipient) => recipient.displayName)
                  .join(", ")}
          </div>
          <div className="flex-1"></div>
        </div>
        <hr className="border-asbrown-100" />
        <div className="h-full w-full grow resize-none whitespace-pre-wrap pt-4 pb-4 focus:outline-none">
          <div className="prose prose-sm prose-stone max-w-prose">
            {htmlMessage}
          </div>
        </div>
      </div>
    </div>
  );
}

export default Read;
