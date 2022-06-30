import Modal from "../Modal";
import { ModalType } from "../Modal";
import { StatusProps } from "../Status";
import { classNames } from "../../utils";

export default function AddFriendRemote({
  onClose,
  setStatus,
}: {
  onClose: () => void;
  setStatus: (status: StatusProps) => void;
}): JSX.Element {
  return (
    <Modal onClose={onClose} type={ModalType.Large}>
      <div className="grid h-full w-full items-center">
        <div className="unselectable grid justify-items-center gap-8">
          <h1 className="text-center font-['Lora'] text-3xl text-asbrown-dark">
            Welcome to{" "}
            <span className="animate-slideunderline underline decoration-2 underline-offset-4">
              complete privacy
            </span>
            .
          </h1>
          <button className="mx-auto mt-4 animate-revealsimple text-asbrown-light opacity-80">
            Get started ›
          </button>
        </div>
      </div>
    </Modal>
  );
}
