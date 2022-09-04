//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

export function generateUniqueNameFromDisplayName(displayName: string): string {
  // make lowercase, replace space by dash
  const name = displayName.toLowerCase().replace(/ /g, "-");
  return name;
}

export async function sha256(s: string): Promise<ArrayBuffer> {
  const encoder = new TextEncoder();
  const data = encoder.encode(s);
  const hash = await crypto.subtle.digest("SHA-256", data);
  return hash;
}
export async function sha256string(s: string): Promise<string> {
  const hashBuffer = await sha256(s);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  const hashHex = hashArray
    .map((b) => b.toString(16).padStart(2, "0"))
    .join("");
  return hashHex;
}

export function randint(rng: any, min: number, max: number): number {
  return Math.floor(rng() * (max - min + 1)) + min;
}
