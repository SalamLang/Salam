export class TokenLocation {
    index: number;
    length: number;
    startLine: number;
    startColumn: number;
    endLine: number;
    endColumn: number;

    constructor(
        index: number,
        length: number,
        startLine: number,
        startColumn: number,
        endLine: number,
        endColumn: number
    ) {
        this.index = index;
        this.length = length;
        this.startLine = startLine;
        this.startColumn = startColumn;
        this.endLine = endLine;
        this.endColumn = endColumn;
    }

    print(): void {
        console.log(`Location - Index: ${this.index}, Length: ${this.length}`);
        console.log(`Start: Line ${this.startLine}, Column ${this.startColumn}`);
        console.log(`End: Line ${this.endLine}, Column ${this.endColumn}`);
    }

    stringify(): string {
        return JSON.stringify({
            index: this.index,
            length: this.length,
            start: { line: this.startLine, column: this.startColumn },
            end: { line: this.endLine, column: this.endColumn },
        });
    }
}
