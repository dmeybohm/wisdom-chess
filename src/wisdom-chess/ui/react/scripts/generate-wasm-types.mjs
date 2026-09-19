// Generates src/lib/wisdom-chess-module.d.ts from the WebIDL interface.
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
const outputPath = join(reactDir, 'src/lib/wisdom-chess-module.d.ts')

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

const idl = readFileSync(idlPath, 'utf8')
const enums = parseEnums(idl)
if (enums.length === 0)
    fail(`no enums found in ${idlPath}`)

const workDir = mkdtempSync(join(tmpdir(), 'wasm-types-'))
let generated
try {
    generated = HEADER + brandEnums(runGenerator(idlPath, join(workDir, 'module.d.ts')), enums)
} finally {
    rmSync(workDir, { recursive: true, force: true })
}

if (process.argv.includes('--check')) {
    let current = ''
    try {
        current = readFileSync(outputPath, 'utf8')
    } catch {
        // A missing file is reported as stale below.
    }
    if (current !== generated)
        fail(`${outputPath} is out of date with ${idlPath}.\nRun: npm run generate:wasm-types`)
    console.log('wisdom-chess-module.d.ts is up to date')
} else {
    writeFileSync(outputPath, generated)
    console.log(`wrote ${outputPath}`)
}
