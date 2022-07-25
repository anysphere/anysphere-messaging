import { AutoLinkPlugin, LinkMatcher } from "@lexical/react/LexicalAutoLinkPlugin";

const URL_MATCHER = /^(?:(?:(?:https?|ftp):)?\/\/)?(?:\S+(?::\S*)?@)?(?:(?!(?:10|127)(?:\.\d{1,3}){3})(?!(?:169\.254|192\.168)(?:\.\d{1,3}){2})(?!172\.(?:1[6-9]|2\d|3[0-1])(?:\.\d{1,3}){2})(?:[1-9]\d?|1\d\d|2[01]\d|22[0-3])(?:\.(?:1?\d{1,2}|2[0-4]\d|25[0-5])){2}(?:\.(?:[1-9]\d?|1\d\d|2[0-4]\d|25[0-4]))|(?:(?:[a-z0-9\u00a1-\uffff][a-z0-9\u00a1-\uffff_-]{0,62})?[a-z0-9\u00a1-\uffff]\.)+(?:[a-z\u00a1-\uffff]{2,}\.?))(?::\d{2,5})?(?:[/?#]\S*)?([.,;:!?"']?)$/i;

const EMAIL_MATCHER =
  /(([^<>()[\]\\.,;:\s@"]+(\.[^<>()[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))/;


const urlMatcher: LinkMatcher = (text: string) => {
  const match: RegExpExecArray | null = URL_MATCHER.exec(text);

  if (match === null || match[0] === undefined) {
    return null;
  }
  // strip ([.,;:!?\"']?) from the end of the match if it exists
  let url = match[0];

  if (url.endsWith(".") || url.endsWith(",") || url.endsWith(";") || url.endsWith("!") || url.endsWith("?") || url.endsWith(":") || url.endsWith("\"") || url.endsWith("'")) {
    // this matches the regex: [.,;:!?\"']$
    url = url.slice(0, -1);
  }

  return (
    {
      index: match.index,
      length: match[0].length,
      text: match[0],
      url: url,
    }
  );
};

const emailMatcher: LinkMatcher = (text: string) => {
  const match = EMAIL_MATCHER.exec(text);

  if (match === null || match[0] === undefined) {
    return null;
  }

  // strip ([.,;:!?\"']?) from the end of the match if it exists
  let url = match[0];
  
  if (url.endsWith(".") || url.endsWith(",") || url.endsWith(";") || url.endsWith("!") || url.endsWith("?") || url.endsWith(":") || url.endsWith("\"") || url.endsWith("'")) {
    // this matches the regex: [.,;:!?\"']$
    url = url.slice(0, -1);
  }

  return (
    {
      index: match.index,
      length: match[0].length,
      text: match[0],
      url: `mailto:${url}`,
    }
  );
};

const MATCHERS: LinkMatcher[] = [
  urlMatcher,
  emailMatcher
];

export default function AsphrAutoLinkPlugin(): JSX.Element {
  return (
    <AutoLinkPlugin matchers={MATCHERS} />
  );
}
