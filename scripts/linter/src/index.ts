#!/usr/bin/env node

import { Command } from 'commander';
import { Linter } from './linter';
import { loadConfig } from './config';
import { formatResults, getSummary, OutputFormat } from './reporter';
import { allRules } from './rules';

const program = new Command();

program
    .name('wisdom-style-lint')
    .description('C++ style linter for wisdom-chess project')
    .version('1.0.0')
    .argument('[patterns...]', 'File patterns to lint (e.g., src/**/*.cpp)')
    .option('-c, --config <path>', 'Path to config file')
    .option('-f, --format <format>', 'Output format: stylish, compact, json', 'stylish')
    .option('--rules <rules>', 'Comma-separated list of rules to run')
    .option('--list-rules', 'List all available rules')
    .action(async (patterns: string[], options) => {
        if (options.listRules) {
            console.log('Available rules:\n');
            for (const rule of allRules) {
                console.log(`  ${rule.name}`);
                console.log(`    ${rule.description}\n`);
            }
            process.exit(0);
        }

        if (!patterns || patterns.length === 0) {
            console.error('Error: No file patterns specified');
            console.error('Usage: wisdom-style-lint <patterns...>');
            process.exit(1);
        }

        const config = loadConfig(options.config);

        // Filter rules if --rules option is provided
        if (options.rules) {
            const selectedRules = options.rules.split(',').map((r: string) => r.trim());
            for (const ruleName of Object.keys(config.rules)) {
                if (!selectedRules.includes(ruleName)) {
                    config.rules[ruleName] = 'off';
                }
            }
        }

        const linter = new Linter(config);

        try {
            const results = await linter.lintFiles(patterns);
            const output = formatResults(results, options.format as OutputFormat);

            if (output) {
                console.log(output);
            }

            const summary = getSummary(results);

            if (summary.errors > 0) {
                process.exit(1);
            } else if (summary.warnings > 0) {
                process.exit(0);
            } else {
                // No issues found
                process.exit(0);
            }
        } catch (error) {
            console.error('Error running linter:', error);
            process.exit(2);
        }
    });

program.parse();
