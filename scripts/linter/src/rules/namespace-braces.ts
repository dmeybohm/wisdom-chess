import { LintRule, LintContext, LintViolation } from '../types';

// Pattern to detect namespace with brace on same line
// Matches: namespace Name { or namespace { (anonymous)
const NAMESPACE_SAME_LINE_BRACE = /^\s*namespace\s+([a-zA-Z_][a-zA-Z0-9_]*)?(\s*::\s*[a-zA-Z_][a-zA-Z0-9_]*)*\s*\{/;

export const namespaceBracesRule: LintRule = {
    name: 'namespace-braces',
    description: 'Namespace opening brace should be on its own line',

    check(context: LintContext): LintViolation[] {
        const violations: LintViolation[] = [];

        for (let i = 0; i < context.lines.length; i++) {
            const line = context.lines[i];
            const lineNumber = i + 1;

            const match = NAMESPACE_SAME_LINE_BRACE.exec(line);
            if (match) {
                const namespaceName = match[1] || '(anonymous)';

                violations.push({
                    rule: 'namespace-braces',
                    message: `Namespace '${namespaceName}' opening brace should be on the next line`,
                    line: lineNumber,
                    column: line.indexOf('{') + 1,
                    severity: 'error',
                });
            }
        }

        return violations;
    },
};
