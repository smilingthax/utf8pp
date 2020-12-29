#!/usr/bin/env node
"use strict"

function utf8enc(ch) { // {{{
  if (ch < 0) {
    throw new Error('bad ch');
  } else if (ch <= 0x7f) {
    return [ch];
  } else if (ch <= 0x7ff) {
    return [
      0xc0 | (ch >> 6),
      0x80 | (ch & 0x3f)
    ];
  } else if (ch <= 0xffff) {
    return [
      0xe0 | (ch >> 12),
      0x80 | ((ch >> 6) & 0x3f),
      0x80 | (ch & 0x3f)
    ];
  } else if (ch <= 0x10ffff) {
    return [
      0xf0 | (ch >> 18),
      0x80 | ((ch >> 12) & 0x3f),
      0x80 | ((ch >> 6) & 0x3f),
      0x80 | (ch & 0x3f)
    ];
  }
  return []; // too large
}
// }}}

function merge(...ars) { // {{{
  if (ars.length === 0) {
    return null;
  }
  const len = ars[0].length;
  if (!ars.every((ar) => ar.length)) {
    throw new Error('arrays for merge must have same length');
  }

  const ret = new Uint8Array(len);
  if (!len) {
    return [[], ret];
  }

  const tuples = [];
  const tuplemap = new Map;

  ret[0] = tuples.length;
  const tuple = ars.map((ar) => ar[0]);
  tuplemap.set(JSON.stringify(tuple), tuples.length);
  tuples.push(tuple);

  for (let i = 1; i < len; i++) {
    if (ars.every((ar) => (ar[i - 1] === ar[i]))) {
      ret[i] = ret[i - 1];
      continue;
    }
    const tuple = ars.map((ar) => ar[i]);
    const key = JSON.stringify(tuple);
    const idx = tuplemap.get(key);
    if (idx !== undefined) {
      ret[i] = idx;
      continue;
    }
    ret[i] = tuples.length;
    tuplemap.set(key, tuples.length);
    tuples.push(tuple);
  }
  return [tuples, ret];
}
// }}}

function extract_sets(ar) { // {{{
  const len = ar.length;
  if (!len) {
    return [];
  }

  const tmp = ar.slice();

  tmp.sort((a, b) => a - b);

  // unique
  const ret = [ tmp[0] ];
  for (let i = 1; i < len; i++) {
    if (tmp[i - 1] !== tmp[i]) {
      ret.push(tmp[i]);
    }
  }

  return ret.filter((u) => (u !== undefined));
}
// }}}

function pad(v, pad, chr = ' ') { v = String(v); if (v.length >= pad) return v; return Array(pad - v.length + 1).join(chr) + v; }

const SetDedup = require('./SetDedup');

const allsets = new SetDedup;

// Basic idea: BFS to generate node-deduped Trie of valid Words in the Language "L_utf8"
const queue = [];

const nodes = new Map;  // (setId -> { childs: new Array(256), sets: [...] })

const childs = new Array(256);
for (let i = 0; i < 0x110000; i++) {
  if (i >= 0xd800 && i <= 0xdfff) { // disallow surrogates
    continue;
  }
  const cur = utf8enc(i);
  const suffixset = childs[cur[0]] || 0;
  childs[cur[0]] = allsets.add(suffixset, cur.slice(1));
}
const sets = extract_sets(childs);
nodes.set(-1, { // start node
  childs,
  sets
});

queue.push(...sets);

while (queue.length) {
  const setId = queue.shift();
  const set = allsets.get(setId);

  const childs = new Array(256);
  for (const tuple of set) {
    const suffixset = childs[tuple[0]] || 0;
    childs[tuple[0]] = allsets.add(suffixset, tuple.slice(1));
  }
  const sets = extract_sets(childs);
  nodes.set(setId, {
    childs,
    sets
  });

  for (const id of sets) {
    if (!nodes.has(id)) {
      queue.push(id);
    }
  }
}

const keys = [], links = [];
for (const [key, node] of nodes) {
  keys.push(key);
  links.push(node.childs);
}

const rs2 = merge(...links);
console.log('char2class:');
for (let i = 0; i < 256; ) {
  const line = Array(16);
  for (let j = 0; j < 16; i++, j++) {
    line[j] = pad(rs2[1][i], 3);
  }
  console.log(' ', line.slice(0, 8).join(',') + ',  ' + line.slice(8).join(',') + ',');
  if (i == 128) console.log();
}

const classes = rs2[1];
const cnum = rs2[0].length; // num classes
const snum = keys.length; // num nodes/states

// rename states...
const setid2state = new Map;
for (let i = 0; i < snum; i++) {
  setid2state.set(keys[i], i);
}

console.log();
console.log('transitions:');

console.log(' cls \\ state ', Array.from(setid2state.values(), (v) => pad(v, 3)).join(' '));
for (let i = 0; i < cnum; i++) { // classes
  const line = Array(snum + 1);
  for (let j = 0; j < snum; j++) { // nodes
    const cell = rs2[0][i][j];
    line[j] = pad(setid2state.get(cell) || '', 3);
  }
  console.log('  ' + pad(i, 2) + '         ', line.join(','));
}

