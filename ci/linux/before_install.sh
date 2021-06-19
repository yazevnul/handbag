#!/bin/bash -x

wget 'https://github.com/bazelbuild/bazel/releases/download/4.1.0/bazel_4.1.0-linux-x86_64.deb'
sha256sum -c bazel_4.1.0-linux-x86_64.deb.sha256
sudo dpkg -i bazel_4.1.0-linux-x86_64.deb
