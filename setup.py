from setuptools import setup, Extension
from Cython.Build import cythonize
import sys

# Platform-specific compiler flags
if sys.platform == 'win32':
    extra_compile_args = ['/O2', '/DNDEBUG']
else:
    extra_compile_args = [
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ]

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=extra_compile_args,
)

setup(
    ext_modules=cythonize(
        [ext],
        compiler_directives={
            'language_level': '3',
            'boundscheck': False,
            'wraparound': False,
            'cdivision': True,
            'initializedcheck': False,
        }
    ),
)
