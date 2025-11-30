import { LintRule, LintContext, LintViolation } from '../types';

// Common C++ types that might appear as leading return types
// NOTE: 'void', 'bool', and 'int' are excluded - they don't need trailing return types
const COMMON_TYPES = [
    'char',
    'short',
    'long',
    'float',
    'double',
    'size_t',
    'int8_t',
    'int16_t',
    'int32_t',
    'int64_t',
    'uint8_t',
    'uint16_t',
    'uint32_t',
    'uint64_t',
    'string',
    'wstring',
];

// Pattern to detect old-style function declarations with leading return type
// Matches: Type functionName(...) where Type is a known type
// Excludes: auto (trailing return), if/for/while (control flow), class/struct definitions
const OLD_STYLE_PATTERN = new RegExp(
    `^\\s*` +
    `(?:(?:static|inline|virtual|explicit|constexpr|consteval|friend|extern)\\s+)*` + // Optional specifiers
    `(${COMMON_TYPES.join('|')})` + // Return type (captured)
    `(?:\\s*[*&]+)?` + // Optional pointer/reference
    `\\s+` +
    `([a-zA-Z_][a-zA-Z0-9_]*)` + // Function name (captured)
    `\\s*\\(` // Opening parenthesis
);

// Pattern for more complex return types (templates, custom types)
// Matches: SomeType<T> funcName( or CustomType funcName(
const COMPLEX_TYPE_PATTERN = /^\s*(?:(?:static|inline|virtual|explicit|constexpr|consteval|friend|extern)\s+)*([A-Z][a-zA-Z0-9_]*(?:<[^>]+>)?)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(/;

// Patterns to skip - these are valid non-function contexts
const SKIP_PATTERNS = [
    /^\s*(?:class|struct|enum|union|namespace)\s/, // Type definitions
    /^\s*(?:using|typedef)\s/, // Type aliases
    /^\s*template\s*</, // Template declarations (next line has the function)
    /^\s*#/, // Preprocessor
    /^\s*\/\//, // Comments
    /^\s*\/\*/, // Multi-line comments
    /^\s*\*/, // Inside multi-line comment
    /^\s*return\s/, // Return statements
    /^\s*(?:if|for|while|switch|catch)\s*\(/, // Control flow
    /=\s*\[/, // Lambda capture
    /^\s*auto\s/, // Already using auto (correct style)
    /^\s*\[\[/, // Attributes
];

// Check if this looks like a function definition vs declaration
function isFunctionDefinitionOrDeclaration(line: string, nextLine?: string): boolean {
    // Has opening brace on same line or next line starts with brace
    if (line.includes('{') || nextLine?.trim().startsWith('{')) {
        return true;
    }
    // Ends with semicolon (declaration)
    if (line.trim().endsWith(';')) {
        return true;
    }
    // Has trailing return type arrow somewhere
    if (line.includes('->')) {
        return false; // Already has trailing return type
    }
    return true;
}

export const trailingReturnTypeRule: LintRule = {
    name: 'trailing-return-type',
    description: 'Functions should use trailing return type syntax: auto func() -> Type',

    check(context: LintContext): LintViolation[] {
        const violations: LintViolation[] = [];

        for (let i = 0; i < context.lines.length; i++) {
            const line = context.lines[i];
            const lineNumber = i + 1;
            const nextLine = i + 1 < context.lines.length ? context.lines[i + 1] : undefined;

            // Skip lines that match skip patterns
            if (SKIP_PATTERNS.some(pattern => pattern.test(line))) {
                continue;
            }

            // Check for common types used as leading return types
            let match = OLD_STYLE_PATTERN.exec(line);
            if (match) {
                const returnType = match[1];
                const funcName = match[2];

                // Skip main() - it's conventional to use int main()
                if (funcName === 'main') {
                    continue;
                }

                // Make sure this is actually a function definition/declaration
                if (isFunctionDefinitionOrDeclaration(line, nextLine)) {
                    violations.push({
                        rule: 'trailing-return-type',
                        message: `Function '${funcName}' should use trailing return type: auto ${funcName}(...) -> ${returnType}`,
                        line: lineNumber,
                        column: match.index + 1,
                        severity: 'warning',
                    });
                }
                continue;
            }

            // Check for complex/custom types
            match = COMPLEX_TYPE_PATTERN.exec(line);
            if (match) {
                const returnType = match[1];
                const funcName = match[2];

                // Skip if it looks like a constructor (type name matches function name)
                if (returnType === funcName) {
                    continue;
                }

                // Skip common macros and special cases
                if (['TEST_CASE', 'SUBCASE', 'SECTION'].includes(funcName)) {
                    continue;
                }

                if (isFunctionDefinitionOrDeclaration(line, nextLine)) {
                    violations.push({
                        rule: 'trailing-return-type',
                        message: `Function '${funcName}' should use trailing return type: auto ${funcName}(...) -> ${returnType}`,
                        line: lineNumber,
                        column: match.index + 1,
                        severity: 'warning',
                    });
                }
            }
        }

        return violations;
    },
};
