# NativeExtractor module for Python
This is official Python binding for the [NativeExtractor](https://github.com/SpongeData-cz/nativeextractor) project.

<p align="center"><img src="https://raw.githubusercontent.com/SpongeData-cz/nativeextractor/main/logo.svg" width="400" /></p>
<p align="center"><img src="logo_python.png" width="400" /></p>

# Installation
## Requirements
* Python >=2.7 (>3 usage is highly recommended)
* `pip`
* `build-essential` (gcc, make)
* `libglib2.0`, `libglib2.0-dev`, `libpythonX-dev`

We recommend to use virtual environments.
```bash
virtualenv myproject
source myproject/bin/activate
```
or
```bash
python -m venv myproject
source myproject/bin/activate
```

## Instant PyPi solution
```pip install pynativeextractor```

## Manual
* Clone the repo
`git clone --recurse-submodules https://github.com/SpongeData-cz/pynativeextractor.git`

* Install via `pip` or `pip3`
    ```bash
    pip install -e ./pynativeextractor/
    ```

# Typical usage

```python
import os
from pynativeextractor.extractor import BufferStream, Extractor, DEFAULT_MINERS_PATH

# Construct new Extractor instance
ex = Extractor()
# Add fictional miner from web_entities.so with name match_url matching all URLs
ex.add_miner_so(os.path.join(DEFAULT_MINERS_PATH, 'web_entities.so'), 'match_url')
text = '{}'.format("https://spongedata.cz")

# Make from hw stream (you can also do the stream from files - use FileStream - mmap is used internally)
with BufferStream(text) as bf:
    # Initialize occurrences list as empty list
    occurrences = []
    # Set the stream to the extractor
    with ex.set_stream(bf):
        # Mine all occurrences of URLs
        while not ex.eof():
            # Summarize occurrences
            occurrences += ex.next()

print(occurrences) # Prints [{'label': 'URL', 'value': 'https://spongedata.cz', 'pos': 0, 'len': 13, 'prob': 1.0}]
```
