"use strict";

class SetDedup {
  constructor() {
    this.data = [
      { parent: null, key: '', dsts: new Map }  // 0: empty set
    ];
  }

  add(setId, elem) {
    // assert(setId >= 0 && setId < this.data.length);
    const key = JSON.stringify(elem); // TODO?!
    const res = this.data[setId].dsts.get(key);
    if (res !== undefined) {
      return res; // found, deduped
    }
    if (key <= this.data[setId].key) { // unsorted: create additional sorted hierarchy
      const elems = [];
      let pos = setId;
      for (; key < this.data[pos].key; pos = this.data[pos].parent) {
        // assert(pos !== 0);
//if (this.data[pos].dsts.has(key)) break;
        elems.push([this.data[pos].key, this.data[pos].elem]);
      }
      if (key === this.data[pos].key) {
        return setId; // already in set...  (TODO? warn?)
      }
      const res = this.data[pos].dsts.get(key);
      pos = (res !== undefined) ? res : this._add_one(pos, key, elem);

      // re-add remaining elems (already sorted)
      for (const [ekey, elem] of elems.reverse()) {
        const res = this.data[pos].dsts.get(ekey);
        pos = (res !== undefined) ? res : this._add_one(pos, ekey, elem);
      }
//this.data[setId].dsts.set(key, pos);
      return pos;
    }
    return this._add_one(setId, key, elem);
  }

  get(setId) {
    // assert(setId >= 0 && setId < this.data.length);
    const elems = [];
    for (; setId > 0; setId = this.data[setId].parent) {
      elems.push(this.data[setId].elem);
    }
    return elems.reverse();
  }

  _add_one(pos, key, elem) {
    const ret = this.data.length;
    this.data[pos].dsts.set(key, ret);
    this.data.push({
      parent: pos,
      key: key,
      elem: elem,
      dsts: new Map
    });
    return ret;
  }
}

module.exports = SetDedup;

