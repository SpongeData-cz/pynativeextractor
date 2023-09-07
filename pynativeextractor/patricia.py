import ctypes
import sys
import logging

RTLD_NOW = 2
RTLD_GLOBAL = ctypes.RTLD_GLOBAL
DEFAULT_MINERS_PATH = "/usr/lib/nativeextractor_miners"

sys.setdlopenflags( RTLD_NOW | RTLD_GLOBAL )
import pynativeextractor_c as ne

class Patricia(object):
    def __init__(self, fullpath = None):
        """Initializes a Patricia. If fullpath is provided, Patricia becomes 
        read-only.

        Args:
            fullpath (str): path to saved Patricia for mmapping
        """
        self._pointer = None
        self._read_only = False

        self.fullpath = None

        if fullpath is not None:
            self._read_only = True
            self.fullpath = fullpath
            self._pointer = ne.create_patricia_from_file(self.fullpath)
            return

        self._pointer = ne.create_patricia(0)

    def insert(self, str):
        """Inserts value into Patricia

        Args:
            str (str): the value
        """
        if self._read_only:
            raise TypeError("Cannot insert on mmapped Patricia")
        return ne.insert_patricia(self._pointer, str)

    def search(self, str):
        """Searches value in a Patricia

        Args:
            str (str): the value

        Returns:
            int: number of matching characters (len(str) if full match)
        """
        return ne.search_patricia(self._pointer, str)
    
    def search_ext(self, str):
        """Searches value in a Patricia, returns extended information.

        Args:
            str (str): the value

        Returns:
            {
                found (int): number of matching characters (len(str) if full match)
                terminal (bool): True if the last matched node is the end of a word in the trie
                edges (int): how many edges stem from last matched node
            } (dict)
        """
        ret = ne.search_ext_patricia(self._pointer, str)
        ret["terminal"] = bool(ret["terminal"])
        return ret

    def print(self):
        """Prints Patricia to stdout
        """
        return ne.print_patricia(self._pointer)

    def __del__(self):
        if self._pointer is not None:
            ne.free_patricia(self._pointer)
            self._pointer = None

    def save(self, fullpath):
        """Saves Patricia in internal format (can be mmapped later).

        Args:
            fullpath (str): path to store Patricia
        """
        return ne.save_patricia(self._pointer, fullpath)