/**
 * Copyright (c) 2021 SpongeData s.r.o.
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

// Define Python 3.10+ macro
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <string.h>
#include <nativeextractor/extractor.h>
#include <nativeextractor/miner.h>
#include <nativeextractor/occurrence.h>
#include <nativeextractor/patricia.h>

static PyObject *stream_file_new(PyObject *self, PyObject *args) {
  const char *fullpath;

  if (!PyArg_ParseTuple(args, "s", &fullpath)) {
    return NULL;
  }

  stream_file_c *sf = stream_file_c_new(fullpath);

  if (sf->stream.state_flags & STREAM_FAILED) {
    DESTROY(sf);
    PyErr_SetString(PyExc_IOError, "Stream failed");
    return NULL;
  }

  return Py_BuildValue("l", sf);
}

static PyObject *stream_check(PyObject *self, PyObject *args) {
  stream_c *stream;

  if (!PyArg_ParseTuple(args, "l", &stream)) {
    return NULL;
  }

  if (stream->state_flags & STREAM_FAILED) {
    Py_RETURN_FALSE;
  }

  Py_RETURN_TRUE;
}

static PyObject *free_file_stream(PyObject *self, PyObject *args) {
  stream_file_c *stream;

  if (!PyArg_ParseTuple(args, "l", &stream)) {
    return NULL;
  }

  DESTROY(stream);

  Py_RETURN_NONE;
}

static PyObject *free_buffer_stream(PyObject *self, PyObject *args) {
  stream_buffer_c *stream;

  if (!PyArg_ParseTuple(args, "l", &stream)) {
    return NULL;
  }

  DESTROY(stream);

  Py_RETURN_NONE;
}

static PyObject *stream_buffer_new(PyObject *self, PyObject *args) {
  const char *buffer;
  Py_ssize_t buflen;

  if (!PyArg_ParseTuple(args, "s#", &buffer, &buflen)) {
    return NULL;
  }

  char * buffer_mem = malloc(buflen);
  memcpy(buffer_mem, buffer, buflen);

  stream_buffer_c *sb = stream_buffer_c_new(buffer_mem, buflen);
  if (sb->stream.state_flags & STREAM_FAILED) {
    DESTROY(sb);
    PyErr_SetString(PyExc_IOError, "Stream failed");
    return NULL;
  }

  return Py_BuildValue("l", sb);
}

static PyObject *create_extractor(PyObject *self, PyObject *args) {
  int threads;

  if (!PyArg_ParseTuple(args, "i", &threads)) {
    return NULL;
  }

  miner_c **m = malloc(sizeof(miner_c *) * 1);
  m[0] = (miner_c *) NULL;
  extractor_c *extractor = extractor_c_new(threads, m);

  return Py_BuildValue("l", extractor);
}

static PyObject *free_extractor(PyObject *self, PyObject *args) {
  extractor_c *extractor;

  if (!PyArg_ParseTuple(args, "l", &extractor)) {
    return NULL;
  }

  DESTROY(extractor);

  Py_RETURN_NONE;
}

static PyObject *eof(PyObject *self, PyObject *args) {
  extractor_c *extractor;

  if (!PyArg_ParseTuple(args, "l", &extractor)) {
    return NULL;
  }

  if ((extractor->stream->state_flags) & STREAM_EOF) {
    Py_RETURN_TRUE;
  }

  Py_RETURN_FALSE;
}

static PyObject *next(PyObject *self, PyObject *args) {
  extractor_c *extractor;
  unsigned batch;

  if (!PyArg_ParseTuple(args, "lI", &extractor, &batch)) {
    return NULL;
  }

  occurrence_t **res = extractor->next(extractor, batch);
  occurrence_t **pres = res;

  PyObject *list = PyList_New(0);
  while (*pres) {
    size_t len = (*pres)->len;
    char *str = malloc(sizeof(char) * (len + 1));
    memcpy(str, (*pres)->str, len);
    str[len] = '\0';

    PyObject *occurrence = Py_BuildValue(
      "{slslslslsssfss}",
      "pos", (*pres)->pos,
      "len", (*pres)->len,
      "upos", (*pres)->upos,
      "ulen", (*pres)->ulen,
      "label", (*pres)->label,
      "prob", (*pres)->prob,
      "value", str
    );

    free(str);

    PyList_Append(list, occurrence);

    free(*pres);
    pres++;
  }

  free(res);

  return list;
}

static PyObject *add_miner_so(PyObject *self, PyObject *args) {
  extractor_c *extractor;
  const char *so_dir;
  const char *symb;
  const char *params;

  if (!PyArg_ParseTuple(args, "lsss", &extractor, &so_dir, &symb, &params)) {
    return NULL;
  }

//  params =

  if (extractor->add_miner_so(extractor, so_dir, symb, params)) {
    Py_RETURN_TRUE;
  }

  Py_RETURN_FALSE;
}

static PyObject *get_last_error(PyObject *self, PyObject *args) {
  extractor_c *extractor;

  if (!PyArg_ParseTuple(args, "l", &extractor)) {
    return NULL;
  }

  #if PY_MAJOR_VERSION < 3
    return PyString_FromString(extractor->get_last_error(extractor));
  #else
    return PyUnicode_FromString(extractor->get_last_error(extractor));
  #endif
}

static PyObject *set_stream(PyObject *self, PyObject *args) {
  extractor_c *extractor;
  stream_c *stream;

  if (!PyArg_ParseTuple(args, "ll", &extractor, &stream)) {
    return NULL;
  }

  if (extractor->set_stream(extractor, stream)) {
    Py_RETURN_TRUE;
  }

  Py_RETURN_FALSE;
}

static PyObject *unset_stream(PyObject *self, PyObject *args) {
  extractor_c *extractor;

  if (!PyArg_ParseTuple(args, "l", &extractor)) {
    return NULL;
  }

  extractor->unset_stream(extractor);

  Py_RETURN_NONE;
}

static PyObject *dlsymbols(PyObject *self, PyObject *args) {
  extractor_c *extractor;

  if (!PyArg_ParseTuple(args, "l", &extractor)) {
    return NULL;
  }

  dl_symbol_t **dlsyms = extractor->dlsymbols;

  PyObject *list = PyList_New(0);

  while (*dlsyms) {
    const char *path = (*dlsyms)->ldpath;
    const char **meta = (*dlsyms)->meta;
    while (*meta) {
      const char *miner = meta[0];
      const char *label = meta[1];
      PyObject *entry = Py_BuildValue(
        "{ssssss}",
        "path", path,
        "miner", miner,
        "label", label
      );
      PyList_Append(list, entry);
      meta += 2;
    }
    ++dlsyms;
  }

  return list;
}

static PyObject *set_flags(PyObject *self, PyObject *args) {
  extractor_c *extractor;
  unsigned flags;

  if (!PyArg_ParseTuple(args, "lI", &extractor, &flags)) {
    return NULL;
  }

  extractor->set_flags(extractor, flags);

  return Py_BuildValue("l", extractor->flags);
}


static PyObject *unset_flags(PyObject *self, PyObject *args) {
  extractor_c *extractor;
  unsigned flags;

  if (!PyArg_ParseTuple(args, "lI", &extractor, &flags)) {
    return NULL;
  }

  extractor->unset_flags(extractor, flags);

  return Py_BuildValue("l", extractor->flags);
}

/********************************** PATRICIA **********************************/

static PyObject *create_patricia(PyObject *self, PyObject *args) {
  stream_c *stream;

  if (!PyArg_ParseTuple(args, "l", &stream)) {
    return NULL;
  }

  patricia_c *patricia = patricia_c_create(stream);

  return Py_BuildValue("l", patricia);
}

static PyObject *create_patricia_from_stream(PyObject *self, PyObject *args) {
  stream_c *stream;

  if (!PyArg_ParseTuple(args, "l", &stream)) {
    return NULL;
  }

  patricia_c *patricia = patricia_c_create_from_stream(stream);

  return Py_BuildValue("l", patricia);
}

static PyObject *create_patricia_from_file(PyObject *self, PyObject *args) {
  const char *fullpath;

  if (!PyArg_ParseTuple(args, "s", &fullpath)) {
    return NULL;
  }

  patricia_c *patricia = patricia_c_from_file(fullpath);

  return Py_BuildValue("l", patricia);
}

static PyObject *free_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;

  if (!PyArg_ParseTuple(args, "l", &patricia)) {
    return NULL;
  }

  DESTROY(patricia);

  Py_RETURN_NONE;
}

static PyObject *insert_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;
  char *str;

  if (!PyArg_ParseTuple(args, "ls", &patricia, &str)) {
    return NULL;
  }

  patricia->insert(patricia, str, 0, strlen(str));

  Py_RETURN_NONE;
}

static PyObject *search_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;
  char *str;

  if (!PyArg_ParseTuple(args, "ls", &patricia, &str)) {
    return NULL;
  }

  uint32_t ret = patricia->search(patricia, str, strlen(str));

  return Py_BuildValue("l", ret);
}

static PyObject *search_ext_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;
  char *str;

  if (!PyArg_ParseTuple(args, "ls", &patricia, &str)) {
    return NULL;
  }

  patricia_node_t *node = NULL;
  uint32_t ret = patricia->search_ext(patricia, str, strlen(str), &node);

  if (node == NULL) {
    return Py_BuildValue(
      "{slsbsl}",
      "found", ret,
      "terminal", 0,
      "edges", 0
    );
  }

  return Py_BuildValue(
    "{slsbsl}",
    "found", ret,
    "terminal", node->is_terminal,
    "edges", node->edge_count
  );
}

static PyObject *save_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;
  const char *fullpath;

  if (!PyArg_ParseTuple(args, "ls", &patricia, &fullpath)) {
    return NULL;
  }

  patricia->save(patricia, fullpath);

  Py_RETURN_NONE;
}

static PyObject *print_patricia(PyObject *self, PyObject *args) {
  patricia_c *patricia;

  if (!PyArg_ParseTuple(args, "l", &patricia)) {
    return NULL;
  }

  patricia->print(patricia);

  Py_RETURN_NONE;
}


static PyMethodDef nativeextractor_methods[] = {
  {
    "stream_file_new",
    stream_file_new,
    METH_VARARGS,
    "stream_file_new( fullpath ): Returns a stream from fullpath."
  },
  {
    "free_file_stream",
    free_file_stream,
    METH_VARARGS,
    "free_file_stream( stream_ptr ): Deallocates a file stream."
  },
  {
    "free_buffer_stream",
    free_buffer_stream,
    METH_VARARGS,
    "free_buffer_stream( stream_ptr ): Deallocates a buffer stream."
  },
  {
    "stream_check",
    stream_check,
    METH_VARARGS,
    "stream_check( stream ): Returns False if stream failed, True otherwise."
  },
  {
    "stream_buffer_new",
    stream_buffer_new,
    METH_VARARGS,
    "stream_buffer_new( buffer ): Returns a stream from a string buffer."
  },
  {
    "create_extractor",
    create_extractor,
    METH_VARARGS,
    "create_extractor( threads, miners, miners_count ): Creates an extractor."
  },
  {
    "free_extractor",
    free_extractor,
    METH_VARARGS,
    "free_extractor( extractor ): Deallocates an extractor."
  },
  {
    "eof",
    eof,
    METH_VARARGS,
    "eof( extractor ): Returns True if stream in extractor has EOF."
  },
  {
    "next",
    next,
    METH_VARARGS,
    "next( extractor, batch ): Returns the next batch of occurrences."
  },
  {
    "add_miner_so",
    add_miner_so,
    METH_VARARGS,
    "add_miner_so( extractor, so_dir, symb, params ): Adds a miner to the extractor."
  },
  {
    "get_last_error",
    get_last_error,
    METH_VARARGS,
    "get_last_error( extractor ): Returns a string detailing the last error."
  },
  {
    "set_stream",
    set_stream,
    METH_VARARGS,
    "set_stream( extractor, stream ): Sets stream for an extractor."
  },
  {
    "unset_stream",
    unset_stream,
    METH_VARARGS,
    "unset_stream( extractor ): Unsets any stream for an extractor."
  },
  {
    "dlsymbols",
    dlsymbols,
    METH_VARARGS,
    "dlsymbols( extractor ): Returns a list of dicts with dlsymbols."
  },
  {
    "set_flags",
    set_flags,
    METH_VARARGS,
    "set_flags( extractor, flags ): Sets flags for extractor. Returns new flags."
  },
  {
    "unset_flags",
    unset_flags,
    METH_VARARGS,
    "unset_flags( extractor, flags ): Unsets flags for extractor. Returns new flags."
  },
  /********************************* PATRICIA *********************************/
  {
    "create_patricia",
    create_patricia,
    METH_VARARGS,
    "create_patricia( stream ): Creates a patricia."
  },
  {
    "create_patricia_from_file",
    create_patricia_from_file,
    METH_VARARGS,
    "create_patricia_from_file( fullpath ): Creates an mmapped patricia."
  },
  {
    "create_patricia_from_stream",
    create_patricia_from_stream,
    METH_VARARGS,
    "create_patricia_from_stream( stream ): Creates a patricia from stream."
  },
  {
    "free_patricia",
    free_patricia,
    METH_VARARGS,
    "free_patricia( pointer ): Frees patricia at address."
  },
  {
    "insert_patricia",
    insert_patricia,
    METH_VARARGS,
    "free_patricia( pointer, value ): Inserts value into patricia."
  },
  {
    "search_patricia",
    search_patricia,
    METH_VARARGS,
    "search_patricia( pointer, value ): Searches for value in patricia."
  },
  {
    "search_ext_patricia",
    search_ext_patricia,
    METH_VARARGS,
    "search_ext_patricia( pointer, value ): Searches for value in patricia, returns extended info."
  },
  {
    "save_patricia",
    save_patricia,
    METH_VARARGS,
    "save_patricia( pointer, fullpath ): Saves patricia into file."
  },
  {
    "print_patricia",
    print_patricia,
    METH_VARARGS,
    "print_patricia( pointer ): Prints patricia in ASCII to stdout."
  },

  {
    NULL
  }
};

#if PY_MAJOR_VERSION >= 3
    #define MOD_DEF(ob, name, doc, methods) \
        static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
        ob = PyModule_Create(&moduledef);
#else
    #define MOD_DEF(ob, name, doc, methods) \
        ob = Py_InitModule3(name, methods, doc);
#endif

static PyObject *
moduleinit() {
  PyObject *m;

  MOD_DEF(m,
    "pynativeextractor_c",
    "Native entity extraction",
    nativeextractor_methods
  );

  return m;
}

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initpynativeextractor_c(void) {
  moduleinit();
}
#else
PyMODINIT_FUNC PyInit_pynativeextractor_c(void)
{
    return moduleinit();
}
#endif
