import { LintRule, LintContext, LintViolation } from '../types';

// Macros/keywords that are exceptions - they don't follow the spacing rule
const EXCEPTION_KEYWORDS = new Set([
    // C++ keywords that look like function calls
    'if',
    'for',
    'while',
    'switch',
    'catch',
    'sizeof',
    'alignof',
    'decltype',
    'typeid',
    'noexcept',
    'void',       // Used in function pointer types: void(int)
    'return',
    'throw',
    'co_return',
    'co_yield',
    'co_await',
    // Preprocessor
    'defined',
    // Common macros
    'NOLINT',
    'Expects',
    'Ensures',
    'assert',
    'static_assert',
    // Test macros - handled by test-macro-spacing rule
    'TEST_CASE',
    'SUBCASE',
    'CHECK',
    'CHECK_FALSE',
    'REQUIRE',
    'REQUIRE_FALSE',
    'REQUIRE_THROWS',
    'REQUIRE_THROWS_AS',
    'REQUIRE_THROWS_WITH',
    'REQUIRE_THROWS_WITH_AS',
    'REQUIRE_NOTHROW',
    'CHECK_THROWS',
    'CHECK_THROWS_AS',
    'CHECK_THROWS_WITH',
    'CHECK_THROWS_WITH_AS',
    'CHECK_NOTHROW',
    'WARN',
    'WARN_FALSE',
    'INFO',
    'CAPTURE',
    'GENERATE',
    'SECTION',
    // Qt macros
    'Q_OBJECT',
    'Q_PROPERTY',
    'Q_SIGNAL',
    'Q_SLOT',
    'Q_EMIT',
    'Q_INVOKABLE',
    'Q_DECLARE_METATYPE',
    'Q_ENUM',
    'Q_FLAG',
    'Q_NAMESPACE',
    'emit',
    'signals',
    'slots',
]);

// Check if a line is a preprocessor directive or comment
function isPreprocessorOrComment(line: string): boolean {
    const trimmed = line.trim();
    return trimmed.startsWith('#') || trimmed.startsWith('//') || trimmed.startsWith('/*') || trimmed.startsWith('*');
}

// Check if we're inside a string literal at a given position
function isInsideString(line: string, position: number): boolean {
    let inString = false;
    let stringChar = '';

    for (let i = 0; i < position; i++) {
        const char = line[i];
        const prevChar = i > 0 ? line[i - 1] : '';

        if (!inString && (char === '"' || char === "'")) {
            inString = true;
            stringChar = char;
        } else if (inString && char === stringChar && prevChar !== '\\') {
            inString = false;
        }
    }

    return inString;
}

// Find the matching closing parenthesis, returns -1 if not found on this line
function findClosingParen(line: string, openParenIndex: number): number {
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
                return i;
            }
        }
        i++;
    }

    return -1; // Not found on this line
}

// Check if this is a multi-line call where ( is at end of line
// e.g., func(\n    arg1,\n    arg2\n)
function isMultiLineCall(line: string, openParenIndex: number, closeParenIndex: number): boolean {
    if (closeParenIndex !== -1) {
        return false; // Closing paren is on same line, not multi-line
    }
    // Check if ( is followed only by whitespace/nothing
    const afterOpen = line.substring(openParenIndex + 1).trim();
    return afterOpen.length === 0;
}

// Check if parentheses contain arguments (non-empty, non-whitespace)
// For multi-line cases, we need to look at subsequent lines
function hasArguments(
    lines: string[],
    lineIndex: number,
    openParenIndex: number,
    closeParenIndex: number
): boolean {
    const line = lines[lineIndex];

    if (closeParenIndex !== -1) {
        // Single line case - easy to check
        const content = line.substring(openParenIndex + 1, closeParenIndex).trim();
        return content.length > 0;
    }

    // Multi-line case - closing paren not on this line
    // Check if there's content after ( on this line
    const afterOpen = line.substring(openParenIndex + 1).trim();
    if (afterOpen.length > 0) {
        return true; // There's content after ( on this line
    }

    // Check next line for content (handles multi-line function signatures)
    if (lineIndex + 1 < lines.length) {
        const nextLine = lines[lineIndex + 1].trim();
        // If next line has content and doesn't start with ), there are arguments
        if (nextLine.length > 0 && !nextLine.startsWith(')')) {
            return true;
        }
    }

    return false;
}

// Pattern to match identifier followed immediately by ( without space
const FUNC_NO_SPACE = /\b([a-zA-Z_][a-zA-Z0-9_]*)\(/g;

// Pattern to match identifier followed by space then (
const FUNC_WITH_SPACE = /\b([a-zA-Z_][a-zA-Z0-9_]*)\s+\(/g;

export const functionCallSpacingRule: LintRule = {
    name: 'function-call-spacing',
    description: 'Functions with arguments need space before (, zero-arg functions do not',

    check(context: LintContext): LintViolation[] {
        const violations: LintViolation[] = [];

        for (let i = 0; i < context.lines.length; i++) {
            const line = context.lines[i];
            const lineNumber = i + 1;

            if (isPreprocessorOrComment(line)) {
                continue;
            }

            // Check for functions WITHOUT space before ( that HAVE arguments (error)
            let match;
            FUNC_NO_SPACE.lastIndex = 0;

            while ((match = FUNC_NO_SPACE.exec(line)) !== null) {
                const funcName = match[1];
                const matchStart = match.index;
                const openParenIndex = matchStart + funcName.length;

                if (isInsideString(line, matchStart)) {
                    continue;
                }

                if (EXCEPTION_KEYWORDS.has(funcName)) {
                    continue;
                }

                const closeParenIndex = findClosingParen(line, openParenIndex);

                // Skip multi-line calls where ( is at end of line - these don't need space
                if (isMultiLineCall(line, openParenIndex, closeParenIndex)) {
                    continue;
                }

                if (hasArguments(context.lines, i, openParenIndex, closeParenIndex)) {
                    violations.push({
                        rule: 'function-call-spacing',
                        message: `Missing space before '(' in '${funcName}' (has arguments)`,
                        line: lineNumber,
                        column: openParenIndex + 1,
                        severity: 'error',
                    });
                }
            }

            // Check for functions WITH space before ( that have NO arguments (error)
            FUNC_WITH_SPACE.lastIndex = 0;

            while ((match = FUNC_WITH_SPACE.exec(line)) !== null) {
                const funcName = match[1];
                const matchStart = match.index;
                const fullMatch = match[0];
                const openParenIndex = matchStart + fullMatch.length - 1;

                if (isInsideString(line, matchStart)) {
                    continue;
                }

                if (EXCEPTION_KEYWORDS.has(funcName)) {
                    continue;
                }

                const closeParenIndex = findClosingParen(line, openParenIndex);

                if (!hasArguments(context.lines, i, openParenIndex, closeParenIndex)) {
                    violations.push({
                        rule: 'function-call-spacing',
                        message: `Unnecessary space before '(' in '${funcName}' (no arguments)`,
                        line: lineNumber,
                        column: openParenIndex + 1,
                        severity: 'error',
                    });
                }
            }
        }

        return violations;
    },
};
