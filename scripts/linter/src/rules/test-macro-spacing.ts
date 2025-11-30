import { LintRule, LintContext, LintViolation } from '../types';

// Test macros that need spaces inside parentheses: MACRO( content )
const TEST_MACROS = [
    'TEST_CASE',
    'SUBCASE',
    'CHECK',
    'CHECK_FALSE',
    'REQUIRE',
    'REQUIRE_FALSE',
    'WARN',
    'WARN_FALSE',
    'INFO',
    'CAPTURE',
];

// Pattern: MACRO(x - no space after opening paren
// Matches MACRO( followed immediately by a non-space, non-) character
function createNoLeadingSpacePattern(macros: string[]): RegExp {
    const macroPattern = macros.join('|');
    // Match MACRO( followed by non-whitespace (but not just closing paren)
    return new RegExp(`\\b(${macroPattern})\\(([^\\s)])`, 'g');
}

// Pattern: MACRO(content ) without proper closing - content) instead of content )
// This is trickier - we need to find ) that isn't preceded by space
function checkClosingSpace(line: string, macroName: string, openParenIndex: number): { hasError: boolean; column: number } | null {
    let depth = 1;
    let i = openParenIndex + 1;

    while (i < line.length && depth > 0) {
        const char = line[i];

        // Handle string literals
        if (char === '"' || char === "'") {
            const quote = char;
            i++;
            while (i < line.length && line[i] !== quote) {
                if (line[i] === '\\') i++; // Skip escaped char
                i++;
            }
        } else if (char === '(') {
            depth++;
        } else if (char === ')') {
            depth--;
            if (depth === 0) {
                // Check if there's no space before this )
                const prevChar = i > 0 ? line[i - 1] : ' ';
                if (prevChar !== ' ' && prevChar !== '\t') {
                    return { hasError: true, column: i + 1 };
                }
            }
        }
        i++;
    }

    return null;
}

// Check if a line is a comment
function isComment(line: string): boolean {
    const trimmed = line.trim();
    return trimmed.startsWith('//') || trimmed.startsWith('/*') || trimmed.startsWith('*');
}

export const testMacroSpacingRule: LintRule = {
    name: 'test-macro-spacing',
    description: 'Test macros should have spaces inside parentheses: MACRO( content )',

    check(context: LintContext): LintViolation[] {
        const violations: LintViolation[] = [];
        const noLeadingSpacePattern = createNoLeadingSpacePattern(TEST_MACROS);

        for (let i = 0; i < context.lines.length; i++) {
            const line = context.lines[i];
            const lineNumber = i + 1;

            // Skip comment lines
            if (isComment(line)) {
                continue;
            }

            // Check for missing leading space: MACRO(x instead of MACRO( x
            noLeadingSpacePattern.lastIndex = 0;
            let match;

            while ((match = noLeadingSpacePattern.exec(line)) !== null) {
                const macroName = match[1];
                const matchStart = match.index;

                violations.push({
                    rule: 'test-macro-spacing',
                    message: `Missing space after '(' in '${macroName}'`,
                    line: lineNumber,
                    column: matchStart + macroName.length + 2,
                    severity: 'error',
                });
            }

            // Check for missing trailing space before )
            for (const macroName of TEST_MACROS) {
                const macroPattern = new RegExp(`\\b${macroName}\\(`, 'g');
                let macroMatch;

                while ((macroMatch = macroPattern.exec(line)) !== null) {
                    const openParenIndex = macroMatch.index + macroName.length;
                    const closingCheck = checkClosingSpace(line, macroName, openParenIndex);

                    if (closingCheck?.hasError) {
                        violations.push({
                            rule: 'test-macro-spacing',
                            message: `Missing space before ')' in '${macroName}'`,
                            line: lineNumber,
                            column: closingCheck.column,
                            severity: 'error',
                        });
                    }
                }
            }
        }

        return violations;
    },
};
