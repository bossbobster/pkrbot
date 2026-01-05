from setuptools import setup, Extension
from Cython.Build import cythonize
import sys

# Platform-specific compiler flags
if sys.platform.startswith('win'):
    extra_compile_args = [
        '/O2',           # Maximum optimization (speed)
        '/Oi',           # Generate intrinsic functions
        '/Ot',           # Favor fast code
        '/fp:fast',      # Fast floating point (like -ffast-math)
        '/GL',           # Whole program optimization
        '/GS-',          # Disable buffer security checks for speed
        '/DNDEBUG',      # Disable debug assertions
    ]
    extra_link_args = ['/LTCG']  # Link-time code generation
else:
    extra_compile_args = [
        '-O3',                      # Maximum optimization
        '-ffast-math',              # Fast floating point math
        '-funroll-loops',           # Unroll loops
        '-finline-functions',       # Inline functions aggressively
        '-fomit-frame-pointer',     # Omit frame pointer for speed
        '-flto',                    # Link-time optimization
        '-ftree-vectorize',         # Enable auto-vectorization
        '-fno-signed-zeros',        # Treat signed zero as unsigned (faster)
        '-fno-trapping-math',       # Assume floating-point ops don't trap
        '-DNDEBUG',                 # Disable assertions
    ]
    extra_link_args = ['-flto']    # Link-time optimization

ext = Extension(
    'pkrbot',
    ['pkrbot.pyx'],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
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
