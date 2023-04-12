# PyPi upload process
* update the version in `setup.py`
* ```python setup.py sdist```
* ```twine upload -r pypi dist/pynativeextractor-X.Y.Z.tar.gz```
