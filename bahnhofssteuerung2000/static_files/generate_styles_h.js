#!/usr/bin/env node

// Convert styles_uncompressed.css to style_css.h
// Run with: node generate_styles_h.js
// No dependencies required!

const fs = require('fs');
const zlib = require('zlib');

const INPUT = 'styles_uncompressed.css';
const OUTPUT = 'styles.h';

// Check input exists
if (!fs.existsSync(INPUT)) {
    console.error(`Error: ${INPUT} not found`);
    process.exit(1);
}

// Read and basic minify (remove comments, extra whitespace)
console.log('Minifying CSS...');
let css = fs.readFileSync(INPUT, 'utf8');
const origSize = css.length;

// Simple minification (no external deps)
css = css
    .replace(/\/\*[\s\S]*?\*\//g, '')  // Remove comments
    .replace(/\s+/g, ' ')               // Collapse whitespace
    .replace(/\s*([{}:;,>+~])\s*/g, '$1') // Remove space around symbols
    .replace(/;}/g, '}')                // Remove last semicolon
    .trim();

const minSize = css.length;

// Gzip
console.log('Compressing...');
const gzipped = zlib.gzipSync(css, { level: 9 });
const gzSize = gzipped.length;

// Convert to C header
console.log('Generating header...');
let header = 'const unsigned char style_css_gz[] = {\n';

const hexValues = [];
for (let i = 0; i < gzipped.length; i++) {
    hexValues.push('0x' + gzipped[i].toString(16).padStart(2, '0'));
}

// Format in rows of 12
for (let i = 0; i < hexValues.length; i += 12) {
    const row = hexValues.slice(i, i + 12).join(', ');
    const comma = i + 12 < hexValues.length ? ',' : '';
    header += `  ${row}${comma}\n`;
}

header += '};\n';
header += `const size_t style_css_gz_len = ${gzipped.length};\n`;

fs.writeFileSync(OUTPUT, header);

console.log('');
console.log('Done!');
console.log(`  Original:   ${origSize} bytes`);
console.log(`  Minified:   ${minSize} bytes`);
console.log(`  Gzipped:    ${gzSize} bytes`);
console.log(`  Output:     ${OUTPUT}`);
