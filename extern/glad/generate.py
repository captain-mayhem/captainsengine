"""Regenerates the glad OpenGL loader in this folder (include/ and src/).

Installs the pinned glad version into a temporary virtual environment and
runs it with --reproducible, so it uses the OpenGL specification bundled with
that version instead of the latest one from Khronos: the same inputs always
give the same files. Needs Python 3 and network access for pip.

    python extern/glad/generate.py

To add an extension, add it to EXTENSIONS and run the script again.
"""

import os
import shutil
import subprocess
import sys
import tempfile
import venv

GLAD_VERSION = "2.0.8"
API = "gl:compatibility=3.3"
EXTENSIONS = [
    "GL_ARB_texture_filter_anisotropic",
    "GL_EXT_texture_filter_anisotropic",
    "GL_KHR_debug",
    "GL_NV_copy_image",
]

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    with tempfile.TemporaryDirectory() as tmp:
        env = os.path.join(tmp, "venv")
        venv.create(env, with_pip=True)
        bindir = "Scripts" if os.name == "nt" else "bin"
        python = os.path.join(env, bindir, "python")
        subprocess.run([python, "-m", "pip", "install", "--quiet", "--disable-pip-version-check",
                        f"glad2=={GLAD_VERSION}"], check=True)
        out = os.path.join(tmp, "out")
        subprocess.run([python, "-m", "glad", "--quiet", "--reproducible",
                        "--api", API, "--extensions", ",".join(EXTENSIONS),
                        "--out-path", out, "c"], check=True)
        for sub in ("include", "src"):
            target = os.path.join(HERE, sub)
            shutil.rmtree(target, ignore_errors=True)
            shutil.copytree(os.path.join(out, sub), target)
    print(f"glad {GLAD_VERSION} ({API}, {len(EXTENSIONS)} extensions) written to {HERE}")


if __name__ == "__main__":
    sys.exit(main())
