//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import * as React from "react";
import Modal from "./Modal";
import anyspherelogo from "../../../assets/anysphere-logo.svg";

enum RegisterScreen {
  Welcome,
  Form,
}

export function RegisterModal({
  onClose,
  onRegister,
}: {
  onClose: () => void;
  onRegister: (username: string, key: string) => void;
}) {
  const [screen, setScreen] = React.useState<RegisterScreen>(
    RegisterScreen.Welcome
  );

  let component;
  switch (screen) {
    case RegisterScreen.Welcome:
      component = (
        <Modal onClose={onClose}>
          <div className="grid h-full w-full items-center">
            <div className="unselectable grid justify-items-center gap-8">
              <img className="h-12" src={anyspherelogo} alt="Anysphere logo." />
              <h1 className="text-center font-['Lora'] text-3xl text-asbrown-dark">
                Welcome to{" "}
                <span className="animate-slideunderline underline decoration-2 underline-offset-4">
                  complete privacy
                </span>
                .
              </h1>
              <button
                className="mx-auto mt-4 animate-revealsimple text-asbrown-light opacity-80"
                onClick={() => setScreen(RegisterScreen.Form)}
              >
                Get started ›
              </button>
            </div>
          </div>
        </Modal>
      );
      break;
    case RegisterScreen.Form:
      component = (
        <RegisterModalForm onClose={onClose} onRegister={onRegister} />
      );
      break;
  }

  return component;
}

function RegisterModalForm({
  onClose,
  onRegister,
}: {
  onClose: () => void;
  onRegister: (username: string, key: string) => void;
}) {
  const [username, setUsername] = React.useState<string>("No Name");
  const [accesskey, setAccesskey] = React.useState<string>("");
  return (
    <Modal onClose={onClose}>
      <div className="grid h-full items-center p-4">
        <div className="grid gap-2 px-1">
          <div className="text-center font-bold">Register</div>
          <div className="unselectable py-1 text-center text-sm">
            Anysphere is in alpha! For support, please see{" "}
            <a className="hover:underline" href="https://discord.gg/FxCBpJRbT9">
              our Discord
            </a>
            .
          </div>
          <div className="unselectable pt-1 text-xs text-asbrown-dark">
            <b>
              Please remember that this is an alpha product. While we think that
              our current implementation meets our privacy guarantees, we may
              have bugs, and those bugs may have privacy consequences.
            </b>{" "}
            While in alpha, don't send anything on Anysphere that you wouldn't
            send on Signal.
          </div>
          {/* <div className="flex flex-row my-2 gap-1 justify-center">
          <label className="unselectable">Name: </label>
          <input
            autoFocus
            type="text"
            value={username}
            onChange={(e) => {
              setUsername(e.target.value);
            }}
            className="bg-red-100/[0] px-1 mx-2 font-mono focus:outline-none
              focus:ring-0 border-b-2 flex-grow border-asbrown-100"
          />
        </div> */}
          <div className="my-2 flex flex-row items-baseline justify-center gap-1">
            <label className="unselectable">Access key: </label>
            <input
              type="text"
              value={accesskey}
              onChange={(e) => {
                setAccesskey(e.target.value);
              }}
              className="mx-2 
              flex-grow border-0 border-b-2 border-asbrown-100 px-1 py-1
              font-mono placeholder:text-asbrown-200 focus:border-asbrown-300 focus:outline-none focus:ring-0"
              autoFocus
              placeholder="alpha testing access key"
            />
          </div>
          <div className="grid">
            <button
              className="unselectable my-2 place-self-center rounded-lg bg-asbrown-100 px-3 py-1 text-asbrown-light"
              onClick={() => onRegister(username, accesskey)}
            >
              Register
            </button>
          </div>
          <div className="unselectable pt-1 text-center text-xs text-asbrown-dark">
            By clicking Register, you agree to our{" "}
            <a
              className="hover:underline"
              href="https://anysphere.co/terms-of-service"
            >
              Terms of Service
            </a>{" "}
            and{" "}
            <a
              className="hover:underline"
              href="https://anysphere.co/privacy-policy"
            >
              Privacy Policy
            </a>
            .
          </div>
          <div className="h-2"></div>
        </div>
      </div>
    </Modal>
  );
}
