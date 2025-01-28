export class IdentifierGenerator {
    private static current: string = 'a';

    static init(): void {
        this.current = 'a';
    }

    static get(): string {
        const identifier = this.current;
        const currentLength = this.current.length;

        for (let i = currentLength - 1; i >= 0; i--) {
            if (this.current[i] < 'z') {
                this.current = this.updateCurrentIdentifier(i);
                return identifier;
            }

            this.current = this.resetCharacterAtIndex(i);

            if (i === 0) {
                this.current = 'a' + this.current;
            }
        }

        return identifier;
    }


    private static updateCurrentIdentifier(index: number): string {
        const updatedChar = String.fromCharCode(this.current.charCodeAt(index) + 1);
        return this.current.slice(0, index) + updatedChar + this.current.slice(index + 1);
    }

    private static resetCharacterAtIndex(index: number): string {
        return this.current.slice(0, index) + 'a' + this.current.slice(index + 1);
    }
};
