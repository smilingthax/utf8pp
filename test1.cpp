#include "utf8core.h"
#include <stdio.h>

struct CESU8_Decode_Strategy : Utf8::Basic_Decode_Strategy {
  static constexpr bool decode_surrogates = true;
  static constexpr bool allow_unpaired = true;  // TODO?
};

#include <string>

void utf8_decode(const std::string &str)
{
  const char *begin = str.data(), *end = begin + str.size();
  while (begin != end) {
    const uint8_t ch = *begin;
    if (ch < 0x80) {
      printf("%x(%c) ", ch, ch);
      ++begin;
    } else {
      const int cp = Utf8::next(begin, end);
//      const int cp = Utf8::next<CESU8_Decode_Strategy>(begin, end);
      if (cp < 0) {
        printf("%d ", cp);
      } else {
        printf("%x ", cp);
      }
    }
  }
  printf("\n");
}


int main()
{
  utf8_decode("§a\xc0\x80sdf");

  utf8_decode("§a\xc0\x81sdf");

  utf8_decode("§a\xff\xfesdf");

  utf8_decode("a\xed\xa0\x80sdf");  // unpaired lead

  utf8_decode("a\xed\xb0\x80sdf");  // unpaired trail

  utf8_decode("a\xed\xa0\x80\xed\xb0\x81sdf");  // good surrogate

  utf8_decode("a\xed\xa0\x80\xed\xa0\x80\xed\xb0\x80sdf");  // unpaired lead + good surrogate

  return 0;
}

