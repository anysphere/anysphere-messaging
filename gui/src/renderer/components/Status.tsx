// TODO: there should be some sort of status manager
// a global setStatus should exist, probably through some context
// should be an enum of acceptable statuses, global list

interface StatusProps {
  message: string;
  action: () => void;
  actionName: string | null;
}

export function Status(props: StatusProps) {}
