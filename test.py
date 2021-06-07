from pynativeextractor.extractor import *
import os

ex = Extractor()
# TODO: test depends on non-free modules
ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "web_entities.so"), u"match_email")
ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "date_entities.so"), u"match_date")
ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, "phone_numbers.so"), u"match_phone_number")

print(ex.miners)

for x in [u"+1 (846) 569-3535", u"2020-05-05"]:
  print(x)
  with BufferStream(x) as bf:
    with ex.set_stream(bf):
      while not ex.eof():
        print(ex.next())