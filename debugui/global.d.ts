declare const ARGV: string[];

declare const imports: {
  [any: string]: any;
  searchPath: string[];
  dbusProxyUtils: typeof import('./dbusProxyUtils');
  debugWindow: typeof import('./debugWindow');
};

declare function log(msg: any): void;

declare function logError(e: any): void;

declare interface Error {
  fileName: string;
}
