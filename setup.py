"""
    Setup file for spar-vec-sucher.
    Use setup.cfg to configure your project.

    This file was generated with PyScaffold 4.4.1.
    PyScaffold helps you to put up the scaffold of your new Python project.
    Learn more under: https://pyscaffold.org/
"""
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext


class BuildExt(build_ext):
    def run(self):
        import numpy

        self.include_dirs.append(numpy.get_include())
        super().run()


if __name__ == "__main__":
    try:
        setup(
            use_scm_version={"version_scheme": "no-guess-dev"},
            packages=find_packages(),
            ext_modules=[
                Extension(
                    "spar_vec_sucher.c_module", ["src/spar_vec_sucher/c_code/sucher.c"],extra_compile_args=["-ffast-math"]
                )
            ],
            install_requires=["numpy"],
            setup_requires=["numpy"],
            cmdclass={"build_ext": BuildExt},
        )

    except:  # noqa
        print(
            "\n\nAn error occurred while building the project, "
            "please ensure you have the most updated version of setuptools, "
            "setuptools_scm and wheel with:\n"
            "   pip install -U setuptools setuptools_scm wheel\n\n"
        )
        raise
