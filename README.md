# Table based UTF8 decoder core for C++11

Inspired by:
 - https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 - [BobSteagall/utf_utils](https://github.com/BobSteagall/utf_utils) / [Fast Conversion From UTF-8 with C++, DFAs and SSE Intrinsics](https://www.youtube.com/watch?v=5FQ87-Ecb-A)

This implementation:
 - Uses a transposed transition matrix, obviating the need for tricky character class ordering or an extra big/small table.
 - Supports resyncing to next UTF-8 boundary, which allows easy generation of U+FFFD Replacement Character.
 - Optional exact error cause reporting (overlong not allowed, surrogate not allowed, unpaired surrogate, ...).
 - When End-of-String is reached, a distinguished error code is returned;
   an implementor can then choose to restart parsing from the original position when more data becomes available.
 - Can be configured for different use cases with a compile-type Decode Strategy: no penalty for unused features.
 - Ready for [Modified UTF-8](https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8), i.e. overlong encoding of `\x00` as `\xCO\x80`.
 - Full surrogate pairs can be decoded (e.g [CESU-8](https://en.wikipedia.org/wiki/CESU-8) - might require small modifications for full compliance),
   unpaired surrogates can be forbidden/allowed or raw surrogates can be returned.
 - Templated for arbitrary ForwardIterators (InputIterator suffices when surrogate decoding is not needed).
 - Easily combinable with Bob Steagull's SSE optimization.
 - Provides the core decoding algorithm (`int Utf8::next<Strategy>(Iterator &it, const Iterator &end)`) to be called by
   a simple custom `utf8_decode` routine, which should be implemented specifically for the intended use case
   (e.g. error handling: throw / return / ..., desired Decode Strategy, SSE optimization, ...):
```c++
void utf8_decode(const std::string &str)
{
  const char *begin = str.data(), *end = begin + str.size();
  while (begin != end) {
    const uint8_t ch = *begin;
    if (ch < 0x80) {
      printf("%d ", ch);
      ++begin;
    } else {
      const int cp = Utf8::next(begin, end);  // cp < 0 on error
      printf("%d ", cp);
    }
  }
  printf("\n");
}
```

Possible future ideas:
 - `utf8_range(inputBytesRange)`.
 - Add reverse decoding algorithm/tables.

Copyright (c) 2020 Tobias Hoffmann

License: http://opensource.org/licenses/MIT

