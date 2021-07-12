import ctypes
import sys
import logging

RTLD_NOW = 2
RTLD_GLOBAL = ctypes.RTLD_GLOBAL
DEFAULT_MINERS_PATH = "/usr/lib/nativeextractor_miners"

sys.setdlopenflags( RTLD_NOW | RTLD_GLOBAL )
import pynativeextractor_c as ne

class Stream(object):
    def __init__(self, stream_ptr=None):
        self._stream = stream_ptr
        pass

    def __enter__(self):
        if not ne.stream_check(self._stream):
            err = "Stream failed"
            raise IOError(err)
        return self

    def __exit__(self, type, value, traceback):
        return False

class FileStream(Stream):
    def __init__(self, fullpath):
        ptr = None
        if sys.version_info < (3, 0):
            ptr = ne.stream_file_new(fullpath.encode('utf-8'))
        else:
            ptr = ne.stream_file_new(fullpath)
        super(FileStream, self).__init__(stream_ptr=ptr)

    def __del__(self):
        if self._stream is not None:
            ne.free_file_stream(self._stream)

    def __exit__(self, type, value, traceback):
        ne.free_file_stream(self._stream)
        self._stream = None
        return False

class BufferStream(Stream):
    def __init__(self, buffer):
        # cbuffer = ctypes.create_string_buffer(buffer)
        cbuffer = None
        if sys.version_info < (3, 0):
            cbuffer = buffer.encode("utf-8")
        else:
            cbuffer = buffer

        ptr = ne.stream_buffer_new(cbuffer)
        super(BufferStream, self).__init__(stream_ptr=ptr)

    def __del__(self):
        if self._stream is not None:
            ne.free_buffer_stream(self._stream)

    def __exit__(self, type, value, traceback):
        ne.free_buffer_stream(self._stream)
        self._stream = None
        return False

class Extractor (object):

    def __init__(self, miners=[], batch=1000, threads=1):
        """Initializes an extractor.

        Args:
            miners (list of tuples of 2 strings):
                which miners to use, see add_miner_so for format
            batch (int): default batch size (optional)"""

        # self.miners = miners # tuples [('path/to/file.so', 'symbol_to_export')]
        self.miners = []
        self.batch = batch
        self.stream = None
        self._extractor = ne.create_extractor(threads)
        for miner in miners:
            if not self.add_miner_so(*miner):
                logging.warning(
                    "Couldn't add %s::%s (%s): %s",
                    miner[0],
                    miner[1],
                    miner[2] if len(miner) > 2 else "",
                    self.get_last_error()
                )

    def __del__(self):
        ne.free_extractor(self._extractor)

    def add_miner_so(self, so_dir, symb, param=""):
        """Adds a miner to the extractor

        Args:
            so_dir (str): path to the *.so file
            symb (str): name of the miner symbol
            param (str) (optional): parameters as a string

        Returns:
            bool: True on success
        """
        ret = None

        so_dir_enc = so_dir
        symb_enc = symb
        param_enc = param

        if sys.version_info < (3, 0):
            so_dir_enc = so_dir.encode('utf-8')
            symb_enc = symb.encode('utf-8')
            param_enc = param.encode('utf-8')

        miner_tuple = (so_dir_enc, symb_enc, param_enc)

        ret = ne.add_miner_so(
            self._extractor,
            *miner_tuple)

        if ret:
            self.miners += [miner_tuple]
        return ret

    def set_stream(self, stream):
        self.stream = stream
        ne.set_stream(self._extractor, stream._stream)
        return self

    def unset_stream(self):
        ne.unset_stream(self._extractor)
        self.stream = None
        return self

    def get_last_error(self):
        return ne.get_last_error(self._extractor)

    def _check_stream(self):
        if self.stream is None:
            raise ValueError("No stream set")

    def __enter__(self):
        self._check_stream()
        return self

    def __exit__(self, type, value, traceback):
        self.unset_stream()
        return False

    def eof(self):
        """
        Returns:
            bool: True if currently processed file is at EOF
        """

        self._check_stream()
        return ne.eof(self._extractor)

    def meta(self):
        """
        Returns:
            dict: the labels mapped to dicts of {miner, path, label}
        """
        ret = {}
        dlsyms = ne.dlsymbols(self._extractor)
        for entry in dlsyms:
            ret[entry["label"]] = entry
        return ret

    def next(self, batch=None):
        """Returns the next batch of found entities

        Args:
            batch (int): override the default batch size parameter (optional)

        Returns:
            list of dicts: the mined entities
        """

        if batch is None:
            batch = self.batch
        self._check_stream()
        return ne.next(self._extractor, batch)


        # @examples
        # with FileStream("some/path.txt") as fs:
        #   e = Extractor().set_stream(fs)
        #   while not e.eof():
        #       ret = e.next()
        #
        # e = Extractor()
        # with FileStream("some/path.txt") as fs:
        #     with e.set_stream(fs):
        #         while not e.eof():
        #             ret = e.next()
        #             foo(ret)

