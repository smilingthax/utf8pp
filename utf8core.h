#pragma once

#include <stdint.h>

namespace Utf8 {

#include "utf8core.tcc"

struct Basic_Decode_Strategy {
  static constexpr bool exact_error = false;        // distinguish different error cases in retval (-3: surrogate, -4: overlong, -6: unpaired)

  static constexpr bool resync = true;              // increment iterator to next possible start when returning error

  static constexpr bool allow_all_overlong = false;
  static constexpr bool allow_overlong0 = false;    // i.e. modified utf8, allows 0xC0 0x80 instead of 0x00

  static constexpr bool raw_surrogates = false;     // (unchecked, no decode / leave unpaired)
  static constexpr bool decode_surrogates = false;  // e.g. CESU-8
  static constexpr bool allow_unpaired = false;     // (i.e. return unpaired, when decode_surrogates is not successful)
};

template <typename Strategy, typename FwdIterator>
static inline int surrogate_trap_impl_(FwdIterator &begin, const FwdIterator &end, int ret)
{
  if (ret < 0xdc00 && (uint8_t)*begin == 0xed) { // lead, following surrogate is possible
    auto saved = begin++;
    if (begin == end) {
      return (Strategy::allow_unpaired) ? ret : -1;  // EOF
    }

    const uint8_t ch = *begin;
    const uint8_t cls = Tables.char2class[ch];
    const int state = Tables.transitions[Tables.SUR_3 + cls];

    if (state == Tables.SURR && // found: lead or trail
        (ch & 0xf0) == 0xb0) {  // really trail
      ++begin;
      if (begin == end) {
        return (Strategy::allow_unpaired) ? ret : -1;  // EOF
      }

      const uint8_t ch2 = *begin;
      if (ch2 >= 0x80 && ch2 <= 0xbf) {
        ++begin;
        ret = ret - 0xd800 + 0x40;
        ret = (ret << 4) | (ch & 0x0f);
        return (ret << 6) | (ch2 & 0x3f);
      }
    }
    begin = saved;
  }

  if (Strategy::allow_unpaired) {
    return ret;
  } else if (Strategy::exact_error) {
    return -6;  // unpaired
  }
  return -2;  // invalid
}

// NOTE: InputIterator is enough, when Strategy::decode_surrogates is false
template <typename Strategy = Basic_Decode_Strategy, typename FwdIterator>
static inline int next(FwdIterator &begin, const FwdIterator &end)
{
  if (begin == end) {
    return -1;  // EOF
  }

  const uint8_t ch0 = *begin;
  const uint8_t cls0 = Tables.char2class[ch0];
  uint8_t state = Tables.transitions[Tables.START + cls0];
  int ret = ch0 & (0x7f >> (state & Tables.SHIFT_MASK));

  while (1) {
    ++begin;

    if (state >= Tables.DONE) {   // trap
      if (state == Tables.DONE) {
        return ret;
      }

      state &= ~( (Strategy::allow_all_overlong ? Tables.OVERLONG_F : 0) |
                  (Strategy::raw_surrogates ? Tables.SURROGATE_F : 0) );
      if (state <= Tables.DONE) {
        if (state == Tables.DONE) {
          return ret;
        } // else: (state < DONE)

      } else if (Strategy::decode_surrogates && state == Tables.SURR) {
        state &= ~Tables.SURROGATE_F;

      } else {
        if (begin == end) {
          break;
        } else if (Strategy::allow_overlong0 && state == Tables.OL1 && (uint8_t)*begin == 0x80) {
          ++begin;
          return 0x00;
        } else if (Strategy::decode_surrogates && state == Tables.SDONE) {
          return surrogate_trap_impl_<Strategy>(begin, end, ret);  // resync never needed
        }

        if (Strategy::resync) {
          while (((uint8_t)*begin & 0xc0) == 0x80) {
            ++begin;
            if (begin == end) {
              break;
            }
          }
        }

        if (Strategy::exact_error) {
          if (state & Tables.SURROGATE_F) {
            return -3;  // surrogate not allowed
          } else if (state & Tables.OVERLONG_F) {
            return -4;  // overlong not allowed
          } else if (state == Tables.OVERF && ret < 0x7e) { // Note: FE ?? and FF ?? are not 5/6 byte sequences, but not defined at all / error
            return -5;  // overflow
          } // else: invalid encoding (-2)
        }

        return -2; // invalid encoding
      }
    }

    if (begin == end) {
      break;
    }

    const uint8_t ch = *begin;
    const uint8_t cls = Tables.char2class[ch];
    state = Tables.transitions[state + cls];

    ret = (ret << 6) | (ch & 0x3f);
  }

  return -1; // EOF
}

} // namespace Utf8

