
from Cython.Distutils import build_ext
from distutils.extension import Extension
from distutils.core import setup
import numpy

from Cython.Compiler import Options
Options.annotate = True

setup(
    name='mine',
    description='Nothing',
    ext_modules=[
        Extension(
            'stabilize',
            ['stabilize.pyx'],
            include_dirs=[numpy.get_include()],
        )
    ],
    cmdclass={'build_ext': build_ext}
)
