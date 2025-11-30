import * as fs from 'fs';
import { glob } from 'glob';
import { LinterConfig, LintRule, LintResult, LintContext, LintViolation } from './types';
import { getRuleSeverity } from './config';
import { allRules } from './rules';

export class Linter {
    private rules: LintRule[];
    private config: LinterConfig;

    constructor(config: LinterConfig) {
        this.config = config;
        this.rules = this.getEnabledRules();
    }

    private getEnabledRules(): LintRule[] {
        return allRules.filter(rule => {
            const severity = getRuleSeverity(this.config, rule.name);
            return severity !== 'off';
        });
    }

    async lintFiles(patterns: string[]): Promise<LintResult[]> {
        const files = await this.resolveFiles(patterns);
        const results: LintResult[] = [];

        for (const file of files) {
            const result = this.lintFile(file);
            results.push(result);
        }

        return results;
    }

    private async resolveFiles(patterns: string[]): Promise<string[]> {
        const allFiles: Set<string> = new Set();

        for (const pattern of patterns) {
            const files = await glob(pattern, {
                ignore: this.config.ignore,
                nodir: true,
            });
            for (const file of files) {
                if (this.isCppFile(file)) {
                    allFiles.add(file);
                }
            }
        }

        return Array.from(allFiles).sort();
    }

    private isCppFile(filename: string): boolean {
        const ext = filename.toLowerCase();
        return ext.endsWith('.cpp') || ext.endsWith('.hpp') || ext.endsWith('.h');
    }

    lintFile(filename: string): LintResult {
        const content = fs.readFileSync(filename, 'utf-8');
        const lines = content.split('\n');

        const context: LintContext = {
            filename,
            lines,
            content,
        };

        const violations: LintViolation[] = [];

        for (const rule of this.rules) {
            const ruleViolations = rule.check(context);
            const severity = getRuleSeverity(this.config, rule.name);

            for (const violation of ruleViolations) {
                violations.push({
                    ...violation,
                    severity: severity === 'error' ? 'error' : 'warning',
                });
            }
        }

        violations.sort((a, b) => a.line - b.line);

        return {
            filename,
            violations,
        };
    }
}
