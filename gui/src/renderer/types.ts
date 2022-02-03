export type Message = {
  id: string;
  from: string;
  to: string;
  message: string;
  timestamp: Date;
};

export type Friend = {
  name: string;
  status: "initiated" | "added";
};
