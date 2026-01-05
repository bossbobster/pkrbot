from setuptools import setup, Extension
from Cython.Build import cythonize
import sys
import os

# Read README for long description
this_directory = os.path.abspath(os.path.dirname(__file__))
try:
    with open(os.path.join(this_directory, 'README.md'), encoding='utf-8') as f:
        long_description = f.read()
except FileNotFoundError:
    long_description = 'Fast poker hand evaluation library'

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=[
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
    extra_link_args=[],
)

setup(ext_modules=[ext])
