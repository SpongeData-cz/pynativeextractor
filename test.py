from pynativeextractor.extractor import *
import os

# TODO use a proper test framework

def test(ex, s, expected):
  print("Checking " + s)
  print("Expecting " + str(expected) + " matches")
  with BufferStream(s) as bf:
    with ex.set_stream(bf):
      while not ex.eof():
        res = ex.next()
        print("Found " + str(len(res)))
        assert(len(res) == expected)

def general_test():
  ex = Extractor()
  # date glob - DO NOT USE FOR REAL DATES!
  ex.add_miner_so(
    os.path.join(DEFAULT_MINERS_PATH, "glob_entities.so"),
    u"match_glob",
    "????-??-??"
  )
  test(ex, "2020-05-05", 1)
  # phone number glob - DO NOT USE FOR REAL PHONE NUMBERS!
  ex.add_miner_so(
    os.path.join(DEFAULT_MINERS_PATH, "glob_entities.so"),
    u"match_glob",
    "+? (???) ???-????"
  )
  test(ex, "+1 (846) 569-3535", 1)

def enclosed_test():
  ex = Extractor()
  x = "123 456"
  ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "glob_entities.so"), "match_glob", "123")
  ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "glob_entities.so"), "match_glob", "456")
  ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "glob_entities.so"), "match_glob", "123 456")

  assert(ex.flags == 0)
  test(ex, x, 3)
  ex.set_flags(Extractor.NO_ENCLOSED_OCCURRENCES)
  test(ex, x, 1)

general_test()
enclosed_test()
print("All tests passed!")