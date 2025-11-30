import { LintRule } from '../types';
import { functionCallSpacingRule } from './function-call-spacing';
import { testMacroSpacingRule } from './test-macro-spacing';
import { trailingReturnTypeRule } from './trailing-return-type';
import { namespaceBracesRule } from './namespace-braces';

export const allRules: LintRule[] = [
    functionCallSpacingRule,
    testMacroSpacingRule,
    trailingReturnTypeRule,
    namespaceBracesRule,
];

export function getRuleByName(name: string): LintRule | undefined {
    return allRules.find(rule => rule.name === name);
}

export {
    functionCallSpacingRule,
    testMacroSpacingRule,
    trailingReturnTypeRule,
    namespaceBracesRule,
};
