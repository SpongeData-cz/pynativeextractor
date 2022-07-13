import glob, setuptools
import sys
from distutils.core import Extension
from setuptools import setup
import subprocess
import re

sources = ['nativeextractormodule.c'] + glob.glob('./nativeextractor/src/*.c')

if sys.version_info >= (3, 0):
    with open("README.md", "r", encoding="utf-8") as fh:
        long_description = fh.read()
else:
    with open("README.md", "r") as fh:
        long_description = fh.read().decode("utf-8")

def get_git_version():
    tag = subprocess.check_output([ "git", "describe", "--tags", "--abbrev=0", ])
    if sys.version_info >= (3, 0):
        tag = tag.decode("utf-8")
    found = re.search(r"v(?P<tag>\d+.\d+.\d+)", tag)
    if found is None:
        raise Exception("No tag in git!")
    found = found.groupdict()
    return found["tag"]

kwargs = {
    "setup_requires": ['wheel'], # for future releases to support wheel by default
    "name": "pynativeextractor",
    "version": "10.0.12", #get_git_version(), - unable to install if you use
    "author": "SpongeData s.r.o.",
    "author_email": "info@spongedata.cz",
    "description": "Python binding for nativeextractor",
    "long_description": long_description,
    "long_description_content_type": "text/markdown",
    "ext_modules": [
        Extension(
            'pynativeextractor_c',
            sources=sources,
            include_dirs=[
                './nativeextractor/src',
                'nativeextractor/include',
                '/usr/include/glib-2.0',
                '/usr/lib/x86_64-linux-gnu/glib-2.0/include'
            ],
            # library_dirs=['build/release'],
            libraries=['glib-2.0'],
            extra_compile_args=['-rdynamic', '-g', '-O0'],
        ),
    ],
    "url": "https://github.com/SpongeData-cz/pynativeextractor",
    "project_urls": {
        "Bug Tracker": "https://github.com/SpongeData-cz/pynativeextractor/issues",
    },
    "classifiers": [
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
    ],
    "packages": setuptools.find_packages(),
    "python_requires":">=2.7",
    "install_requires": [
        "wheel",
    ],
}

setup(**kwargs)

