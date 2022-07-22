export function generateUniqueNameFromDisplayName(displayName: string): string {
  // make lowercase, replace space by dash
  const name = displayName.toLowerCase().replace(/ /g, "-");
  return name;
}
