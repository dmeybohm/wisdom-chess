import * as fs from 'fs';
import * as path from 'path';
import { LinterConfig, Severity } from './types';

const DEFAULT_CONFIG: LinterConfig = {
    rules: {
        'function-call-spacing': 'error',
        'test-macro-spacing': 'error',
        'trailing-return-type': 'warning',
        'namespace-braces': 'error',
    },
    ignore: [
        '**/build/**',
        '**/node_modules/**',
        '**/.git/**',
    ],
};

export function loadConfig(configPath?: string): LinterConfig {
    const searchPaths = configPath
        ? [configPath]
        : [
            '.wisdomstylerc.json',
            path.join(process.cwd(), '.wisdomstylerc.json'),
        ];

    for (const searchPath of searchPaths) {
        if (fs.existsSync(searchPath)) {
            try {
                const content = fs.readFileSync(searchPath, 'utf-8');
                const userConfig = JSON.parse(content) as Partial<LinterConfig>;
                return mergeConfig(DEFAULT_CONFIG, userConfig);
            } catch (e) {
                console.error(`Error loading config from ${searchPath}:`, e);
            }
        }
    }

    return DEFAULT_CONFIG;
}

function mergeConfig(base: LinterConfig, override: Partial<LinterConfig>): LinterConfig {
    return {
        rules: { ...base.rules, ...override.rules },
        ignore: override.ignore ?? base.ignore,
    };
}

export function getRuleSeverity(config: LinterConfig, ruleName: string): Severity {
    const ruleConfig = config.rules[ruleName];
    if (!ruleConfig) {
        return 'off';
    }
    if (Array.isArray(ruleConfig)) {
        return ruleConfig[0];
    }
    return ruleConfig;
}

export function getRuleOptions(config: LinterConfig, ruleName: string): Record<string, unknown> {
    const ruleConfig = config.rules[ruleName];
    if (Array.isArray(ruleConfig) && ruleConfig[1]) {
        return ruleConfig[1];
    }
    return {};
}
