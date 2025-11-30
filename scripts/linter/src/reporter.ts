import chalk from 'chalk';
import { LintResult, LintViolation } from './types';

export type OutputFormat = 'stylish' | 'compact' | 'json';

export function formatResults(results: LintResult[], format: OutputFormat): string {
    switch (format) {
        case 'compact':
            return formatCompact(results);
        case 'json':
            return formatJson(results);
        case 'stylish':
        default:
            return formatStylish(results);
    }
}

function formatStylish(results: LintResult[]): string {
    const output: string[] = [];
    let totalErrors = 0;
    let totalWarnings = 0;

    for (const result of results) {
        if (result.violations.length === 0) {
            continue;
        }

        output.push('');
        output.push(chalk.underline(result.filename));

        for (const violation of result.violations) {
            const severity = violation.severity === 'error'
                ? chalk.red('error')
                : chalk.yellow('warning');

            const location = violation.column
                ? `${violation.line}:${violation.column}`
                : `${violation.line}`;

            output.push(
                `  ${chalk.dim(location.padEnd(8))} ${severity.padEnd(17)} ${violation.message}  ${chalk.dim(violation.rule)}`
            );

            if (violation.severity === 'error') {
                totalErrors++;
            } else {
                totalWarnings++;
            }
        }
    }

    if (totalErrors > 0 || totalWarnings > 0) {
        output.push('');
        const summary: string[] = [];
        if (totalErrors > 0) {
            summary.push(chalk.red(`${totalErrors} error${totalErrors === 1 ? '' : 's'}`));
        }
        if (totalWarnings > 0) {
            summary.push(chalk.yellow(`${totalWarnings} warning${totalWarnings === 1 ? '' : 's'}`));
        }
        output.push(`  ${summary.join(', ')}`);
        output.push('');
    }

    return output.join('\n');
}

function formatCompact(results: LintResult[]): string {
    const lines: string[] = [];

    for (const result of results) {
        for (const violation of result.violations) {
            const location = violation.column
                ? `${result.filename}:${violation.line}:${violation.column}`
                : `${result.filename}:${violation.line}`;

            lines.push(`${location}: ${violation.severity}: ${violation.message} [${violation.rule}]`);
        }
    }

    return lines.join('\n');
}

function formatJson(results: LintResult[]): string {
    return JSON.stringify(results, null, 2);
}

export function getSummary(results: LintResult[]): { errors: number; warnings: number } {
    let errors = 0;
    let warnings = 0;

    for (const result of results) {
        for (const violation of result.violations) {
            if (violation.severity === 'error') {
                errors++;
            } else {
                warnings++;
            }
        }
    }

    return { errors, warnings };
}
