export type Severity = 'error' | 'warning' | 'off';

export interface LintViolation {
    rule: string;
    message: string;
    line: number;
    column?: number;
    severity: 'error' | 'warning';
}

export interface LintContext {
    filename: string;
    lines: string[];
    content: string;
}

export interface LintRule {
    name: string;
    description: string;
    check(context: LintContext): LintViolation[];
}

export interface RuleConfig {
    [ruleName: string]: Severity | [Severity, Record<string, unknown>];
}

export interface LinterConfig {
    rules: RuleConfig;
    ignore: string[];
}

export interface LintResult {
    filename: string;
    violations: LintViolation[];
}
