from setuptools import setup, Extension
from Cython.Build import cythonize

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=[
        '-O3',
        '-ffast-math', '-funroll-loops', '-finline-functions',
        '-fomit-frame-pointer', '-DNDEBUG',
    ],
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
