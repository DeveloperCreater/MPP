#!/usr/bin/env node
const { spawnSync } = require('node:child_process');
const { readdirSync, readFileSync, existsSync } = require('node:fs');
const { join, resolve } = require('node:path');

function parseArgs(argv) {
  const out = { cases: 'tests/cases', runner: 'dist\\mpp.exe -r' };
  for (let i = 2; i < argv.length; i++) {
    const a = argv[i];
    if (a === '--cases' && i + 1 < argv.length) out.cases = argv[++i];
    else if (a === '--runner' && i + 1 < argv.length) out.runner = argv[++i];
  }
  return out;
}

function normOutput(s) {
  if (s.startsWith('\uFEFF')) s = s.replace(/^\uFEFF+/, '');
  s = s.replace(/\r\n/g, '\n').replace(/\r/g, '\n');
  const lines = s.split('\n').map(l => l.replace(/\s+$/, ''));
  return lines.join('\n').trim() + '\n';
}

function splitCommand(cmd) {
  // Minimal Windows-friendly split: handles quoted exe path.
  const parts = [];
  let cur = '';
  let inQuotes = false;
  for (let i = 0; i < cmd.length; i++) {
    const ch = cmd[i];
    if (ch === '"') inQuotes = !inQuotes;
    else if (ch === ' ' && !inQuotes) {
      if (cur.length) { parts.push(cur); cur = ''; }
    } else cur += ch;
  }
  if (cur.length) parts.push(cur);
  return parts;
}

function main() {
  const args = parseArgs(process.argv);
  const casesDir = resolve(args.cases);
  const runnerParts = splitCommand(args.runner);
  if (!existsSync(casesDir)) {
    console.error(`Missing cases dir: ${casesDir}`);
    process.exit(2);
  }

  const files = readdirSync(casesDir).filter(f => f.endsWith('.mpp')).sort();
  let total = 0, failed = 0;

  for (const f of files) {
    total++;
    const mppPath = join(casesDir, f);
    const outPath = join(casesDir, f.replace(/\.mpp$/, '.out'));
    if (!existsSync(outPath)) {
      console.log(`[SKIP] ${f} (missing .out)`);
      continue;
    }
    const cmd = runnerParts[0];
    const cmdArgs = runnerParts.slice(1).concat([mppPath]);
    const p = spawnSync(cmd, cmdArgs, { encoding: 'utf8' });
    if (p.status !== 0) {
      failed++;
      console.log(`[FAIL] ${f} (exit ${p.status})`);
      if ((p.stdout || '').trim()) {
        console.log('--- stdout ---');
        console.log(p.stdout);
      }
      if ((p.stderr || '').trim()) {
        console.log('--- stderr ---');
        console.log(p.stderr);
      }
      continue;
    }
    const got = normOutput(p.stdout || '');
    const want = normOutput(readFileSync(outPath, 'utf8'));
    if (got !== want) {
      failed++;
      console.log(`[FAIL] ${f} (output mismatch)`);
      console.log('--- want ---');
      console.log(want);
      console.log('--- got ---');
      console.log(got);
      continue;
    }
    console.log(`[OK]   ${f}`);
  }

  console.log(`\nResult: ${total - failed}/${total} passed`);
  process.exit(failed === 0 ? 0 : 1);
}

main();

