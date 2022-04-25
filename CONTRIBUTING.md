# PyPi upload process
* the version in `setup.py` is calculated from the latest git tag
* ```python setup.py sdist```
* ```twine upload -r pypi dist/pynativeextractor-X.Y.Z.tar.gz```
