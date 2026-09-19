// Generates, from the WebIDL interface:
//   src/lib/wisdom-chess-module.d.ts   the module's types
//   src/test/wasm-enum-values.ts       the enum values, for test doubles
//
//   node scripts/generate-wasm-types.mjs           write the file
//   node scripts/generate-wasm-types.mjs --check   fail if the file is stale

import { execFileSync } from 'node:child_process'
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const GENERATOR = 'webidl-dts-gen@1.12.0'
const MODULE_NAME = 'WisdomChessModule'

const reactDir = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const idlPath = resolve(reactDir, '../wasm/wisdom-chess.idl')
const typesPath = join(reactDir, 'src/lib/wisdom-chess-module.d.ts')
const enumValuesPath = join(reactDir, 'src/test/wasm-enum-values.ts')

const HEADER = `// Generated from ui/wasm/wisdom-chess.idl. Do not edit.
// Regenerate with: npm run generate:wasm-types
`

function fail(message) {
    console.error(`generate-wasm-types: ${message}`)
    process.exit(1)
}

// Enum names and their members, with the C++ namespace stripped.
function parseEnums(idl) {
    const enums = []
    for (const match of idl.matchAll(/^enum\s+(\w+)\s*\{([^}]*)\}/gm)) {
        const members = [...match[2].matchAll(/"([^"]+)"/g)]
            .map(member => member[1].replace(/.*::/, ''))
        enums.push({ name: match[1], members })
    }
    return enums
}

function runGenerator(idlFile, outFile) {
    const npx = process.platform === 'win32' ? 'npx.cmd' : 'npx'
    execFileSync(
        npx,
        ['--yes', GENERATOR, '-e', '-d', '-n', MODULE_NAME, '-i', idlFile, '-o', outFile],
        { stdio: ['ignore', 'ignore', 'inherit'] },
    )
    return readFileSync(outFile, 'utf8')
}

// The generator types every enum as `number`. Make each enum a distinct
// branded number, so that one enum cannot be passed where another is expected.
function brandEnums(declarations, enums) {
    let result = declarations

    const replaceOnce = (pattern, replacement, what) => {
        if (!pattern.test(result))
            fail(`expected ${what} in the generator's output`)
        result = result.replace(pattern, replacement)
    }

    for (const { name, members } of enums) {
        const union = members.map(member => `typeof ${member}`).join(' \\| ')
        replaceOnce(
            new RegExp(`type ${name} = ${union};`),
            `type ${name} = number & { readonly __enum: '${name}' };`,
            `the union type for ${name}`,
        )
        for (const member of members) {
            replaceOnce(
                new RegExp(`const ${member}: number;`),
                `const ${member}: ${name};`,
                `the constant ${member} of ${name}`,
            )
        }
    }
    return result
}

// The C++ enums are plain enums, numbered from zero in declaration order.
function enumValuesSource(enums) {
    const lines = enums.flatMap(({ name, members }) => [
        `    // ${name}`,
        ...members.map((member, index) => `    ${member}: ${index},`),
    ])
    return `export const wasmEnumValues = {\n${lines.join('\n')}\n} as const\n`
}

const idl = readFileSync(idlPath, 'utf8')
const enums = parseEnums(idl)
if (enums.length === 0)
    fail(`no enums found in ${idlPath}`)

const workDir = mkdtempSync(join(tmpdir(), 'wasm-types-'))
let types
try {
    types = brandEnums(runGenerator(idlPath, join(workDir, 'module.d.ts')), enums)
} finally {
    rmSync(workDir, { recursive: true, force: true })
}

const outputs = [
    { path: typesPath, content: HEADER + types },
    { path: enumValuesPath, content: HEADER + enumValuesSource(enums) },
]

if (process.argv.includes('--check')) {
    const stale = outputs.filter(({ path, content }) => {
        try {
            return readFileSync(path, 'utf8') !== content
        } catch {
            return true
        }
    })
    if (stale.length > 0) {
        fail(
            `out of date with ${idlPath}:\n`
            + stale.map(({ path }) => `  ${path}\n`).join('')
            + 'Run: npm run generate:wasm-types'
        )
    }
    console.log('generated WASM types are up to date')
} else {
    for (const { path, content } of outputs) {
        writeFileSync(path, content)
        console.log(`wrote ${path}`)
    }
}
