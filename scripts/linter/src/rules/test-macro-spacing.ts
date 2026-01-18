import { LintRule, LintContext, LintViolation } from '../types';

// Test macros that need spaces inside parentheses: MACRO( content )
const TEST_MACROS = [
    // Test structure
    'TEST_CASE',
    'SUBCASE',

    // Basic assertions
    'CHECK',
    'CHECK_FALSE',
    'REQUIRE',
    'REQUIRE_FALSE',
    'WARN',
    'WARN_FALSE',

    // Binary comparisons
    'CHECK_EQ',
    'CHECK_NE',
    'CHECK_GT',
    'CHECK_LT',
    'CHECK_GE',
    'CHECK_LE',
    'REQUIRE_EQ',
    'REQUIRE_NE',
    'REQUIRE_GT',
    'REQUIRE_LT',
    'REQUIRE_GE',
    'REQUIRE_LE',
    'WARN_EQ',
    'WARN_NE',
    'WARN_GT',
    'WARN_LT',
    'WARN_GE',
    'WARN_LE',

    // Unary
    'CHECK_UNARY',
    'CHECK_UNARY_FALSE',
    'REQUIRE_UNARY',
    'REQUIRE_UNARY_FALSE',
    'WARN_UNARY',
    'WARN_UNARY_FALSE',

    // Exception handling
    'CHECK_THROWS',
    'CHECK_THROWS_AS',
    'CHECK_THROWS_WITH',
    'CHECK_THROWS_WITH_AS',
    'CHECK_NOTHROW',
    'REQUIRE_THROWS',
    'REQUIRE_THROWS_AS',
    'REQUIRE_THROWS_WITH',
    'REQUIRE_THROWS_WITH_AS',
    'REQUIRE_NOTHROW',
    'WARN_THROWS',
    'WARN_THROWS_AS',
    'WARN_THROWS_WITH',
    'WARN_THROWS_WITH_AS',
    'WARN_NOTHROW',

    // Message variants
    'CHECK_MESSAGE',
    'REQUIRE_MESSAGE',
    'WARN_MESSAGE',

    // Logging
    'INFO',
    'CAPTURE',
    'MESSAGE',
    'FAIL',
    'FAIL_CHECK',
];

// Pattern: MACRO(x - no space after opening paren
// Matches MACRO( followed immediately by a non-space, non-) character
function createNoLeadingSpacePattern(macros: string[]): RegExp {
    const macroPattern = macros.join('|');
    // Match MACRO( followed by non-whitespace (but not just closing paren)
    return new RegExp(`\\b(${macroPattern})\\(([^\\s)])`, 'g');
}

// Pattern: MACRO ( - space before opening paren (should be MACRO()
function createSpaceBeforeParenPattern(macros: string[]): RegExp {
    const macroPattern = macros.join('|');
    return new RegExp(`\\b(${macroPattern})\\s+\\(`, 'g');
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
        const spaceBeforeParenPattern = createSpaceBeforeParenPattern(TEST_MACROS);

        for (let i = 0; i < context.lines.length; i++) {
            const line = context.lines[i];
            const lineNumber = i + 1;

            // Skip comment lines
            if (isComment(line)) {
                continue;
            }

            // Check for space before opening paren: MACRO ( instead of MACRO(
            spaceBeforeParenPattern.lastIndex = 0;
            let spaceMatch;

            while ((spaceMatch = spaceBeforeParenPattern.exec(line)) !== null) {
                const macroName = spaceMatch[1];
                const matchStart = spaceMatch.index;

                violations.push({
                    rule: 'test-macro-spacing',
                    message: `Unexpected space before '(' in '${macroName}'`,
                    line: lineNumber,
                    column: matchStart + macroName.length + 1,
                    severity: 'error',
                });
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
