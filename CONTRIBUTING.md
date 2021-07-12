# PyPi upload process
* Change version in `setup.py` file
* ```python setup.py sdist```
* ```twine upload -r pypi dist/pynativeextractor-X.Y.Z.tar.gz```
