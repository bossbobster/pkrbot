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
    extra_link_args=[],
)

setup(
    # Just the extension, pyproject.toml has the metadata
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
    # Explicitly set name and version so setuptools uses them
    name='pkrbot',
    version='1.0.14',
)
